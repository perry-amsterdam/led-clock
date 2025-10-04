#pragma once
#include <Arduino.h>
#include <time.h>				 // voor struct tm

// ---------------------------------------------------
// Hardware pins (mag je overschrijven vr de include)
// ---------------------------------------------------
#ifndef LED_PIN_60
#define LED_PIN_60  8			 // datapin voor 60-leds ring (minuten + seconden)
#endif
#ifndef LED_PIN_24
#define LED_PIN_24  9			 // datapin voor 24-leds ring (uren)
#endif

#ifndef GLOBAL_BRIGHTNESS
#define GLOBAL_BRIGHTNESS  64	 // 0..255, globale dimmer voor lle pixels
#endif

// Orintatie per ring (offset = waar index 0 valt, dir = draairichting)
#ifndef RING60_OFFSET
#define RING60_OFFSET  0
#endif
#ifndef RING60_DIR
#define RING60_DIR     +1		 // +1 = met klok mee, -1 = tegen klok in
#endif
#ifndef RING24_OFFSET
#define RING24_OFFSET  0
#endif
#ifndef RING24_DIR
#define RING24_DIR     +1
#endif

// ---------------------------------------------------
// Wijzerkleuren (RGB 0..255)  pas aan naar smaak
// ---------------------------------------------------
#ifndef COLOR_SEC_R
#define COLOR_SEC_R   0
#endif
#ifndef COLOR_SEC_G
#define COLOR_SEC_G   0
#endif
#ifndef COLOR_SEC_B
#define COLOR_SEC_B   150
#endif

#ifndef COLOR_MIN_R
#define COLOR_MIN_R   0
#endif
#ifndef COLOR_MIN_G
#define COLOR_MIN_G   80
#endif
#ifndef COLOR_MIN_B
#define COLOR_MIN_B   0
#endif

#ifndef COLOR_HOUR_R
#define COLOR_HOUR_R  140
#endif
#ifndef COLOR_HOUR_G
#define COLOR_HOUR_G  0
#endif
#ifndef COLOR_HOUR_B
#define COLOR_HOUR_B  0
#endif

// ---------------------------------------------------
// Tick-kleuren (warm wit, gedimd standaard)
// - Minuten-ticks op 60-ring (elke 5)
// - Uur-ticks op 24-ring (elke 3)
// Tip: verhoog/verlaag alle drie samen als je ze feller/zachter wil.
// ---------------------------------------------------
#ifndef TICK_MIN_R
#define TICK_MIN_R  20
#endif
#ifndef TICK_MIN_G
#define TICK_MIN_G  16
#endif
#ifndef TICK_MIN_B
#define TICK_MIN_B  10
#endif

#ifndef TICK_HOUR_R
#define TICK_HOUR_R  20
#endif
#ifndef TICK_HOUR_G
#define TICK_HOUR_G  16
#endif
#ifndef TICK_HOUR_B
#define TICK_HOUR_B  10
#endif

// ---------------------------------------------------
// Publieke API
// ---------------------------------------------------

// Draw led clock status.
void drawStatusTicks(uint8_t r, uint8_t g, uint8_t b);

// Init strips. Aanroepen in setup().
void ws2812bBegin();

// Periodieke update met huidige tijd.
// - now: struct tm (lokaal of UTC; jij bepaalt)
// - epoch: seconds since epoch (optioneel; bruikbaar voor animaties)
void ws2812bUpdate(const tm& now, time_t epoch);

// Hard flush als je zelf pixels set en direct wilt pushen.
void ws2812bShow();
