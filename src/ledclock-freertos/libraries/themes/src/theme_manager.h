#pragma once
#include <cstddef>
#include "theme.h"

void        themeInit();
void        themeUpdate(const tm& now, time_t epoch);
void        themeshowStartupPattern(uint8_t r, uint8_t g, uint8_t b);

size_t      themeCount();
const char* themeIdAt(size_t i);   
const char* themeNameAt(size_t i);
const char* themeCurrentId();  
const char* themeCurrentName();

// Backwards compat:
bool        themeSelect(const char* name);

// Preferred:
bool        themeSelectById(const char* id);  
bool        themeSelectDefault();              

