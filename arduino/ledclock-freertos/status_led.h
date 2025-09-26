#pragma once
#include "globals.h"

// Basale LED-bediening
void ledBegin();
void ledColor(uint8_t r, uint8_t g, uint8_t b);
void ledBlue();
void ledRed();
void ledGreen();
void ledOff();

// Pulse-helpers (nieuw)
void ledBluePulse();			 // 120 ms aan, 880 ms uit
void ledGreenPulse();			 // 120 ms aan, 380 ms uit
void ledRedPulse();			 // 120 ms aan, 380 ms uit

// Optioneel: generieke pulse (handig voor toekomstige varianten)
void ledPulse(uint8_t r, uint8_t g, uint8_t b, uint16_t on_ms, uint16_t off_ms);
