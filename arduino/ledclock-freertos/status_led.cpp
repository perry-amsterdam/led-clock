#include <Arduino.h>
#include "status_led.h"

void ledBegin(){ pixel.begin(); pixel.setBrightness(LED_BRIGHTNESS); pixel.clear(); pixel.show(); }

void ledColor(uint8_t r,uint8_t g,uint8_t b){ pixel.setPixelColor(0, pixel.Color(r,g,b)); pixel.show(); }

void ledBlue(){   if(DEBUG_NET) Serial.println("\r[LED] Blauw (verbinden)\n\r");    ledColor(0,0,60); }
void ledRed(){    if(DEBUG_NET) Serial.println("\r[LED] Rood (geen WiFi/AP)\n\r");  ledColor(60,0,0); }
void ledGreen(){  if(DEBUG_NET) Serial.println("\r[LED] Groen (verbonden)\n\r");    ledColor(0,60,0); }
void ledOff(){    if(DEBUG_NET) Serial.println("\r[LED] Uit\n\r");                  ledColor(0,0,0); }
