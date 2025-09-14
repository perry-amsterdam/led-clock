#pragma once
#include <Arduino.h>
#include <time.h>   // voor struct tm

// Init strip. Aanroepen in setup()
void ws2812bBegin();

// Optioneel: periodieke update met huidige tijd
void ws2812bUpdate(const tm& now, time_t epoch);

// Eventueel: hard flush als je iets direct wil pushen
void ws2812bShow();
