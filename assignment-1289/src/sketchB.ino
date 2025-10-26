//Assisgnment
//Embedded IoT System Fall-2025

//Name: wania zanib                Reg#: 1289

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pin Definitions
#define BTN 13
#define LED1 18
#define BUZZER 27

// OLED Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Variables
bool ledState = false;
bool btnPressed = false;
unsigned long pressStart = 0;
String lastEvent = "Ready";

void setup() {
  // Initialize pins
  pinMode(BTN, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  
  // Initialize OLED
  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  
  updateDisplay();
}

void loop() {
  bool btnState = digitalRead(BTN);
  
  // Button pressed
  if (btnState == LOW && !btnPressed) {
    btnPressed = true;
    pressStart = millis();
  }
  
  // Button released
  if (btnState == HIGH && btnPressed) {
    unsigned long pressDuration = millis() - pressStart;
    btnPressed = false;
    
    if (pressDuration > 1500) {
      // Long press - play buzzer
      lastEvent = "Long Press";
      tone(BUZZER, 1000, 500); // 1kHz for 500ms
    } 
    else if (pressDuration > 50) {
      // Short press - toggle LED
      ledState = !ledState;
      digitalWrite(LED1, ledState);
      lastEvent = ledState ? "LED ON" : "LED OFF";
    }
    
    updateDisplay();
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("Event:");
  display.setCursor(0, 35);
  display.print(lastEvent);
  display.display();
}