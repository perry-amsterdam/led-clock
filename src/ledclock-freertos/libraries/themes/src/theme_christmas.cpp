#include <Arduino.h>
#include "theme.h"
#include "theme_registry.h"
#include "hal_time_freertos.h"
#include "ledhw.h"
#include "theme_christmas.h"

// ---------- Color helpers (0..255) ----------
static inline uint8_t qadd8(uint8_t a, uint8_t b){ uint16_t s = uint16_t(a)+uint16_t(b); return (s>255)?255:uint8_t(s); }
static inline uint8_t qmul8(uint8_t a, uint8_t b){ return uint8_t((uint16_t(a)*uint16_t(b))/255); }

// Warm cozy colors
#ifndef COLOR_RED_R
#define COLOR_RED_R   255
#endif
#ifndef COLOR_RED_G
#define COLOR_RED_G   36
#endif
#ifndef COLOR_RED_B
#define COLOR_RED_B   36
#endif

#ifndef COLOR_GREEN_R
#define COLOR_GREEN_R 20
#endif
#ifndef COLOR_GREEN_G
#define COLOR_GREEN_G 255
#endif
#ifndef COLOR_GREEN_B
#define COLOR_GREEN_B 40
#endif

#ifndef COLOR_WHITE_R
#define COLOR_WHITE_R 255
#endif
#ifndef COLOR_WHITE_G
#define COLOR_WHITE_G 200
#endif
#ifndef COLOR_WHITE_B
#define COLOR_WHITE_B 160
#endif

// --------- Forward declarations ---------
static void beginChristmas();
static void updateChristmas(const tm& now, time_t epoch);
static void showStartupPattern(uint8_t r, uint8_t g, uint8_t b);

// --------- Internal state ---------
static ChristmasConfig g_cfg = kChristmas;

// Small PRNG for twinkles (deterministic per frame)
static uint32_t frameRand;

// ------------- Helpers -------------
static inline void add60(int idx, uint8_t r, uint8_t g, uint8_t b)
{
	ledhwAdd60(ring60Index(idx%60), r, g, b);
}


static inline void add24(int idx, uint8_t r, uint8_t g, uint8_t b)
{
	ledhwAdd24(ring24Index(idx%24), r, g, b);
}


static void drawTicks()
{
	if(!g_cfg.showMinuteTicks && !g_cfg.showHourTicks) return;

	// minute ticks: very faint warm white
	if(g_cfg.showMinuteTicks)
	{
		for(int i=0;i<60;i++)
		{
			uint8_t br = (i%5==0)? 0 : 10;
			if(br)
			{
				add60(i, qmul8(COLOR_WHITE_R, br), qmul8(COLOR_WHITE_G, br), qmul8(COLOR_WHITE_B, br));
			}
		}
	}
	// hour ticks: brighter
	if(g_cfg.showHourTicks)
	{
		for(int i=0;i<60;i+=5)
		{
			uint8_t br = 40;
			add60(i, qmul8(COLOR_WHITE_R, br), qmul8(COLOR_WHITE_G, br), qmul8(COLOR_WHITE_B, br));
		}
	}
}


static void drawCandyCaneBand(int start, int length)
{
	// Red/white stripes around the minute hand
	for(int k=0;k<length;k++)
	{
		int i = (start + k) % 60;
		bool red = ((k/2)%2)==0; // 2-pixel wide stripes
		if(red)
		{
			add60(i, 180, 20, 20);
		}
		else
		{
			add60(i, 160, 140, 120);
		}
	}
}


static void drawTwinkles(uint32_t seed, uint8_t level)
{
	if(level==0) return;
	// spawn few white sparkles on the 60 ring each frame
	// count scales roughly with level
	int count = 1 + (level/16);
	uint32_t s = seed ^ 0xA5A5BEEF;
	for(int n=0;n<count;n++)
	{
		s = 1664525u*s + 1013904223u;
		int pos = (s>>24)%60;
		s = 1664525u*s + 1013904223u;
								 // 16..79
		uint8_t br = 16 + ((s>>24)&0x3F);
		add60(pos, qmul8(COLOR_WHITE_R, br), qmul8(COLOR_WHITE_G, br), qmul8(COLOR_WHITE_B, br));
	}
}


// Smooth tails for hands
static void drawHandWithTail60(int head, uint8_t r, uint8_t g, uint8_t b, int tailLen, uint8_t falloff)
{
	for(int d=0; d<=tailLen; d++)
	{
		uint8_t f = (d==0)? 255 : (uint8_t)max(0, 255 - d*falloff);
		add60((head - d + 60)%60, qmul8(r,f), qmul8(g,f), qmul8(b,f));
	}
}


// ---------------- Theme API ----------------
static void beginChristmas()
{
	ledhwSetGlobalBrightness(g_cfg.brightness);
}


static void updateChristmas(const tm& now, time_t epoch)
{
	(void)epoch;
	ledhwClearAll();

	// current second/minute/hour positions on 60 ring
	int sec = now.tm_sec % 60;
	int min = now.tm_min % 60;
	int hour12 = now.tm_hour % 12;
								 // hour mapped onto 0..59
	int hourPos60 = (hour12*5 + (min/12)) % 60;

	drawTicks();

	// Candy cane band centered on minute hand
	drawCandyCaneBand((min - 4 + 60)%60, 9);

	// Hands: hour = red w/ short tail, minute = white candy cane band already plus white head, seconds = green w/ longer tail
	// hour
	drawHandWithTail60(hourPos60, COLOR_RED_R, COLOR_RED_G, COLOR_RED_B, 2, 80);
	// minute head
	drawHandWithTail60(min, COLOR_WHITE_R, COLOR_WHITE_G, COLOR_WHITE_B, 1, 120);
	// seconds sweeping tail
	drawHandWithTail60(sec, COLOR_GREEN_R, COLOR_GREEN_G, COLOR_GREEN_B, 6, 30);

	// 24h inner ring: mark current hour in red, next in green for AM/PM hint
	add24(now.tm_hour % 24, 120, 20, 20);
	add24((now.tm_hour+1) % 24, 20, 100, 20);

	// snowy twinkles
	uint32_t ms = hal_millis();
	frameRand = (uint32_t(now.tm_mday)<<24) ^ (uint32_t(now.tm_hour)<<16) ^ (uint32_t(ms/40)<<8) ^ 0x1234;
	drawTwinkles(frameRand, g_cfg.twinkleLevel);

	ledhwShow();
}


static void showStartupPattern(uint8_t r, uint8_t g, uint8_t b)
{
	ledhwSetGlobalBrightness(g_cfg.brightness);
	ledhwClearAll();
	// spin red-green-white wipe
	for(int round=0; round<2; ++round)
	{
		for(int i=0;i<60;i++)
		{
			ledhwClearAll();
			// stripe of 6 pixels cycling R-G-W
			for(int k=0;k<6;k++)
			{
				int j = (i+k)%60;
				uint8_t rr = (k%3==0)? 200:0;
				uint8_t gg = (k%3==1)? 200:0;
				uint8_t bb = (k%3==2)? 180:0;
				add60(j, rr, gg, bb);
			}
			ledhwShow();
			hal_delay_ms(18);
		}
	}
}


// definitie (heeft externe linkage nodig)
extern const Theme THEME_CHRISTMAS =
{
	.id     = "christmas",
	.name   = "Christmas",
	.begin  = beginChristmas,
	.update = updateChristmas,
	.showStartupPattern = showStartupPattern,
};

// Auto-registratie (selecteer eventueel als default met REGISTER_DEFAULT_THEME)
REGISTER_THEME(THEME_CHRISTMAS)
