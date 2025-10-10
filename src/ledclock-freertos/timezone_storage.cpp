#include <Preferences.h>
#include <Arduino.h>
#include "timezone_storage.h"

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
