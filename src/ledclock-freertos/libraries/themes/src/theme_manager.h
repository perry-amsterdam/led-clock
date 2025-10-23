#pragma once

#include <time.h>
#include <stdint.h>

struct Theme;					 // forward declaration

// Initialize the theme system (loads last-used theme if available)
void        themeInit(const char* id);

// Select a theme by string id; returns true on success
bool        themeSelectById(const char* id);

// Select the default theme; returns true on success
bool        themeSelectDefault();

// Accessor for the default theme (read-only)
const Theme* themeDefault();

// Accessor for the currently selected theme (read-only)
const Theme* themeCurrent();

// Update de huidige theme (animation tick)
void themeUpdate(const tm& now, time_t epoch);

const Theme* const* themeList(size_t* count);

bool themeExists(const char* themeId);

void themeShowStatus(ThemeStatus status);

uint16_t themeFrameDelayMs();
