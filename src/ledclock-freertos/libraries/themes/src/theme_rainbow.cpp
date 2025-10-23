#include <Arduino.h>
#include "theme.h"
#include "theme_registry.h"
#include "hal_time_freertos.h"
#include "ledhw.h"
#include "theme_rainbow.h"

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
#define COLOR_MIN_G   150
#endif
#ifndef COLOR_MIN_B
#define COLOR_MIN_B   0
#endif

#ifndef COLOR_HOUR_PM_R
#define COLOR_HOUR_PM_R  150
#endif
#ifndef COLOR_HOUR_PM_G
#define COLOR_HOUR_PM_G  0
#endif
#ifndef COLOR_HOUR_PM_B
#define COLOR_HOUR_PM_B  0
#endif

#ifndef COLOR_HOUR_AM_R
#define COLOR_HOUR_AM_R  0
#endif
#ifndef COLOR_HOUR_AM_G
#define COLOR_HOUR_AM_G  150
#endif
#ifndef COLOR_HOUR_AM_B
#define COLOR_HOUR_AM_B  0
#endif

// ---------------------------------------------------
// Tick-kleuren (warm wit, gedimd standaard)
// - Minuten-ticks op 60-ring (elke 5)
// - Uur-ticks op 24-ring (elke 3)
// Tip: verhoog/verlaag alle drie samen als je ze feller/zachter wil.
// ---------------------------------------------------
#ifndef TICK_MIN_R
#define TICK_MIN_R  30
#endif
#ifndef TICK_MIN_G
#define TICK_MIN_G  24
#endif
#ifndef TICK_MIN_B
#define TICK_MIN_B  15
#endif

#ifndef TICK_HOUR_R
#define TICK_HOUR_R  30
#endif
#ifndef TICK_HOUR_G
#define TICK_HOUR_G  24
#endif
#ifndef TICK_HOUR_B
#define TICK_HOUR_B  15
#endif

#include <Arduino.h>
#include "ledhw.h"
#include "theme_rainbow.h"

// ---- HSV  RGB helper -------------------------------------------------------
// Snel integer-gebaseerde omzetting voor WS2812B (0..255 bereik)
static inline void hsvToRgb(uint8_t h, uint8_t s, uint8_t v,
uint8_t& r, uint8_t& g, uint8_t& b)
{
	if (s == 0) { r = g = b = v; return; }
	uint8_t region = h / 43;	 // 0..5
	uint8_t remainder = (h - (region * 43)) * 6;

	uint8_t p = (uint16_t(v) * (255 - s)) >> 8;
	uint8_t q = (uint16_t(v) * (255 - ((uint16_t(s) * remainder) >> 8))) >> 8;
	uint8_t t = (uint16_t(v) * (255 - ((uint16_t(s) * (255 - remainder)) >> 8))) >> 8;

	switch (region)
	{
		default:
		case 0: r = v; g = t; b = p; break;
		case 1: r = q; g = v; b = p; break;
		case 2: r = p; g = v; b = t; break;
		case 3: r = p; g = q; b = v; break;
		case 4: r = t; g = p; b = v; break;
		case 5: r = v; g = p; b = q; break;
	}
}


// ---- Rainbow-renderer -------------------------------------------------------
// Tekent een regenboog over beide ringen.
// - hueOffset schuift het kleurenspectrum (0..255)
// - brightness (0..255) bepaalt de intensiteit.
void renderRainbow(uint8_t hueOffset, uint8_t brightness)
{
	ledhwClearAll();

	// 60-LED ring
	for (int i = 0; i < 60; ++i)
	{
		uint8_t hue = uint8_t((uint16_t(i) * 256 / 60) + hueOffset);
		uint8_t r, g, b;
		hsvToRgb(hue, 255, brightness, r, g, b);
		ledhwAdd60(ring60Index(i), r, g, b);
	}

	// 24-LED ring
	for (int i = 0; i < 24; ++i)
	{
		uint8_t hue = uint8_t((uint16_t(i) * 256 / 24) + hueOffset);
		uint8_t r, g, b;
		hsvToRgb(hue, 255, brightness, r, g, b);
		ledhwAdd24(ring24Index(i), r, g, b);
	}
}


static void beginRainbow()
{
	ledhwSetGlobalBrightness(kRainbow.brightness);
	ledhwClearAll();
	ledhwShow();
}


static void updateRainbow(const tm& now, time_t epoch)
{
	(void)epoch;
	ledhwClearAll();

	static uint8_t hue = 0;
	renderRainbow(hue, 4);		 // brightness = 5
	hue += 1;					 // schuift de regenboog

	if (kRainbow.showMinuteTicks)
	{
		for (int m=0; m<60; m+=5)
		{
			ledhwAdd60(ring60Index(m), TICK_MIN_R, TICK_MIN_G, TICK_MIN_B);
		}
	}

	if (kRainbow.showHourTicks)
	{
		for (int h=0; h<12; h+=3)
		{
			ledhwAdd24(ring24Index(h*2), TICK_HOUR_R, TICK_HOUR_G, TICK_HOUR_B);
		}
	}

	const int s = now.tm_sec % 60;
	const int m = now.tm_min % 60;
	const int h = now.tm_hour % 12;
	const int h24 = now.tm_hour % 24;

	ledhwAdd60(ring60Index(s), COLOR_SEC_R, COLOR_SEC_G, COLOR_SEC_B);
	ledhwAdd60(ring60Index(m), COLOR_MIN_R, COLOR_MIN_G, COLOR_MIN_B);

	if (h24 > 12)
	{
		ledhwAdd24(ring24Index(h*2), COLOR_HOUR_AM_R, COLOR_HOUR_AM_G, COLOR_HOUR_AM_B);
	}
	else
	{
		ledhwAdd24(ring24Index(h*2), COLOR_HOUR_PM_R, COLOR_HOUR_PM_G, COLOR_HOUR_PM_B);
	}

	ledhwShow();
}


static void showStatus(ThemeStatus status)
{
	ledhwClearAll();
	uint8_t h = (hal_millis()/6) & 0xFF;
	switch (status)
	{
		case ThemeStatus::WifiNotConnected:
			renderRainbow(h, 32);
			break;
		case ThemeStatus::PortalActive:
			renderRainbow(h*2, 48);
			break;
		case ThemeStatus::TimeReady:
			for (int i=0;i<60;i+=5) ledhwAdd60(i, 24,24,24);
			break;
	}
	ledhwShow();
}


static uint16_t frameDelayMs()
{
	return 33;					 // ~30 FPS
}


// definitie (heeft externe linkage nodig)
extern const Theme THEME_RAINBOW =
{
	.id     = "rainbow",
	.name   = "Rainbow",
	.begin  = beginRainbow,
	.update = updateRainbow,
	.showStatus = showStatus,
	.frameDelayMs = frameDelayMs,
};

// Auto-registratie + markeer als default (of gebruik REGISTER_THEME)
REGISTER_THEME(THEME_RAINBOW)
