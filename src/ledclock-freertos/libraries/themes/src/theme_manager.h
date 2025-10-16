#pragma once

// Compact, const-correct theme manager interface.
// Drop-in replacement for libraries/themes/src/theme_manager.h

struct Theme; // forward declaration

// Initialize the theme system (loads last-used theme if available)
void        themeInit(const char* id);

// Select a theme by string id; returns true on success
bool        themeSelectById(const char* id);

// Select the default theme; returns true on success
bool        themeSelectDefault();

// Accessor for the currently selected theme (read-only)
const Theme* themeCurrent();
