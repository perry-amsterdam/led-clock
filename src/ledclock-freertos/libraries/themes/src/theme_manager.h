#pragma once

#include <time.h>
#include <stdint.h>

struct Theme;	// forward declaration

// Initialize the theme system (loads last-used theme if available)
void        themeInit(const char* id);

// Select a theme by string id; returns true on success
bool        themeSelectById(const char* id);

// Select the default theme; returns true on success
bool        themeSelectDefault();

// Accessor for the currently selected theme (read-only)
const Theme* themeCurrent();

// Accessor for the currently selected theme (read-only)
const Theme* themeCurrent();

// Update de huidige theme (animation tick)
void themeUpdate(const tm& now, time_t epoch);

// Optioneel: startuppatroon laten zien in theme-stijl
void themeShowStartupPattern(uint8_t r, uint8_t g, uint8_t b);
