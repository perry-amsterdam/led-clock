#include <Preferences.h>
#include <Arduino.h>
#include "config_storage.h"

// Overweeg een globale mutex als je meerdere taken hebt:
// static SemaphoreHandle_t g_cfgMutex = xSemaphoreCreateMutex();

namespace
{
	constexpr const char* PREF_NS        = "sys";

	// Timezone keys
	constexpr const char* KEY_TZ_USER    = "tz_user";

	// Theme key
	constexpr const char* KEY_THEME_ID   = "theme_id";

	// Hulpfuncties
	bool openPrefsRO(Preferences& p)
	{
		return p.begin(PREF_NS, true);
	}
	bool openPrefsRW(Preferences& p)
	{
		return p.begin(PREF_NS, false);
	}

	// Schrijf alleen als anders om slijtage te beperken
	bool writeIfChanged(Preferences& p, const char* key, const String& value)
	{
		String current = p.getString(key, "");
		// niets te doen
		if (current == value) return true;
		return p.putString(key, value) > 0;
	}

	bool isNonEmpty(const String& s) { return s.length() > 0; }
}


// -------- Timezone --------
bool tz_user_is_set()
{
	Preferences p;
	if (!openPrefsRO(p)) return false;
	String tz = p.getString(KEY_TZ_USER, "");
	p.end();
	return isNonEmpty(tz);
}


bool tz_user_get(String& out)
{
	Preferences p;
	if (!openPrefsRO(p)) return false;
	out = p.getString(KEY_TZ_USER, "");
	p.end();
	return isNonEmpty(out);
}


bool tz_user_set(const String& tz)
{
	// Optionele eenvoudige validatie: moet "Region/City" bevatten
	if (tz.indexOf('/') <= 0)
	{
		// desgewenst toch opslaan; ik kies hier voor "false" bij duidelijke ongeldige vorm
		return false;
	}

	Preferences p;
	if (!openPrefsRW(p)) return false;
	bool ok = writeIfChanged(p, KEY_TZ_USER, tz);
	p.end();
	return ok;
}


bool tz_user_clear()
{
	Preferences p;
	if (!openPrefsRW(p)) return false;
	bool existed = p.isKey(KEY_TZ_USER);
	if (existed) p.remove(KEY_TZ_USER);
	p.end();
	return true;				 // remove geeft geen status; we nemen aan dat end() gelukt is
}


// -------- Theme --------
bool theme_is_set()
{
	Preferences p;
	if (!p.begin("sys", true)) return false;
	String id = p.getString("theme_id", "");
	p.end();
	return id.length() > 0;
}


bool saveThemeId(const String& id)
{
	// eenvoudige validatie: geen spaties, max 64 chars
	if (id.length() == 0 || id.length() > 64 || id.indexOf(' ') >= 0) return false;

	Preferences p;
	if (!openPrefsRW(p)) return false;
	bool ok = writeIfChanged(p, KEY_THEME_ID, id);
	p.end();
	return ok;
}


bool loadThemeId(String& out)
{
	Preferences p;
	if (!openPrefsRO(p)) return false;
	out = p.getString(KEY_THEME_ID, "");
	p.end();
	return isNonEmpty(out);
}


bool clearSavedTheme()
{
	Preferences p;
	if (!openPrefsRW(p)) return false;
	bool existed = p.isKey(KEY_THEME_ID);
	if (existed) p.remove(KEY_THEME_ID);
	p.end();
	return true;
}
