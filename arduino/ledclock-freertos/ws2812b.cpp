#include "ws2812b.h"
#include <Adafruit_NeoPixel.h>

// -----------------------------
// Hardware configuration
// -----------------------------
#ifndef LED_PIN_60
#define LED_PIN_60       8		 // Data pin voor de 60-leds ring (minuten + seconden)
#endif
#ifndef LED_PIN_24
#define LED_PIN_24       9		 // Data pin voor de 24-leds ring (uren)
#endif

#ifndef GLOBAL_BRIGHTNESS
#define GLOBAL_BRIGHTNESS  64	 // 0..255
#endif

#ifndef TRAIL_LENGTH_SEC
#define TRAIL_LENGTH_SEC   2
#endif
#ifndef TRAIL_LENGTH_MIN
#define TRAIL_LENGTH_MIN   1
#endif

#ifndef RING60_OFFSET
#define RING60_OFFSET      0
#endif
#ifndef RING60_DIR
#define RING60_DIR         +1	 // +1 clockwise, -1 counter-clockwise
#endif

#ifndef RING24_OFFSET
#define RING24_OFFSET      0
#endif
#ifndef RING24_DIR
#define RING24_DIR         +1	 // +1 clockwise, -1 counter-clockwise
#endif

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

#ifndef TICK_BRIGHTNESS
#define TICK_BRIGHTNESS    8	 // helderheid van 5-minuten markeringen
#endif

// ---------------------------------
// NeoPixel strips
// ---------------------------------
static Adafruit_NeoPixel strip60(60, LED_PIN_60, NEO_GRB + NEO_KHZ800);
static Adafruit_NeoPixel strip24(24, LED_PIN_24, NEO_GRB + NEO_KHZ800);

// Helpers om index-orintatie te normaliseren
static inline uint16_t idx60(uint8_t pos)
{
	// pos 0..59  ring-index met offset en richting
	int16_t p = pos;
	p = (int16_t)((p * (int8_t)RING60_DIR) + RING60_OFFSET);
	p %= 60;
	if (p < 0) p += 60;
	return (uint16_t)p;
}


static inline uint16_t idx24(uint8_t pos)
{
	// pos 0..23  ring-index met offset en richting
	int16_t p = pos;
	p = (int16_t)((p * (int8_t)RING24_DIR) + RING24_OFFSET);
	p %= 24;
	if (p < 0) p += 24;
	return (uint16_t)p;
}


// Extract r,g,b uit NeoPixel packed kleur (GRB volgorde bij NEO_GRB)
static inline void unpackGRB(uint32_t c, uint8_t& r, uint8_t& g, uint8_t& b)
{
	g = (uint8_t)(c >> 16);
	r = (uint8_t)(c >> 8);
	b = (uint8_t)(c);
}


// Saturating add voor een pixel op de 60-ring
static inline void addPix60(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t pr=0, pg=0, pb=0;
	unpackGRB(strip60.getPixelColor(i), pr, pg, pb);
	uint16_t nr = pr + r; if (nr > 255) nr = 255;
	uint16_t ng = pg + g; if (ng > 255) ng = 255;
	uint16_t nb = pb + b; if (nb > 255) nb = 255;
	strip60.setPixelColor(i, strip60.Color((uint8_t)nr, (uint8_t)ng, (uint8_t)nb));
}


// Saturating add voor een pixel op de 24-ring
static inline void addPix24(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t pr=0, pg=0, pb=0;
	unpackGRB(strip24.getPixelColor(i), pr, pg, pb);
	uint16_t nr = pr + r; if (nr > 255) nr = 255;
	uint16_t ng = pg + g; if (ng > 255) ng = 255;
	uint16_t nb = pb + b; if (nb > 255) nb = 255;
	strip24.setPixelColor(i, strip24.Color((uint8_t)nr, (uint8_t)ng, (uint8_t)nb));
}


static inline void clearAll()
{
	strip60.clear();
	strip24.clear();
}


// 5-minuten markeringen (60-ring) als dim wit
static void drawMinuteTicks()
{
	for (int m = 0; m < 60; m += 5)
	{
		addPix60(idx60(m), TICK_BRIGHTNESS, TICK_BRIGHTNESS, TICK_BRIGHTNESS);
	}
}


// 6-hour markeringen (24-ring) als dim wit
static void drawHourTicks()
{
	for (int m = 0; m < 24; m += 3)
	{
		addPix24(idx24(m), TICK_BRIGHTNESS, TICK_BRIGHTNESS, TICK_BRIGHTNESS);
	}
}


// Teken een wijzer op de 60-ring met trailing
static void drawHand60(uint8_t position, uint8_t r, uint8_t g, uint8_t b, uint8_t trailLen)
{
	addPix60(idx60(position), r, g, b);
}


// ---------------------------------
// Public API
// ---------------------------------
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


// now = civil time (local of UTC  jij bepaalt), epoch = seconds since epoch (optioneel voor animaties)
void ws2812bUpdate(const tm& now, time_t /*epoch*/) {
	clearAll();
	
	// kleuren per wijzer
	const uint8_t rSec  = COLOR_SEC_R,  gSec  = COLOR_SEC_G,  bSec  = COLOR_SEC_B;
	const uint8_t rMin  = COLOR_MIN_R,  gMin  = COLOR_MIN_G,  bMin  = COLOR_MIN_B;
	const uint8_t rHour = COLOR_HOUR_R, gHour = COLOR_HOUR_G, bHour = COLOR_HOUR_B;
	
	// posities
	const uint8_t posSec  = (uint8_t)(now.tm_sec % 60);
	const uint8_t posMin  = (uint8_t)(now.tm_min % 60);
	const uint8_t posHour = (uint8_t)(now.tm_hour % 24);
	
	// 60-ring: ticks, minuten en seconden
	drawMinuteTicks();
	drawHand60(posMin, rMin, gMin, bMin, TRAIL_LENGTH_MIN);
	drawHand60(posSec, rSec, gSec, bSec, TRAIL_LENGTH_SEC);
	
	// 24-ring: uren + (optioneel) subtiele vooruitblik op volgende uur op basis van minutenprogressie
	drawHourTicks();
	addPix24(idx24(posHour), rHour, gHour, bHour);
	
	// Uitsturen
	strip24.show();					 // mag in willekeurige volgorde; beide lijnen lopen parallel
	strip60.show();
}


void ws2812bShow()
{
	strip24.show();
	strip60.show();
}
