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

static void beginRainbow()
{
	ledhwSetGlobalBrightness(kRainbow.brightness);
	ledhwClearAll();
	ledhwShow();
}


// Functie to display status during startup on the leds (24 en 60).
static void showStartupPattern(uint8_t r, uint8_t g, uint8_t b)
{
	ledhwClearAll();

	for (int index = 0; index < 60; index += 5)
	{
		// Positie = (statische offset + lus-index)
		ledhwAdd60(ring60Index(index), r, g, b);
	}

	for (int index = 0; index < 24; index += 3)
	{
		// Positie = (statische offset + lus-index)
		ledhwAdd24(ring24Index(index), r, g, b);
	}

	ledhwShow();

	hal_delay_ms(500);

	ledhwClearAll();
	ledhwShow();
}


static void updateRainbow(const tm& now, time_t epoch)
{
	(void)epoch;
	ledhwClearAll();

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

	ledhwAdd60(ring60Index(s), COLOR_SEC_R, COLOR_SEC_G, COLOR_SEC_B);
	ledhwAdd60(ring60Index(m), COLOR_MIN_R, COLOR_MIN_G, COLOR_MIN_B);
	ledhwAdd24(ring24Index(h*2), COLOR_HOUR_R, COLOR_HOUR_G, COLOR_HOUR_B);

	ledhwShow();
}


// definitie (heeft externe linkage nodig)
extern const Theme THEME_RAINBOW =
{
	.id     = "theme_rainbow",
	.name   = "Rainbow",
	.begin  = beginRainbow,
	.update = updateRainbow,
	.showStartupPattern = showStartupPattern,
};

// Auto-registratie + markeer als default (of gebruik REGISTER_THEME)
//REGISTER_DEFAULT_THEME(THEME_RAINBOW)
