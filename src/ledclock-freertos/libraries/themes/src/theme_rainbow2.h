#pragma once
#include <stdint.h>

struct Rainbow2Config
{
	uint8_t brightness;
	bool    showHourTicks;
	bool    showMinuteTicks;
};

static constexpr Rainbow2Config kRainbow2 =
{
	64,
	true,
	true,
};
