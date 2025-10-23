#include <Arduino.h>
#include "theme.h"
#include "theme_registry.h"
#include "hal_time_freertos.h"
#include "ledhw.h"
#include "theme_christmas.h"

// ---------- Color helpers (0..255) ----------
static inline uint8_t qadd8(uint8_t a, uint8_t b){ uint16_t s = uint16_t(a)+uint16_t(b); return (s>255)?255:uint8_t(s); }
static inline uint8_t qmul8(uint8_t a, uint8_t b){ return uint8_t((uint16_t(a)*uint16_t(b))/255); }

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
#define TICK_MIN_G  20
#endif
#ifndef TICK_MIN_B
#define TICK_MIN_B  20
#endif

#ifndef TICK_HOUR_R
#define TICK_HOUR_R  20
#endif
#ifndef TICK_HOUR_G
#define TICK_HOUR_G  20
#endif
#ifndef TICK_HOUR_B
#define TICK_HOUR_B  20
#endif

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

	if (g_cfg.showMinuteTicks)
	{
		for (int m=0; m<60; m+=5)
		{
			ledhwAdd60(ring60Index(m), TICK_MIN_R, TICK_MIN_G, TICK_MIN_B);
		}
	}

	if (g_cfg.showHourTicks)
	{
		for (int h=0; h<12; h+=3)
		{
			ledhwAdd24(ring24Index(h*2), TICK_HOUR_R, TICK_HOUR_G, TICK_HOUR_B);
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


static void drawTwinkles(uint32_t seed, uint8_t level, uint8_t speed)
{
	if(level==0) return;

	// Hoe hoger speed, hoe vaker de twinkles wisselen.
	// Gebruik speed om de random seed per frame te variren.
								 // hogere speed => sneller wisselen
	uint32_t divisor = (uint32_t)((64 - speed) ? (64 - speed) : 1);
	uint32_t frameSeed = seed + (hal_millis() / divisor);

	int count60 = 1 + (level/16);
	int count24 = max(1, level/32);

	uint32_t s = frameSeed ^ 0xA5A5BEEF;

	// 60-ring twinkles
	for(int n=0; n<count60; n++)
	{
		s = 1664525u*s + 1013904223u;
		int pos = (s>>24)%60;
		s = 1664525u*s + 1013904223u;
		uint8_t br = 16 + ((s>>24)&0x3F);
		add60(pos, qmul8(COLOR_WHITE_R, br), qmul8(COLOR_WHITE_G, br), qmul8(COLOR_WHITE_B, br));
	}

	// 24-ring twinkles
	for(int n=0; n<count24; n++)
	{
		s = 1664525u*s + 1013904223u;
		int pos = (s>>24)%24;
		s = 1664525u*s + 1013904223u;
		uint8_t br = 12 + ((s>>24)&0x33);
		add24(pos, qmul8(COLOR_WHITE_R, br), qmul8(COLOR_WHITE_G, br), qmul8(COLOR_WHITE_B, br));
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

	// Candy cane band centered on minute hand
	//drawCandyCaneBand((min - 4 + 60)%60, 9);

	g_cfg.twinkleLevel = 64;	 // gematigde snelheid
	g_cfg.twinkleSpeed = 10;	 // gematigde snelheid

	// snowy twinkles
	uint32_t ms = hal_millis();
	frameRand = (uint32_t(now.tm_mday)<<24) ^ (uint32_t(now.tm_hour)<<16) ^ (uint32_t(ms/40)<<8) ^ 0x1234;
	drawTwinkles(frameRand, g_cfg.twinkleLevel, g_cfg.twinkleSpeed);

	drawTicks();

	const int s = now.tm_sec % 60;
	const int m = now.tm_min % 60;
	const int h = now.tm_hour % 12;

	ledhwAdd60(ring60Index(sec), COLOR_SEC_R, COLOR_SEC_G, COLOR_SEC_B);
	ledhwAdd60(ring60Index(min), COLOR_MIN_R, COLOR_MIN_G, COLOR_MIN_B);
	ledhwAdd24(ring24Index(hour12*2), COLOR_HOUR_R, COLOR_HOUR_G, COLOR_HOUR_B);

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


static void showStatus(ThemeStatus status)
{
	ledhwClearAll();
	switch (status)
	{
		case ThemeStatus::WifiNotConnected:
								 // rood
			ledhwAdd24(0, 32, 0, 0);
			break;
		case ThemeStatus::PortalActive:
								 // groen
			ledhwAdd24(6, 0, 32, 0);
			break;
		case ThemeStatus::TimeReady:
								 // wit
			for (int i=0; i<24; i+=6) ledhwAdd24(i, 16,16,16);
			break;
	}
	ledhwShow();
}


static uint16_t frameDelayMs()
{
	return 40;					 // iets sneller dan classic
}


// definitie (heeft externe linkage nodig)
extern const Theme THEME_CHRISTMAS =
{
	.id     = "christmas",
	.name   = "Christmas",
	.begin  = beginChristmas,
	.update = updateChristmas,
	.showStartupPattern = showStartupPattern,
	.showStatus = showStatus,
	.frameDelayMs = frameDelayMs,
};

// Auto-registratie (selecteer eventueel als default met REGISTER_DEFAULT_THEME)
REGISTER_THEME(THEME_CHRISTMAS)
