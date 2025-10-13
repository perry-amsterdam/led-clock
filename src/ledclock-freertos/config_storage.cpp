#include <Preferences.h>
#include <Arduino.h>
#include "config_storage.h"

static const char* NS_SYS         = "sys";
static const char* KEY_TZ_USER    = "tz_user";
static const char* KEY_TZ_USERSET = "tz_user_set";

bool tz_user_is_set()
{
	Preferences p;
	if (!p.begin(NS_SYS, /*readOnly*/ true)) return false;
	// default = 0
	uint8_t v = p.getUChar(KEY_TZ_USERSET, 0);
	p.end();
	return v != 0;
}


bool tz_user_get(String& out)
{
	Preferences p;
	if (!p.begin(NS_SYS, /*readOnly*/ true)) { out = ""; return false; }
	String tz = p.getString(KEY_TZ_USER, "");
	p.end();
	out = tz;
	return tz.length() > 0;
}


void tz_user_set(const String& tz)
{
	Preferences p;
	if (!p.begin(NS_SYS, /*readOnly*/ false)) return;
	p.putString(KEY_TZ_USER, tz);
	// bit zetten
	p.putUChar(KEY_TZ_USERSET, 1);
	p.end();
}


void tz_user_clear()
{
	Preferences p;
	if (!p.begin(NS_SYS, /*readOnly*/ false)) return;
	// Leeg de string en reset flag
	p.putString(KEY_TZ_USER, "");
	// default = 0
	p.putUChar(KEY_TZ_USERSET, 0);
	p.end();
}
//
////
//// Function to save and load theme stettings.
////
//
//static const char* PREF_NS        = "settings";
//static const char* PREF_THEME_KEY = "theme_id";
//
//static bool saveThemeId(const String& id)
//{
//	Preferences p;
//	if (!p.begin(PREF_NS, false)) return false;
//	bool ok = p.putString(PREF_THEME_KEY, id) > 0;
//	p.end();
//	return ok;
//}
//
//
//static String loadThemeId()
//{
//	Preferences p;
//	if (!p.begin(PREF_NS, true)) return String();
//	String id = p.getString(PREF_THEME_KEY, "");
//	p.end();
//	return id;
//}
//
//
//static void clearSavedTheme()
//{
//	Preferences p;
//	if (p.begin(PREF_NS, false))
//	{
//		p.remove(PREF_THEME_KEY);
//		p.end();
//	}
//}
