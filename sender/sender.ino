/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/ColorReplica
   ________________________________________ */


#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);

uint8_t broadcastAddress[] = {0x0C, 0xDC, 0x7E, 0xE9, 0x33, 0x58};

#define BOARD_ID 1

typedef struct struct_message {
  int r;
  int g;
  int b;
  int c;
  int ct;
  int lx;
} struct_message;

struct_message myData;

unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

const int buttonPin = 3; // Change to the pin you have your button connected to
int buttonState = 0;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("\r\nLast Packet Send Status: ");

  if (sendStatus == 0) {
    Serial.println("Delivery success");
  } else {
    Serial.println("Delivery fail");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  pinMode(buttonPin, INPUT_PULLUP);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
}

void loop() {
  uint16_t r, g, b, c, colorTemp, lux;

  tcs.getRawData(&r, &g, &b, &c);
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
  lux = tcs.calculateLux(r, g, b);

  // Normalize the values
  float normalizedRed = (float)r / c;
  float normalizedGreen = (float)g / c;
  float normalizedBlue = (float)b / c;

  // Scale the normalized values to 0-255
  uint8_t scaledRed = normalizedRed * 255;
  uint8_t scaledGreen = normalizedGreen * 255;
  uint8_t scaledBlue = normalizedBlue * 255;

  buttonState = digitalRead(buttonPin);

    if (buttonState == LOW) {
      // Button is pressed, send data
      myData.r = scaledRed;
      myData.g = scaledGreen;
      myData.b = scaledBlue;
      myData.c = c;
      myData.ct = colorTemp;
      myData.lx = lux;

      esp_now_send(0, (uint8_t *)&myData, sizeof(myData));
    
  

  if ((millis() - lastTime) > timerDelay) {
    // Send data periodically as before
    myData.r = scaledRed;
    myData.g = scaledGreen;
    myData.b = scaledBlue;
    myData.c = c;
    myData.ct = colorTemp;
    myData.lx = lux;

    esp_now_send(0, (uint8_t *)&myData, sizeof(myData));
    lastTime = millis();
  }
 }
}
