#pragma once
#include <stdint.h>

struct ChristmasConfig
{
    uint8_t brightness;      // 0..255
    bool    showHourTicks;   // show 12 markers on 60-ring
    bool    showMinuteTicks; // show 60 faint markers on 60-ring
    uint8_t twinkleLevel;    // 0..255 how many/bright twinkles
};

// Sensible defaults for a cozy Xmas look
static constexpr ChristmasConfig kChristmas =
{
    40,   // brightness
    true, // hour ticks
    true, // minute ticks
    32,   // twinkle
};
