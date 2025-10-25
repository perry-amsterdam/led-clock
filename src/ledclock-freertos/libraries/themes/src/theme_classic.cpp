#include <Arduino.h>
#include "theme.h"
#include "theme_registry.h"
#include "ledhw.h"
#include "hal_time_freertos.h"
#include "theme_classic.h"

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

static void beginClassic()
{
	ledhwSetGlobalBrightness(kClassic.brightness);
	ledhwClearAll();
	ledhwShow();
}


static void updateClassic(const tm& now, time_t epoch)
{
	(void)epoch;
	ledhwClearAll();

	if (kClassic.showMinuteTicks)
	{
		for (int m=0; m<60; m+=5)
		{
			ledhwAdd60(ring60Index(m), TICK_MIN_R, TICK_MIN_G, TICK_MIN_B);
		}
	}

	if (kClassic.showHourTicks)
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


static void showStatus(ThemeStatus status)
{
	ledhwClearAll();
	switch (status)
	{
		case ThemeStatus::WifiNotConnected:
			for (int i=0; i<24; i+=6) ledhwAdd24(i, 32, 16, 16);
			break;
		case ThemeStatus::PortalActive:
			ledhwAdd60((hal_millis()/40) % 60, 16, 32, 16);
			break;
		case ThemeStatus::TimeNotReady:
			for (int i=0; i<60; i+=5) ledhwAdd60(i, 12,12,12);
			break;
	}
	ledhwShow();
}


static uint16_t frameDelayMs()
{
	return 50;					 // classic iets rustiger
}


// definitie (heeft externe linkage nodig)
extern const Theme THEME_CLASSIC =
{
	.id     = "classic",
	.name   = "Classic",
	.begin  = beginClassic,
	.update = updateClassic,
	.showStatus = showStatus,
	.frameDelayMs = frameDelayMs,
};

// Auto-registratie + markeer als default (of gebruik REGISTER_THEME)
REGISTER_DEFAULT_THEME(THEME_CLASSIC)
