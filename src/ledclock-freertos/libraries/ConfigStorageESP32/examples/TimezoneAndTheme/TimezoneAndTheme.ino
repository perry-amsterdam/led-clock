#include <Arduino.h>
#include <config_storage.h>

void setup()
{
	Serial.begin(115200);
	delay(100);

	// Set a timezone once
	if (!tz_user_is_set())
	{
		tz_user_set("Europe/Amsterdam");
	}

	// Load timezone
	String tz;
	if (tz_user_get(tz))
	{
		Serial.print("Timezone: ");
		Serial.println(tz);
	}

	// Save and load a theme id
	saveThemeId("classic");
	String theme;
	if (loadThemeId(theme))
	{
		Serial.print("Theme: ");
		Serial.println(theme);
	}
}


void loop()
{
}
