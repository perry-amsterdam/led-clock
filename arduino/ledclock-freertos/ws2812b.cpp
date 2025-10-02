#define TICK_MIN_R  14
#define TICK_MIN_G   9
#define TICK_MIN_B   4

#define TICK_HOUR_R 14
#define TICK_HOUR_G  9
#define TICK_HOUR_B  4

#include "ws2812b.h"
#include <Adafruit_NeoPixel.h>
#include "hal_time_freertos.h"

// ---------------------------------------------------
// Hardware pins (mag je overschrijven vr de include)
// ---------------------------------------------------
#ifndef LED_PIN_60
#define LED_PIN_60  8			 // datapin voor 60-leds ring (minuten + seconden)
#endif
#ifndef LED_PIN_24
#define LED_PIN_24  9			 // datapin voor 24-leds ring (uren)
#endif

// ---------------------------------------------------
// NeoPixel strip instances
// ---------------------------------------------------
static Adafruit_NeoPixel strip60(60, LED_PIN_60, NEO_GRB + NEO_KHZ800);
static Adafruit_NeoPixel strip24(24, LED_PIN_24, NEO_GRB + NEO_KHZ800);

// ---------------------------------------------------
// Helpers: index mapping, kleur extractie, additief tekenen
// ---------------------------------------------------

// 60-ring index met offset/richting
static inline uint16_t idx60(uint8_t pos)
{
	int16_t p = pos;
	p = (int16_t)((p * (int8_t)RING60_DIR) + RING60_OFFSET);
	p %= 60;
	if (p < 0) p += 60;
	return (uint16_t)p;
}


// 24-ring index met offset/richting
static inline uint16_t idx24(uint8_t pos)
{
	int16_t p = pos;
	p = (int16_t)((p * (int8_t)RING24_DIR) + RING24_OFFSET);
	p %= 24;
	if (p < 0) p += 24;
	return (uint16_t)p;
}


// Haal r,g,b uit door NeoPixel opgeslagen kleur (GRB volgorde bij NEO_GRB)
static inline void unpackGRB(uint32_t c, uint8_t& r, uint8_t& g, uint8_t& b)
{
	g = (uint8_t)(c >> 16);
	r = (uint8_t)(c >> 8);
	b = (uint8_t)(c);
}


// Additief tekenen (saturating add) op 60-ring
static inline void addPix60(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t pr=0, pg=0, pb=0;
	unpackGRB(strip60.getPixelColor(i), pr, pg, pb);
	uint16_t nr = pr + r; if (nr > 255) nr = 255;
	uint16_t ng = pg + g; if (ng > 255) ng = 255;
	uint16_t nb = pb + b; if (nb > 255) nb = 255;
	strip60.setPixelColor(i, strip60.Color((uint8_t)nr, (uint8_t)ng, (uint8_t)nb));
}


// Additief tekenen (saturating add) op 24-ring
static inline void addPix24(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t pr=0, pg=0, pb=0;
	unpackGRB(strip24.getPixelColor(i), pr, pg, pb);
	uint16_t nr = pr + r; if (nr > 255) nr = 255;
	uint16_t ng = pg + g; if (ng > 255) ng = 255;
	uint16_t nb = pb + b; if (nb > 255) nb = 255;
	strip24.setPixelColor(i, strip24.Color((uint8_t)nr, (uint8_t)ng, (uint8_t)nb));
}


// Alles uit
inline void clearAll()
{
	strip60.clear();
	strip24.clear();
}


void drawStatusTicks(uint8_t r, uint8_t g, uint8_t b)
{
	for (int m = 0; m < 60; m += 5)
	{
		for (int index = 0; index < 60; index += 5)
		{
			addPix60(idx60(m+index), r, g, b);
		}
	}
}


// Nieuwe functie met ingebouwde rotatie-logica
void drawStatusTicks_Rotated_Static(uint8_t r, uint8_t g, uint8_t b)
{
	clearAll();

	for (int index = 0; index < 60; index += 5)
	{
		// Positie = (statische offset + lus-index)
		addPix60(idx60(index), r, g, b);
	}

	for (int index = 0; index < 24; index += 3)
	{
		// Positie = (statische offset + lus-index)
		addPix24(idx24(index), r, g, b);
	}


	strip24.show();
	strip60.show();

	hal_delay_ms(500);

	clearAll();
	strip60.show();
	strip24.show();
}


// ---------------------------------------------------
// Tekenen: ticks & wijzers
// ---------------------------------------------------

// 5-minuten ticks (warm wit, gedimd) op 60-ring
static void drawMinuteTicks()
{
	for (int m = 0; m < 60; m += 5)
	{
		addPix60(idx60(m), TICK_MIN_R, TICK_MIN_G, TICK_MIN_B);
	}
}


// 3-uur ticks (warm wit, gedimd) op 24-ring
static void drawHourTicks()
{
	for (int h = 0; h < 24; h += 3)
	{
		addPix24(idx24(h), TICK_HOUR_R, TICK_HOUR_G, TICK_HOUR_B);
	}
}


// Wijzer op 60-ring (trailLen gereserveerd  nu enkel hoofdpixel)
static void drawHand60(uint8_t position, uint8_t r, uint8_t g, uint8_t b, uint8_t)
{
	addPix60(idx60(position), r, g, b);
}


// ---------------------------------------------------
// Publieke API
// ---------------------------------------------------
void ws2812bBegin()
{
	strip60.begin();
	strip24.begin();
	strip60.setBrightness(GLOBAL_BRIGHTNESS);
	strip24.setBrightness(GLOBAL_BRIGHTNESS);
	clearAll();
	strip60.show();
	strip24.show();
}


// now = civil time (lokaal of UTC), epoch = seconds since epoch (optioneel)
void ws2812bUpdate(const tm& now, time_t)
{
	clearAll();

	// Wijzerkleuren ophalen
	const uint8_t rSec  = COLOR_SEC_R,  gSec  = COLOR_SEC_G,  bSec  = COLOR_SEC_B;
	const uint8_t rMin  = COLOR_MIN_R,  gMin  = COLOR_MIN_G,  bMin  = COLOR_MIN_B;
	const uint8_t rHour = COLOR_HOUR_R, gHour = COLOR_HOUR_G, bHour = COLOR_HOUR_B;

	// Posities bepalen
	const uint8_t posSec  = (uint8_t)(now.tm_sec % 60);
	const uint8_t posMin  = (uint8_t)(now.tm_min % 60);
	const uint8_t posHour = (uint8_t)(now.tm_hour % 24);

	// 60-ring: minuten-ticks + wijzers
	drawMinuteTicks();
	drawHand60(posMin, rMin, gMin, bMin, TRAIL_LENGTH_MIN);
	drawHand60(posSec, rSec, gSec, bSec, TRAIL_LENGTH_SEC);

	// 24-ring: uur-ticks + urenwijzer
	drawHourTicks();
	addPix24(idx24(posHour), rHour, gHour, bHour);

	// Naar de leds sturen
	strip24.show();
	strip60.show();
}


// Handmatig refresh (als je zelf iets getekend hebt buiten ws2812bUpdate)
void ws2812bShow()
{
	strip24.show();
	strip60.show();
}
