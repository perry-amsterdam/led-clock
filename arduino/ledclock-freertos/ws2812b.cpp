#define TICK_MIN_R  14
#define TICK_MIN_G   9
#define TICK_MIN_B   4

#define TICK_HOUR_R 14
#define TICK_HOUR_G  9
#define TICK_HOUR_B  4

#include "ws2812b.h"
#include <Adafruit_NeoPixel.h>
#include "hal_time_freertos.h"
#include <pgmspace.h>

// ---- Gamma tabel ----
static const uint8_t gammaTable[256] PROGMEM =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6,
	6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12,
	12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19,
	20, 20, 21, 22, 22, 23, 23, 24, 24, 25, 26, 26, 27, 27, 28, 29,
	29, 30, 31, 31, 32, 33, 33, 34, 35, 35, 36, 37, 38, 39, 39, 40,
	41, 42, 43, 43, 44, 45, 46, 47, 48, 49, 49, 50, 51, 52, 53, 54,
	55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
	71, 73, 74, 75, 76, 77, 78, 79, 81, 82, 83, 84, 85, 87, 88, 89,
	90, 91, 93, 94, 95, 97, 98, 99, 100, 102, 103, 105, 106, 107, 109, 110,
	111, 113, 114, 116, 117, 119, 120, 121, 123, 124, 126, 127, 129, 130, 132, 133,
	135, 137, 138, 140, 141, 143, 145, 146, 148, 149, 151, 153, 154, 156, 158, 159,
	161, 163, 165, 166, 168, 170, 172, 173, 175, 177, 179, 181, 182, 184, 186, 188,
	190, 192, 194, 196, 197, 199, 201, 203, 205, 207, 209, 211, 213, 215, 217, 219,
	221, 223, 225, 227, 229, 231, 234, 236, 238, 240, 242, 244, 246, 248, 251, 253
};

static inline uint8_t gammaCorrect(uint8_t v)
{
	return pgm_read_byte(&gammaTable[v]);
}


static inline void applyGammaToStrip(Adafruit_NeoPixel& strip)
{
	const int n = strip.numPixels();
	for (int i = 0; i < n; ++i)
	{
		uint32_t c = strip.getPixelColor(i);
		uint8_t r = (c >> 16) & 0xFF;
		uint8_t g = (c >> 8) & 0xFF;
		uint8_t b = (c) & 0xFF;
		r = gammaCorrect(r);
		g = gammaCorrect(g);
		b = gammaCorrect(b);
		strip.setPixelColor(i, (uint32_t)r << 16 | (uint32_t)g << 8 | b);
	}
}


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


// Nieuwe functie met ingebouwde rotatie-logica
void drawStatusTicks(uint8_t r, uint8_t g, uint8_t b)
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
	applyGammaToStrip(strip24);
	strip24.show();
	applyGammaToStrip(strip60);
	strip60.show();
}


// Handmatig refresh (als je zelf iets getekend hebt buiten ws2812bUpdate)
void ws2812bShow()
{
	applyGammaToStrip(strip24);
	strip24.show();
	applyGammaToStrip(strip60);
	strip60.show();
}


bool ws2812bGammaEnabled() { return true; }
