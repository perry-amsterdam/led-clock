#pragma once
#include <Arduino.h>
#include <time.h>				 // voor struct tm

// Configuratie:
// - Gebruik twee IO-pinnen: n voor de 60-leds ring (minuten/seconden)
//   en n voor de 24-leds ring (uren).
// - Pas deze defines eventueel aan in je build of voor je include van deze header:
//
//   #define LED_PIN_60   8     // data pin voor ring met 60 leds
//   #define LED_PIN_24   9     // data pin voor ring met 24 leds
//
//   #define GLOBAL_BRIGHTNESS  64    // 0..255 (optioneel)
//
//   // Orintatie (offset/richting) per ring
//   #define RING60_OFFSET  0         // 0..59
//   #define RING60_DIR     +1        // +1 = clockwise, -1 = counter-clockwise
//   #define RING24_OFFSET  0         // 0..23
//   #define RING24_DIR     +1        // +1 = clockwise, -1 = counter-clockwise
//
//   // Trails
//   #define TRAIL_LENGTH_SEC  2
//   #define TRAIL_LENGTH_MIN  1
//
//   // Kleuren (0..255)
/*
   #define COLOR_SEC_R   0
   #define COLOR_SEC_G   0
   #define COLOR_SEC_B   150

   #define COLOR_MIN_R   0
   #define COLOR_MIN_G   80
   #define COLOR_MIN_B   0

   #define COLOR_HOUR_R  140
   #define COLOR_HOUR_G  0
   #define COLOR_HOUR_B  0
*/

// Init strips. Aanroepen in setup()
void ws2812bBegin();

// Optioneel: periodieke update met huidige tijd
void ws2812bUpdate(const tm& now, time_t epoch);

// Eventueel: hard flush als je iets direct wil pushen
void ws2812bShow();
