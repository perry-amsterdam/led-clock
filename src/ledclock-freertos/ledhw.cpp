#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "ledhw.h"

#ifndef LED_PIN_60
#define LED_PIN_60 8
#endif
#ifndef LED_PIN_24
#define LED_PIN_24 9
#endif
#ifndef GLOBAL_BRIGHTNESS
#define GLOBAL_BRIGHTNESS 64
#endif
#ifndef RING60_DIR
#define RING60_DIR (+1)
#endif
#ifndef RING24_DIR
#define RING24_DIR (+1)
#endif
#ifndef RING60_OFFSET
#define RING60_OFFSET 0
#endif
#ifndef RING24_OFFSET
#define RING24_OFFSET 0
#endif

static Adafruit_NeoPixel s60(60, LED_PIN_60, NEO_GRB + NEO_KHZ800);
static Adafruit_NeoPixel s24(24, LED_PIN_24, NEO_GRB + NEO_KHZ800);
static bool s_inited = false;

static inline uint8_t sadd8(uint8_t a, uint8_t b)
{
	uint16_t x = uint16_t(a)+uint16_t(b);
	return (x>255)?255:uint8_t(x);
}


void ledhwInitOnce()
{
	if (s_inited) return;
	s_inited = true;
	s60.begin(); s24.begin();
	ledhwSetGlobalBrightness(GLOBAL_BRIGHTNESS);
	ledhwClearAll();
	ledhwShow();
}


void ledhwSetGlobalBrightness(uint8_t b)
{
	s60.setBrightness(b);
	s24.setBrightness(b);
}


void ledhwClearAll()
{
	s60.clear();
	s24.clear();
}


void ledhwShow()
{
	s60.show();
	s24.show();
}


uint16_t ring60Index(int raw)
{
	int x = (raw % 60 + 60) % 60;
	x = (RING60_DIR>0) ? x : (60 - x) % 60;
	x = (x + RING60_OFFSET) % 60;
	return (uint16_t)x;
}


uint16_t ring24Index(int raw)
{
	int x = (raw % 24 + 24) % 24;
	x = (RING24_DIR>0) ? x : (24 - x) % 24;
	x = (x + RING24_OFFSET) % 24;
	return (uint16_t)x;
}


void ledhwSet60(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
	s60.setPixelColor(i, r,g,b);
}


void ledhwSet24(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
	s24.setPixelColor(i, r,g,b);
}


void ledhwAdd60(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t prev = s60.getPixelColor(i);
	uint8_t pr = (prev>>16) & 0xFF;
	uint8_t pg = (prev>> 8) & 0xFF;
	uint8_t pb = (prev    ) & 0xFF;
	s60.setPixelColor(i, sadd8(pr,r), sadd8(pg,g), sadd8(pb,b));
}


void ledhwAdd24(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t prev = s24.getPixelColor(i);
	uint8_t pr = (prev>>16) & 0xFF;
	uint8_t pg = (prev>> 8) & 0xFF;
	uint8_t pb = (prev    ) & 0xFF;
	s24.setPixelColor(i, sadd8(pr,r), sadd8(pg,g), sadd8(pb,b));
}
