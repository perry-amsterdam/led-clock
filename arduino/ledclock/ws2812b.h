#pragma once
#include <Arduino.h>
#include <time.h>

// Public API
void ws2812bBegin();
void ws2812bUpdate(const tm &now, time_t epoch);
void ws2812bShow();

