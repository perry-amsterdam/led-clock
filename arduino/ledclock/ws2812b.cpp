#include "ws2812b.h"
#include <Adafruit_NeoPixel.h>

// ---- Config defaults (kun je via -D... of #define overschrijven) ----
#ifndef LED_PIN
  #define LED_PIN 2
#endif
#ifndef LED_COUNT
  #define LED_COUNT 84
#endif
#ifndef LED_BRIGHTNESS
  #define LED_BRIGHTNESS 32
#endif

// ---- interne strip ----
static Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void ws2812bBegin() {
  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
  strip.show(); // alle leds uit
}

// Kleine helper
static void clearStrip() {
  for (int i = 0; i < LED_COUNT; i++) strip.setPixelColor(i, 0);
}

// Demo-klok (uren = rood, minuten = groen, seconden = blauw)
void ws2812bUpdate(const tm &now, time_t /*epoch*/) {
  clearStrip();

  int sec = now.tm_sec % LED_COUNT;
  int min = now.tm_min % LED_COUNT;
  int hour = (now.tm_hour % 12) * 5;  // 12h → 60 leds

  strip.setPixelColor(sec,  strip.Color(0,   0, 255)); // blauw
  strip.setPixelColor(min,  strip.Color(0, 255,   0)); // groen
  strip.setPixelColor(hour, strip.Color(255, 0,   0)); // rood

  strip.show();
}

void ws2812bShow() {
  strip.show();
}
