//Assisgnment
//Embedded IoT System Fall-2025

//Name: wania zanib                Reg#: 1289

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pins
#define BTN_MODE 13
#define BTN_RESET 12
#define LED1 18
#define LED2 19
#define LED3 23

// OLED
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// State
int mode = 0;
unsigned long lastTime = 0;
int fadeValue = 0;
int fadeStep = 5;
bool blinkOn = false;

void setup() {
  pinMode(BTN_MODE, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  
  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  
  turnOffLEDs();
  updateDisplay();
}

void loop() {
  // Button 1: Cycle modes
  if (digitalRead(BTN_MODE) == LOW) {
    delay(200);
    mode = (mode + 1) % 4;
    fadeValue = 0;
    fadeStep = 5;
    turnOffLEDs();
    updateDisplay();
  }
  
  // Button 2: Reset
  if (digitalRead(BTN_RESET) == LOW) {
    delay(200);
    mode = 0;
    fadeValue = 0;
    fadeStep = 5;
    turnOffLEDs();
    updateDisplay();
  }
  
  if (mode == 0) {
    // OFF 
  }
  else if (mode == 1) {
    // Alternate Blink
    if (millis() - lastTime > 500) {
      blinkOn = !blinkOn;
      digitalWrite(LED1, blinkOn ? HIGH : LOW);
      digitalWrite(LED2, blinkOn ? LOW : HIGH);
      digitalWrite(LED3, blinkOn ? HIGH : LOW);
      lastTime = millis();
    }
  }
  else if (mode == 2) {
    // ON
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
  }
  else if (mode == 3) {
    // PWM Fade
    if (millis() - lastTime > 10) {
      analogWrite(LED1, fadeValue);
      analogWrite(LED2, fadeValue);
      analogWrite(LED3, fadeValue);
      
      fadeValue += fadeStep;
      if (fadeValue <= 0 || fadeValue >= 255) {
        fadeStep = -fadeStep;
        fadeValue = constrain(fadeValue, 0, 255);
      }
      lastTime = millis();
    }
  }
}

void turnOffLEDs() {
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("Mode: ");
  display.println(mode);
  display.setCursor(0, 35);
  
  if (mode == 0) display.print("LEDs OFF");
  else if (mode == 1) display.print("BLINK");
  else if (mode == 2) display.print("LEDs ON");
  else if (mode == 3) display.print("PWM FADE");
  
  display.display();
}