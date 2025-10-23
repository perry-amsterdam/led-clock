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
	if (!themeId) return false;
	size_t count = 0;

	// themeList returns an array of pointers to Theme
	const Theme* const* themes = themeList(&count);
	for (size_t i = 0; i < count; ++i)
	{
		const Theme* t = themes[i];
		if (t && t->id && strcmp(t->id, themeId) == 0)
		{
			return true;
		}
	}
	return false;
}


void themeShowStatus(ThemeStatus status)
{
	if (currentTheme && currentTheme->showStatus)
	{
		currentTheme->showStatus(status);
	}
}


uint16_t themeFrameDelayMs()
{
	if (currentTheme && currentTheme->frameDelayMs)
	{
		return currentTheme->frameDelayMs();
	}
	return 33; // default ~30 FPS
}
