/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/ColorReplica
   ________________________________________ */

#ifndef reciver_H
#define reciver_H

#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <U8g2lib.h>
#include <Wire.h>

void reciverSetup();
void reciverLoop();

#endif
