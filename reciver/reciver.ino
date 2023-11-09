/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/ColorReplica
   ________________________________________ */

#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <U8g2lib.h>
#include <Wire.h>

#define PIN            23  
#define NUM_PIXELS     18  

typedef struct struct_message {
    int r;
    int g;
    int b;
    int c;
    int ct;
    int lx;
} struct_message;


struct_message myData;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("R: ");
  Serial.println(myData.r);
  Serial.print("G: ");
  Serial.println(myData.g);
  Serial.print("B: ");
  Serial.println(myData.b);
  Serial.print("colorTemp: ");
  Serial.println(myData.ct);
  Serial.print("lux: ");
  Serial.println(myData.lx);
  Serial.println();
}
 
void setup() {

  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);

  strip.begin();
  strip.show(); 

  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g_font_7x14B);
  u8g2.drawStr(0, 10, "Color:");
  u8g2.drawStr(0, 30, "Brightness:");
  u8g2.drawStr(0, 50, "Mode:");
  u8g2.sendBuffer();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
   esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {

  int r = myData.r;  
  int g = myData.g;   
  int b = myData.b;
  
  unsigned long hexColor = (unsigned long)r << 16 | (unsigned long)g << 8 | b;
  String hexColorStr = String("#" + String(hexColor, HEX));

  Serial.println("RGB: (" + String(r) + ", " + String(g) + ", " + String(b) + ")");
  Serial.println("HEX Color Code: " + hexColorStr);


  float R = r / 255.0;
  float G = g / 255.0;
  float B = b / 255.0;
  
  float maxVal = max(max(R, G), B);
  float minVal = min(min(R, G), B);
  
  float V = maxVal;
  float S = (maxVal == 0) ? 0 : (maxVal - minVal) / maxVal;
  
  float H;
  if (maxVal == R) {
    H = 60 * ((G - B) / (maxVal - minVal)) + 360;
  } else if (maxVal == G) {
    H = 60 * ((B - R) / (maxVal - minVal)) + 120;
  } else {
    H = 60 * ((R - G) / (maxVal - minVal)) + 240;
  }
  
  H = fmod(H, 360.0);

  Serial.println(" HSV: (" + String(H) + ", " + String(S) + ", " + String(V) + ")");

  
  u8g2.clearBuffer();
  u8g2.setFont(u8g_font_7x14B);
  u8g2.drawStr(0, 10, "RGB:");
  u8g2.drawStr(0, 30, "HEX:");
  u8g2.drawStr(0, 50, "HSV:");
  u8g2.setCursor(30, 10);
  u8g2.print(myData.r);
  u8g2.print(",");
  u8g2.print(myData.g);
  u8g2.print(",");
  u8g2.println(myData.b);
  u8g2.setCursor(30, 30);
  u8g2.print(hexColorStr);
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setCursor(30, 50);
  u8g2.print(String(H));
  u8g2.print(",");
  u8g2.print(String(S));
  u8g2.print(",");
  u8g2.println(String(V));
  u8g2.sendBuffer();

  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, myData.r, myData.g, myData.b);
  }
  strip.show();
}
