#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "rtos.h"
#include "globals.h"
#include "task_wifi.h"
#include "wifi.h"
#include "task_portal.h"
#include "portal.h"
#include "hal_time_freertos.h"

void task_wifi(void*)
{
	vTaskDelay(pdMS_TO_TICKS(50));
	LOG_STACK_WATERMARK("wifi:init");

	Preferences prefs;

	// Load saved credentials
	prefs.begin(PREF_NS,true);
	String ssid = prefs.getString("ssid","");
	String pass = prefs.getString("pass","");
	prefs.end();
	// One-shot WPS: read and clear flag
	bool wpsOnce = false;
	prefs.begin(PREF_NS, true);
	wpsOnce = prefs.getBool("wps_once", false);
	prefs.end();
	if(wpsOnce)
	{
		// clear immediately to ensure one-time
		prefs.begin(PREF_NS, false);
		prefs.putBool("wps_once", false);
		prefs.end();

		Serial.println("[WiFi] WPS-once geactiveerd; probeer WPS...");
		if(wpsConnect())
		{
			xEventGroupSetBits(g_sysEvents, EVT_WIFI_UP);
			LOG_STACK_WATERMARK("wifi:ip");
		}
		else
		{
			Serial.println("[WiFi] WPS mislukt; ga door met normale flow");
		}
	}

	// If connected via WPS, skip normal connection attempts
	if (WiFi.status()==WL_CONNECTED)
	{
		for(;;)
		{
			LOG_STACK_WATERMARK("wifi:loop");

			// keep task alive, no portal
			hal_delay_ms(250);
		}
	}

	if(ssid.isEmpty())
	{
		// Start portal immediately
		xEventGroupSetBits(g_sysEvents, EVT_PORTAL_ON);
		startPortal();
	}

	for(;;)
	{
		if(!rtos_test_bits(EVT_WIFI_UP) && !rtos_test_bits(EVT_PORTAL_ON))
		{
			connectWiFi(ssid, pass);

			// Wait up to 10s
			uint32_t t0 = hal_millis();
			while(hal_millis()-t0 < 10000 && WiFi.status()!=WL_CONNECTED)
			{
				hal_delay_ms(50);
			}

			if(WiFi.status()==WL_CONNECTED)
			{
				xEventGroupSetBits(g_sysEvents, EVT_WIFI_UP);
			}
			else
			{
				// Fallback to portal
				xEventGroupSetBits(g_sysEvents, EVT_PORTAL_ON);
				startPortal();
			}
		}
		hal_delay_ms(250);
	}
}
