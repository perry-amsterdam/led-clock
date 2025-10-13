#pragma once
#include <stdint.h>

struct RainbowConfig
{
	uint8_t brightness;
	bool    showHourTicks;
	bool    showMinuteTicks;
};

static constexpr RainbowConfig kRainbow =
{
	64,
	true,
	true,
};
