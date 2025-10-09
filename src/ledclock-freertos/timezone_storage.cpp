#include <Preferences.h>
#include <Arduino.h>

// ======================================================
// Tijdzone opslag in NVS (Non-Volatile Storage)
// ======================================================

String nvsReadTimezone()
{
	Preferences p;
	if (!p.begin("sys", true)) return "";
	String tz = p.getString("tz", "");
	p.end();
	return tz;
}

void nvsWriteTimezone(const String& tz)
{
	Preferences p;
	if (p.begin("sys", false))
	{
		p.putString("tz", tz);
		p.end();
	}
}
