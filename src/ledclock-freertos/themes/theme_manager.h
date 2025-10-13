#pragma once
#include <cstddef>
#include "themes/theme.h"

void        themeInit();
void        themeUpdate(const tm& now, time_t epoch);
void        themeshowStartupPattern(uint8_t r, uint8_t g, uint8_t b);

size_t      themeCount();
const char* themeNameAt(size_t i);
const char* themeCurrentName();
bool        themeSelect(const char* name);
