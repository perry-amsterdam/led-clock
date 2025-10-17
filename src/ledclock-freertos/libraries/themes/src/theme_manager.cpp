#include "theme.h"
#include "theme_manager.h"
#include "theme_registry.h"
#include "theme_manager.h"

// Huidige thema pointer
static const Theme* currentTheme = nullptr;

void themeInit(const char* id)
{
	if (id == nullptr || id[0] == '\0')
	{
		themeSelectDefault();
	}
	else
	{
		if (!themeSelectById(id))
		{
			themeSelectDefault();
		}
	}
}


size_t themeCount()
{
	return ThemeRegistry::size();
}


const char* themeIdAt(size_t i)
{
	auto items = ThemeRegistry::items();
	return (i < ThemeRegistry::size() && items[i]) ? items[i]->id : nullptr;
}


const char* themeNameAt(size_t i)
{
	auto items = ThemeRegistry::items();
	return (i < ThemeRegistry::size() && items[i]) ? items[i]->name : nullptr;
}


const Theme* themeCurrent()
{
	return currentTheme;
}


const char* themeCurrentId()
{
	return currentTheme ? currentTheme->id : "";
}


const char* themeCurrentName()
{
	return currentTheme ? currentTheme->name : "";
}


bool themeSelectById(const char* id)
{
	if (id == nullptr || id[0] == '\0')
	{
		return false;
	}
	const Theme* t = ThemeRegistry::findById(id);
	if (!t) return false;
	if (currentTheme == t) return true;
	currentTheme = t;
	if (currentTheme && currentTheme->begin) currentTheme->begin();
	return true;
}


// Backwards compat  selecteren op display-naam
bool themeSelect(const char* name)
{
	const Theme* t = ThemeRegistry::findByName(name);
	if (!t) return false;
	currentTheme = t;
	if (currentTheme->begin) currentTheme->begin();
	return true;
}


bool themeSelectDefault()
{
	const Theme* def = ThemeRegistry::getDefault();
	if (!def) return false;
	if (currentTheme == def) return true;
	currentTheme = def;
	if (currentTheme && currentTheme->begin) currentTheme->begin();
	return true;
}


void themeUpdate(const tm& now, time_t epoch)
{
	if (currentTheme && currentTheme->update)
	{
		currentTheme->update(now, epoch);
	}
}


void themeShowStartupPattern(uint8_t r, uint8_t g, uint8_t b)
{
	if (currentTheme && currentTheme->showStartupPattern)
	{
		currentTheme->showStartupPattern(r,g,b);
	}
}


const Theme* themeDefault()
{
	return ThemeRegistry::getDefault();
}


const Theme* const* themeList(size_t* count)
{
	// Return the registry's internal array directly; caller must use *count to know valid entries.
	if (count) *count = ThemeRegistry::size();
	return ThemeRegistry::items();
}


bool themeExists(const char* themeId)
{
	size_t count;
	const Theme* themes = themeList(&count);
	for (size_t i = 0; i < count; i++)
	{
		if (strcmp(themes[i]->id, themeId) == 0)
		{
			return true;
		}
	}
	return false;
}
