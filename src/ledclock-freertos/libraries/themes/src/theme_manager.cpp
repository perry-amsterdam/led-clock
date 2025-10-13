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

const char* themeCurrentId()
{
    return currentTheme ? currentTheme->id : nullptr;
}

const char* themeCurrentName()
{
    return currentTheme ? currentTheme->name : nullptr;
}

bool themeSelectById(const char* id)
{
    auto t = ThemeRegistry::findById(id);
    if (!t) return false;
    currentTheme = t;
    if (currentTheme->begin) currentTheme->begin();
    return true;
}

// Backwards compat – selecteren op display-naam
bool themeSelect(const char* name)
{
    auto t = ThemeRegistry::findByName(name);
    if (!t) return false;
    currentTheme = t;
    if (currentTheme->begin) currentTheme->begin();
    return true;
}

bool themeSelectDefault()
{
    auto t = ThemeRegistry::getDefault();
    if (!t) return false;
    currentTheme = t;
    if (currentTheme->begin) currentTheme->begin();
    return true;
}

void themeUpdate(const tm& now, time_t epoch)
{
    if (currentTheme && currentTheme->update) {
        currentTheme->update(now, epoch);
    }
}

void themeshowStartupPattern(uint8_t r, uint8_t g, uint8_t b)
{
    if (currentTheme && currentTheme->showStartupPattern) {
        currentTheme->showStartupPattern(r,g,b);
    }
}

