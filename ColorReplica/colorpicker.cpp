/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/ColorReplica
   ________________________________________ */

#include <Arduino.h> 
#include "colorpicker.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

extern Adafruit_NeoPixel strip;

#define PIN            23  
#define NUM_PIXELS     18  
#define DEFAULT_COLOR  0xFF0000  
#define DEFAULT_BRIGHTNESS 255  

const int touchPin1 = 12;
const int touchPin2 = 13;
const int touchPin3 = 14;
const int touchPin4 = 15;

const int threshold = 20;

int touchValue1;
int touchValue2;
int touchValue3;
int touchValue4;

enum LightingMode {
  STATIC,   
  TEST,  
  CHASE     
};

uint32_t selectedColor = DEFAULT_COLOR;
uint8_t selectedBrightness = DEFAULT_BRIGHTNESS;
LightingMode selectedMode = STATIC;  


int br;
void updateNeoPixelColorBrightness() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, selectedColor);
  }
  strip.setBrightness(selectedBrightness);
  strip.show();

  br = map(selectedBrightness, 0,255, 0,100);
}


void rainbow(int wait) {
    for (int i = 0; i < strip.numPixels(); i++)
      {
        strip.setPixelColor(i, 0, 0, 0);
      }
    strip.show(); 
    int R = 0; int G = 0; int B = 0; 
    int OldLamp;
    OldLamp = random(100,200);
    R = 250-OldLamp; G = R-255-OldLamp; B = 255-OldLamp; 
    if(G<0) G=0; if(R<0) R=0; if(B<0) B=0;
    delay(random(100,150));
}


void theaterChase(uint32_t color, int wait) {
  for (int a = 0; a < 10; a++) {
    for (int b = 0; b < 3; b++) {
      strip.clear();
      for (int c = b; c < strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color);
      }
      strip.show();
      delay(wait);
    }
  }
}

String colorname;

uint32_t colors[] = {0xf8f5f5, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF};
String colorNames[] = {"White", "Red", "Green", "Blue", "Yellow", "Magenta", "Cyan"};
int currentColorIndex = 0;

uint32_t getNextColor(uint32_t currentColor) {
  currentColorIndex = (currentColorIndex + 1) % (sizeof(colors) / sizeof(colors[0]));
  return colors[currentColorIndex];
}


void updateOLED() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g_font_7x14B);
  u8g2.drawStr(0, 10, "Color:");
  u8g2.drawStr(0, 30, "Brightness:");
  u8g2.drawStr(0, 50, "Mode:");
  u8g2.setCursor(80, 10);
  u8g2.print(colorNames[currentColorIndex]);
  u8g2.setCursor(80, 30);
  u8g2.print(br);
  u8g2.setCursor(80, 50);
  u8g2.print(selectedMode == STATIC ? "Static" : selectedMode == TEST ? "TEST" : "Chase");
  u8g2.sendBuffer();
}


void colorpickerSetup(){
  
  strip.begin();
  strip.show();  // Initialize all pixels to 'off'

  pinMode(17, INPUT_PULLUP);  // Button to change color
  pinMode(16, INPUT_PULLUP);  // Button to change brightness +
  pinMode(18, INPUT_PULLUP);  // Button to change brightness -

  Serial.begin(9600);
  Serial.println("Serial NeoPixel Control");
  Serial.println("Commands:");
  Serial.println("CRRGGBB - Set color (RRGGBB is in hexadecimal)");
  Serial.println("B### - Set brightness (0-255)");
  Serial.println("M# - Set mode (0: Static, 1: TEST, 2: Chase)");
  Serial.println("Example: C00FF00 - Set color to green");
  Serial.println("Example: B128 - Set brightness to 128");
  Serial.println("Example: M1 - Set mode to TEST");
  Serial.println();
  Serial.print("Current Color: #");
  Serial.println(selectedColor, HEX);
  Serial.print("Current Brightness: ");
  Serial.println(selectedBrightness);
  Serial.print("Current Mode: ");
  Serial.println(selectedMode == STATIC ? "Static" : selectedMode == TEST ? "TEST" : "Chase");

  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g_font_7x14B);
  u8g2.drawStr(0, 10, "Color:");
  u8g2.drawStr(0, 30, "Brightness:");
  u8g2.drawStr(0, 50, "Mode:");
  u8g2.sendBuffer();
}


void colorpickerLoop(){
  
  touchValue1 = touchRead(touchPin1);
  touchValue2 = touchRead(touchPin2);
  touchValue3 = touchRead(touchPin3);
  touchValue4 = touchRead(touchPin4);
  
  if (touchValue1 < threshold) {
    selectedColor = getNextColor(selectedColor);
    updateNeoPixelColorBrightness();
    Serial.print("New Color: #");
    Serial.println(selectedColor, HEX);
    updateOLED();
    delay(500);  
  }


  if (touchValue3 < threshold) {
    if (selectedBrightness > 0) {
      selectedBrightness = selectedBrightness - 50;
      updateNeoPixelColorBrightness();
      Serial.print("New Brightness: ");
      Serial.println(selectedBrightness);
      updateOLED();
    }
    delay(500);  
  }


  if (touchValue2 < threshold) {
    selectedMode = static_cast<LightingMode>((selectedMode + 1) % 3);
    Serial.print("New Mode: ");
    Serial.println(selectedMode == STATIC ? "Static" : selectedMode == CHASE ? "TEST" : "Chase");
    updateNeoPixelColorBrightness();
    updateOLED();
    delay(500);   
  }

  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'C') {
      // Set color based on hexadecimal input (e.g., C00FF00 for green)
      selectedColor = strtol(Serial.readStringUntil('\n').c_str(), NULL, 16);
      updateNeoPixelColorBrightness();
      Serial.print("New Color: #");
      Serial.println(selectedColor, HEX);
      updateOLED();
    } else if (command == 'B') {
      // Set brightness (0-255)
      selectedBrightness = Serial.parseInt();
      updateNeoPixelColorBrightness();
      Serial.print("New Brightness: ");
      Serial.println(selectedBrightness);
      updateOLED();
    } else if (command == 'M') {
      // Set lighting mode (0: Static, 1: TEST, 2: Chase)
      selectedMode = static_cast<LightingMode>(Serial.parseInt());
      Serial.print("New Mode: ");
      Serial.println(selectedMode == STATIC ? "Static" : selectedMode == TEST ? "TEST" : "Chase");
    }
  }


  switch (selectedMode) {
    case STATIC:
      
      break;
    case TEST:
      
      rainbow(20); 
      break;
    case CHASE:
      
      theaterChase(selectedColor, 50); 
      break;
  }
}
