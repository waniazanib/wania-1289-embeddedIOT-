#define BLYNK_TEMPLATE_ID "TMPL6vuoCKd1R"
#define BLYNK_TEMPLATE_NAME "final"
#define BLYNK_AUTH_TOKEN "viEEqpqzocObA6QuEJzZf4wp_lsW2Dzj"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include "model.h"

// Pin Definitions
#define DHTPIN 4
#define PIRPIN 13
#define LDRPIN 34
#define RELAY_FAN 12
#define RELAY_LIGHT 14
#define DHTTYPE DHT11

// WiFi & MQTT Configuration
char ssid[] = "Mian House";
char pass[] = "65329650";

// MQTT Broker Settings
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_user = "";
const char* mqtt_pass = "";

// MQTT Topics
const char* topic_temp = "smartroom/sensors/temperature";
const char* topic_hum = "smartroom/sensors/humidity";
const char* topic_light = "smartroom/sensors/lightlevel";
const char* topic_motion = "smartroom/sensors/motion";
const char* topic_comfort = "smartroom/ai/comfort";
const char* topic_mode = "smartroom/control/mode";
const char* topic_fan = "smartroom/control/fan";
const char* topic_light_ctrl = "smartroom/control/light";
const char* topic_status = "smartroom/status";

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
Eloquent::ML::Port::RandomForest model;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

char auth[] = BLYNK_AUTH_TOKEN;
bool manualOverride = false;
int manualFanPower = 0;
int manualLightPower = 0;

unsigned long lastMqttPublish = 0;
const long mqttPublishInterval = 2000;

// --- Blynk Callbacks ---

// Manual Override Switch
BLYNK_WRITE(V5) {
  manualOverride = param.asInt();
  mqttClient.publish(topic_mode, manualOverride ? "MANUAL" : "AUTO");

  if (!manualOverride) {
    // Reset manual fan/light when switching to AUTO
    manualFanPower = 0;
    manualLightPower = 0;
    Blynk.virtualWrite(V6, manualFanPower);
    Blynk.virtualWrite(V7, manualLightPower);
  }
}

// Fan Slider
BLYNK_WRITE(V6) {
  if (manualOverride) {
    manualFanPower = param.asInt();
    Serial.printf("Manual Fan Power set to %d\n", manualFanPower);
  } else {
    Blynk.virtualWrite(V6, 0);
  }
}

// Light Slider
BLYNK_WRITE(V7) {
  if (manualOverride) {
    manualLightPower = param.asInt();
    Serial.printf("Manual Light Power set to %d\n", manualLightPower);
  } else {
    Blynk.virtualWrite(V7, 0);
  }
}

// --- MQTT Callback ---
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) message += (char)payload[i];

  Serial.print("MQTT Message on topic: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.println(message);

  if (strcmp(topic, topic_mode) == 0) {
    if (message == "MANUAL" || message == "1") {
      manualOverride = true;
      Blynk.virtualWrite(V5, 1);
    } else if (message == "AUTO" || message == "0") {
      manualOverride = false;
      Blynk.virtualWrite(V5, 0);
      manualFanPower = 0;
      manualLightPower = 0;
      Blynk.virtualWrite(V6, 0);
      Blynk.virtualWrite(V7, 0);
    }
  }

  if (strcmp(topic, topic_fan) == 0) {
    if (message == "ON" || message == "1") {
      manualFanPower = 1;
      Blynk.virtualWrite(V6, 1);
    } else if (message == "OFF" || message == "0") {
      manualFanPower = 0;
      Blynk.virtualWrite(V6, 0);
    }
  }

  if (strcmp(topic, topic_light_ctrl) == 0) {
    if (message == "ON" || message == "1") {
      manualLightPower = 1;
      Blynk.virtualWrite(V7, 1);
    } else if (message == "OFF" || message == "0") {
      manualLightPower = 0;
      Blynk.virtualWrite(V7, 0);
    }
  }
}

// --- MQTT Reconnect ---
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "ESP32-SmartRoom-" + String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Connected!");
      mqttClient.subscribe(topic_mode);
      mqttClient.subscribe(topic_fan);
      mqttClient.subscribe(topic_light_ctrl);
      mqttClient.publish(topic_status, "online");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" - Retrying in 5 seconds");
      delay(5000);
    }
  }
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  pinMode(PIRPIN, INPUT_PULLDOWN);  // fix PIR always HIGH
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_LIGHT, OUTPUT);

  // Set initial relay state OFF (active LOW)
  digitalWrite(RELAY_FAN, HIGH);
  digitalWrite(RELAY_LIGHT, HIGH);

  dht.begin();

  Blynk.begin(auth, ssid, pass);

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();

  Serial.println("Smart Room System Started");
}

// --- Loop ---
void loop() {
  Blynk.run();

  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();

  // --- Read Sensors ---
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  int lightLevel = analogRead(LDRPIN);
  int motion = digitalRead(PIRPIN);

  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // --- AI Prediction ---
  float input[3] = {t, h, (float)lightLevel};
  int comfortScore = model.predict(input);

  // --- FAN CONTROL ---
  if (manualOverride) {
    digitalWrite(RELAY_FAN, manualFanPower ? LOW : HIGH);
  } else {
    digitalWrite(RELAY_FAN, (motion == HIGH && comfortScore == 0) ? LOW : HIGH);
  }

  // --- LIGHT CONTROL ---
  if (manualOverride) {
    digitalWrite(RELAY_LIGHT, manualLightPower ? LOW : HIGH);
  } else {
    digitalWrite(RELAY_LIGHT, (lightLevel < 1000) ? LOW : HIGH); // adjust threshold if needed
  }

  // --- Publish MQTT every 2 sec ---
  unsigned long currentMillis = millis();
  if (currentMillis - lastMqttPublish >= mqttPublishInterval) {
    lastMqttPublish = currentMillis;

    mqttClient.publish(topic_temp, String(t, 1).c_str());
    mqttClient.publish(topic_hum, String(h, 1).c_str());
    mqttClient.publish(topic_light, String(lightLevel).c_str());
    mqttClient.publish(topic_motion, motion == HIGH ? "DETECTED" : "NONE");
    mqttClient.publish(topic_comfort, comfortScore == 1 ? "GOOD" : "BAD");

    StaticJsonDocument<200> doc;
    doc["temperature"] = t;
    doc["humidity"] = h;
    doc["lightLevel"] = lightLevel;
    doc["motion"] = motion == HIGH ? 1 : 0;
    doc["comfortScore"] = comfortScore;
    doc["mode"] = manualOverride ? "MANUAL" : "AUTO";
    doc["fanState"] = digitalRead(RELAY_FAN) == LOW ? "ON" : "OFF";
    doc["lightState"] = digitalRead(RELAY_LIGHT) == LOW ? "ON" : "OFF";

    char jsonBuffer[200];
    serializeJson(doc, jsonBuffer);
    mqttClient.publish(topic_status, jsonBuffer);

    Serial.println("Published to MQTT:");
    Serial.println(jsonBuffer);
  }

  // --- Update Blynk ---
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
  Blynk.virtualWrite(V3, lightLevel);
  Blynk.virtualWrite(V4, comfortScore);

  // --- OLED Display ---
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Temp: "); display.print(t, 1); display.println("C");
  display.print("Hum:  "); display.print(h, 1); display.println("%");
  display.print("LDR:  "); display.println(lightLevel);
  display.print("Motion: "); display.println(motion == HIGH ? "Detected" : "None");
  display.print("Mode: "); display.println(manualOverride ? "MANUAL" : "AUTO AI");
  display.print("Room: "); display.println(comfortScore == 1 ? "GOOD :)" : "BAD !!");
  display.display();

  // --- Debug ---
  Serial.printf(
    "Motion=%d | LDR=%d | Fan=%s | Light=%s | Manual=%d\n",
    motion,
    lightLevel,
    digitalRead(RELAY_FAN) == LOW ? "ON" : "OFF",
    digitalRead(RELAY_LIGHT) == LOW ? "ON" : "OFF",
    manualOverride
  );

  delay(1000);
}
