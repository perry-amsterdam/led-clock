#pragma once
#include <Arduino.h>

// Timezone (IANA string)
bool tz_user_is_set();			 // true als er een niet-lege TZ in NVS staat
bool tz_user_get(String& out);	 // true als gevonden (out gevuld)
bool tz_user_set(const String& tz);
bool tz_user_clear();			 // true bij succes

// Theme (id)
bool theme_is_set();			 // true als een theme-id opgeslagen is
bool saveThemeId(const String& id);
bool loadThemeId(String& out);	 // true als gevonden
bool clearSavedTheme();			 // true bij succes
