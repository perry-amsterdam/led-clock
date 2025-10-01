#include "ws2812b.h"
#include <Adafruit_NeoPixel.h>

// ---------------------------------------------------
// Hardware instellingen
// ---------------------------------------------------
#ifndef LED_PIN_60
#define LED_PIN_60  8			 // datapin voor 60-leds ring (seconden + minuten)
#endif

#ifndef LED_PIN_24
#define LED_PIN_24  9			 // datapin voor 24-leds ring (uren)
#endif

#ifndef GLOBAL_BRIGHTNESS
#define GLOBAL_BRIGHTNESS  64	 // standaard helderheid (0..255)
#endif

// Trail-instellingen (lengte van staarten bij bewegende wijzers)
#ifndef TRAIL_LENGTH_SEC
#define TRAIL_LENGTH_SEC   2
#endif
#ifndef TRAIL_LENGTH_MIN
#define TRAIL_LENGTH_MIN   1
#endif

// Orintatie en richting van de ringen
#ifndef RING60_OFFSET
#define RING60_OFFSET      0
#endif
#ifndef RING60_DIR
#define RING60_DIR         +1	 // +1 = met de klok mee, -1 = tegen de klok in
#endif

#ifndef RING24_OFFSET
#define RING24_OFFSET      0
#endif
#ifndef RING24_DIR
#define RING24_DIR         +1
#endif

// Kleuren van wijzers
#ifndef COLOR_SEC_R
#define COLOR_SEC_R        0
#endif
#ifndef COLOR_SEC_G
#define COLOR_SEC_G        0
#endif
#ifndef COLOR_SEC_B
#define COLOR_SEC_B        150
#endif

#ifndef COLOR_MIN_R
#define COLOR_MIN_R        0
#endif
#ifndef COLOR_MIN_G
#define COLOR_MIN_G        80
#endif
#ifndef COLOR_MIN_B
#define COLOR_MIN_B        0
#endif

#ifndef COLOR_HOUR_R
#define COLOR_HOUR_R       140
#endif
#ifndef COLOR_HOUR_G
#define COLOR_HOUR_G       0
#endif
#ifndef COLOR_HOUR_B
#define COLOR_HOUR_B       0
#endif

// Helderheid van tick-markeringen (bv. 5-minuten stipjes)
#ifndef TICK_BRIGHTNESS
#define TICK_BRIGHTNESS    8
#endif

// ---------------------------------------------------
// Initialiseer LED-strips
// ---------------------------------------------------
static Adafruit_NeoPixel strip60(60, LED_PIN_60, NEO_GRB + NEO_KHZ800);
static Adafruit_NeoPixel strip24(24, LED_PIN_24, NEO_GRB + NEO_KHZ800);

// ---------------------------------------------------
// Helper functies
// ---------------------------------------------------

// Bereken correcte index voor 60-leds ring met offset/richting
static inline uint16_t idx60(uint8_t pos)
{
	int16_t p = pos;
	p = (int16_t)((p * (int8_t)RING60_DIR) + RING60_OFFSET);
	p %= 60;
	if (p < 0) p += 60;
	return (uint16_t)p;
}


// Bereken correcte index voor 24-leds ring
static inline uint16_t idx24(uint8_t pos)
{
	int16_t p = pos;
	p = (int16_t)((p * (int8_t)RING24_DIR) + RING24_OFFSET);
	p %= 24;
	if (p < 0) p += 24;
	return (uint16_t)p;
}


// Kleur uit NeoPixel-pakket (kleur zit in GRB volgorde ipv RGB)
static inline void unpackGRB(uint32_t c, uint8_t& r, uint8_t& g, uint8_t& b)
{
	g = (uint8_t)(c >> 16);
	r = (uint8_t)(c >> 8);
	b = (uint8_t)(c);
}


// Voeg kleur toe aan pixel op 60-ring, zonder dat waarden boven 255 gaan
static inline void addPix60(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t pr=0, pg=0, pb=0;
	unpackGRB(strip60.getPixelColor(i), pr, pg, pb);
	uint16_t nr = pr + r; if (nr > 255) nr = 255;
	uint16_t ng = pg + g; if (ng > 255) ng = 255;
	uint16_t nb = pb + b; if (nb > 255) nb = 255;
	strip60.setPixelColor(i, strip60.Color((uint8_t)nr, (uint8_t)ng, (uint8_t)nb));
}


// Zelfde als hierboven maar voor 24-ring
static inline void addPix24(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t pr=0, pg=0, pb=0;
	unpackGRB(strip24.getPixelColor(i), pr, pg, pb);
	uint16_t nr = pr + r; if (nr > 255) nr = 255;
	uint16_t ng = pg + g; if (ng > 255) ng = 255;
	uint16_t nb = pb + b; if (nb > 255) nb = 255;
	strip24.setPixelColor(i, strip24.Color((uint8_t)nr, (uint8_t)ng, (uint8_t)nb));
}


// Maak beide ringen leeg (alles uit)
static inline void clearAll()
{
	strip60.clear();
	strip24.clear();
}


// Teken 5-minuten stipjes (dim wit) op 60-leds ring
static void drawMinuteTicks()
{
	for (int m = 0; m < 60; m += 5)
	{
		addPix60(idx60(m), TICK_BRIGHTNESS, TICK_BRIGHTNESS, TICK_BRIGHTNESS);
	}
}


// Teken 3-uurs markeringen (dim wit) op 24-leds ring
static void drawHourTicks()
{
	for (int m = 0; m < 24; m += 3)
	{
		addPix24(idx24(m), TICK_BRIGHTNESS, TICK_BRIGHTNESS, TICK_BRIGHTNESS);
	}
}


// Teken wijzer (nu zonder echte trail, maar wel voorzien van parameter)
static void drawHand60(uint8_t position, uint8_t r, uint8_t g, uint8_t b, uint8_t /*trailLen*/) {
addPix60(idx60(position), r, g, b);
}


// ---------------------------------------------------
// Publieke functies
// ---------------------------------------------------

// Initialisatie: strips starten en helderheid zetten
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


// Update klokweergave
void ws2812bUpdate(const tm& now, time_t /*epoch*/) {
	clearAll();
	
	// Kleuren instellen per wijzer
	const uint8_t rSec  = COLOR_SEC_R,  gSec  = COLOR_SEC_G,  bSec  = COLOR_SEC_B;
	const uint8_t rMin  = COLOR_MIN_R,  gMin  = COLOR_MIN_G,  bMin  = COLOR_MIN_B;
	const uint8_t rHour = COLOR_HOUR_R, gHour = COLOR_HOUR_G, bHour = COLOR_HOUR_B;
	
	// Huidige posities bepalen
	const uint8_t posSec  = (uint8_t)(now.tm_sec % 60);
	const uint8_t posMin  = (uint8_t)(now.tm_min % 60);
	const uint8_t posHour = (uint8_t)(now.tm_hour % 24);
	
	// 60-led ring: ticks, minuten- en secondenwijzer
	drawMinuteTicks();
	drawHand60(posMin, rMin, gMin, bMin, TRAIL_LENGTH_MIN);
	drawHand60(posSec, rSec, gSec, bSec, TRAIL_LENGTH_SEC);
	
	// 24-led ring: urenwijzer en tickmarks
	drawHourTicks();
	addPix24(idx24(posHour), rHour, gHour, bHour);
	
	// Data naar LEDs sturen
	strip24.show();
	strip60.show();
}


// Handmatig forceren van refresh
void ws2812bShow()
{
	strip24.show();
	strip60.show();
}
