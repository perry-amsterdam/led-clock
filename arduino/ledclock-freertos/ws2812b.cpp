#include "ws2812b.h"

// Library: Adafruit NeoPixel (recommended on ESP32/Arduino)
// Install via Library Manager: "Adafruit NeoPixel"
#include <Adafruit_NeoPixel.h>

// -----------------------------
// Hardware configuration
// -----------------------------
#ifndef LED_PIN
#define LED_PIN           8	 // Data pin for the WS2812B strip
#endif

#ifndef LED_COUNT
#define LED_COUNT         84	 // Total LEDs in a single chain (60 + 24)
#endif

#ifndef LED_BRIGHTNESS
#define LED_BRIGHTNESS    48	 // 0..255
#endif

// Strip layout: first 60 LEDs are the outer ring (seconds+minutes), next 24 LEDs are the inner ring (hours).
static constexpr uint16_t RING60_COUNT = 60;
static constexpr uint16_t RING24_COUNT = 24;

// Orientation controls  change to match your physical clock orientation.
// Position 0 is considered "12 o'clock". Increase pos clockwise if DIR = +1, counter-clockwise if DIR = -1.
#ifndef RING60_OFFSET
#define RING60_OFFSET     0	 // shift (0..59) so that position 0 maps to your 12 o'clock LED
#endif
#ifndef RING60_DIR
#define RING60_DIR        +1	 // +1 for clockwise, -1 for counter-clockwise
#endif

#ifndef RING24_OFFSET
#define RING24_OFFSET     0	 // shift (0..23) so that hour position 0 maps to your top LED on the inner ring
#endif
#ifndef RING24_DIR
#define RING24_DIR        +1	 // +1 for clockwise, -1 for counter-clockwise
#endif

// Appearance
#ifndef TICK_BRIGHTNESS
#define TICK_BRIGHTNESS   8	 // brightness for 5-minute tick marks on the 60-ring
#endif
#ifndef TRAIL_LENGTH_SEC
#define TRAIL_LENGTH_SEC  2	 // how many LEDs trail behind seconds hand (0 = no trail)
#endif
#ifndef TRAIL_LENGTH_MIN
#define TRAIL_LENGTH_MIN  1	 // trail for minutes hand
#endif

// ---------------------------------
// NeoPixel strip
// ---------------------------------
static Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// ---------------------------------
// Helpers
// ---------------------------------
static inline uint32_t col(uint8_t r, uint8_t g, uint8_t b)
{
	return strip.Color(r, g, b);
}


static inline uint16_t mod_wrap(int32_t x, uint16_t m)
{
	int32_t r = x % (int32_t)m;
	if (r < 0) r += m;
	return (uint16_t)r;
}


// Map a logical position (0..59) on the 60-ring to the strip index
static inline uint16_t idx60(int32_t pos)
{
	const int32_t logical = mod_wrap(RING60_OFFSET + (int32_t)RING60_DIR * pos, RING60_COUNT);
	return (uint16_t)logical;
}


// Map a logical position (0..23) on the 24-ring to the strip index (offset after the first 60)
static inline uint16_t idx24(int32_t pos)
{
	const int32_t logical = mod_wrap(RING24_OFFSET + (int32_t)RING24_DIR * pos, RING24_COUNT);
	return (uint16_t)(RING60_COUNT + logical);
}


// Safe set/add pixel
static inline void setPix(uint16_t i, uint32_t c)
{
	if (i < LED_COUNT) strip.setPixelColor(i, c);
}


static inline void addPix(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
	if (i >= LED_COUNT) return;
	uint32_t prev = strip.getPixelColor(i);
	uint8_t pr = (prev >> 16) & 0xFF;
	uint8_t pg = (prev >>  8) & 0xFF;
	uint8_t pb = (prev >>  0) & 0xFF;
	uint8_t nr = (uint16_t)pr + r > 255 ? 255 : pr + r;
	uint8_t ng = (uint16_t)pg + g > 255 ? 255 : pg + g;
	uint8_t nb = (uint16_t)pb + b > 255 ? 255 : pb + b;
	strip.setPixelColor(i, col(nr, ng, nb));
}


static inline void clearAll()
{
	strip.clear();
}


// Draw 5-minute tick marks on the 60-ring (dim white)
static void drawMinuteTicks()
{
	for (int m = 0; m < 60; m += 5)
	{
		const uint16_t i = idx60(m);
		addPix(i, TICK_BRIGHTNESS, TICK_BRIGHTNESS, TICK_BRIGHTNESS);
	}
}


// Draw "hand" with optional trailing
static void drawHand60(uint8_t position, uint8_t r, uint8_t g, uint8_t b, uint8_t trailLen)
{
	// tip
	addPix(idx60(position), r, g, b);

	// trail behind (wrapping)
	for (uint8_t t = 1; t <= trailLen; ++t)
	{
		uint8_t fade = (uint8_t)((uint16_t)(r+g+b) ? (uint16_t)((trailLen - t + 1) * 255 / (trailLen + 1)) : 0);

		// Apply fade proportionally to each channel if non-zero input channel
		uint8_t tr = r ? (uint8_t)((uint16_t)r * (uint16_t)(trailLen - t + 1) / (trailLen + 1)) : 0;
		uint8_t tg = g ? (uint8_t)((uint16_t)g * (uint16_t)(trailLen - t + 1) / (trailLen + 1)) : 0;
		uint8_t tb = b ? (uint8_t)((uint16_t)b * (uint16_t)(trailLen - t + 1) / (trailLen + 1)) : 0;
		addPix(idx60(mod_wrap((int)position - (int)t, 60)), tr, tg, tb);
	}
}


// ---------------------------------
// Public API
// ---------------------------------
void ws2812bBegin()
{
	strip.begin();
	strip.setBrightness(LED_BRIGHTNESS);
	clearAll();
	strip.show();
}


// now = civil time (local or UTC  you decide), epoch = seconds since epoch (optional for animations)
void ws2812bUpdate(const tm& now, time_t /*epoch*/) {
	
	// Extract time parts
	const uint8_t posSec  = (uint8_t)(now.tm_sec % 60);
	const uint8_t posMin  = (uint8_t)(now.tm_min % 60);
	const uint8_t posHour = (uint8_t)(now.tm_hour % 24);
	
	// Colors (you can tweak)
	const uint8_t rHour = 180, gHour = 0,   bHour = 0;	// Red
	const uint8_t rMin  = 0,   gMin  = 160, bMin  = 0;      // Green
	const uint8_t rSec  = 0,   gSec  = 0,   bSec  = 180;	// B;ue
	
	clearAll();
	
	// Background ticks on the 60-ring every 5 minutes
	drawMinuteTicks();
	
	// Minutes hand on 60-ring (with a short trail)
	drawHand60(posMin, rMin, gMin, bMin, TRAIL_LENGTH_MIN);
	
	// Seconds hand on 60-ring (with a short trail)
	drawHand60(posSec, rSec, gSec, bSec, TRAIL_LENGTH_SEC);
	
	// Hours on 24-ring
	addPix(idx24(posHour), rHour, gHour, bHour);
	
	//// Optionally: show minute-progress on hour ring (subtle)
	//// E.g., light the next hour slot dimly proportional to minutes progress.
	//{
	//	const uint8_t nextHour = (uint8_t)((posHour + 1) % 24);
	//
	//	// 0..177 approx
	//	const uint8_t dim = (uint8_t)(now.tm_min * 3);
	//
	//	// subtle red
	//	addPix(idx24(nextHour), dim / 8, 0, 0);
	//}
	
	strip.show();
}
