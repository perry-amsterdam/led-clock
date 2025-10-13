#pragma once
#include <stdint.h>

struct ClassicConfig
{
	uint8_t brightness;
	bool    showHourTicks;
	bool    showMinuteTicks;
};

static constexpr ClassicConfig kClassic =
{
	16,
	true,
	true,
};
