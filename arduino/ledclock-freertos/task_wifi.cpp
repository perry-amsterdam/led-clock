#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>

#include "rtos.h"
#include "globals.h"
#include "task_wifi.h"
#include "task_http.h"
#include "wifi.h"				 // connectWiFi(...)
#include "portal.h"				 // startPortal/stopPortal
#include "http_api.h"			 // startApi/stopApi
#include "hal_time_freertos.h"

// This task attempts to connect using saved credentials once,
// then supervises link state and keeps the portal/API mutually exclusive.
void task_wifi(void*)
{
	vTaskDelay(pdMS_TO_TICKS(100));

	// ----- 1) Try to connect using saved credentials (from NVS) -----
	bool connected = false;
	String ssid, pass;

	// Use the global Preferences instance declared in globals.cpp
	prefs.begin(PREF_NS, true);	 // read-only
	ssid = prefs.getString(PREF_SSID, "");
	pass = prefs.getString(PREF_PASS, "");
	prefs.end();

	if (!ssid.isEmpty())
	{
		Serial.printf("[WiFi] Attempting to connect to saved SSID '%s'...\n", ssid.c_str());
								 // 15s timeout
		connected = connectWiFi(ssid, pass, 15000);
	}
	else
	{
		Serial.println("[WiFi] No saved credentials found.");
	}

	// ----- 2) Bring up the right side based on result -----
	if (connected)
	{
		Serial.println("[WiFi] Connected. Starting API...");
		stopPortal();			 // make sure portal is OFF
		startApi();				 // start REST API + mDNS
	}
	else
	{
		Serial.println("[WiFi] Not connected. Starting captive portal...");
		stopApi();				 // make sure API is OFF
		startPortal();			 // AP + DNS + setup UI
	}

	bool last_link_up = connected;

	// ----- 3) Supervisor loop: react to link changes -----
	for (;;)
	{
		const bool link_up = (WiFi.status() == WL_CONNECTED);

		if (link_up && !last_link_up)
		{
			Serial.println("[WiFi] Connection restored.");
			stopPortal();
			startApi();
			LOG_STACK_WATERMARK("wifi:ip");
		}
		else if (!link_up && last_link_up)
		{
			Serial.println("[WiFi] Connection lost.");
			stopApi();
			startPortal();
		}

		last_link_up = link_up;
		hal_delay_ms(500);
	}
}
