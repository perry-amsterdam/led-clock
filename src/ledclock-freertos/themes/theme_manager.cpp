#include "theme.h"
#include "theme_registry.h"

// Huidige thema pointer
static const Theme* currentTheme = nullptr;

void themeInit()
{
	// eerste of expliciete default
	currentTheme = ThemeRegistry::getDefault();
	if (currentTheme && currentTheme->begin) currentTheme->begin();
}


size_t themeCount()
{
	return ThemeRegistry::size();
}


const Theme* themeByIndex(size_t idx)
{
	if (idx >= ThemeRegistry::size()) return nullptr;
	return ThemeRegistry::items()[idx];
}


bool themeSelectByIndex(size_t idx)
{
	const Theme* t = themeByIndex(idx);
	if (!t) return false;
	currentTheme = t;
	if (currentTheme->begin) currentTheme->begin();
	return true;
}


const char* themeName(size_t idx)
{
	const Theme* t = themeByIndex(idx);
	return t ? t->name : "";
}


void themeUpdate(const tm& now, time_t epoch)
{
	if (currentTheme && currentTheme->update)
	{
		currentTheme->update(now, epoch);
	};
}


void themeshowStartupPattern(uint8_t r, uint8_t g, uint8_t b)
{
	if (currentTheme && currentTheme->showStartupPattern)
	{
		currentTheme->showStartupPattern(r,g,b);
	};
}
