#include <Arduino.h>
#include <WiFi.h>
#include "wifi.h"
#include "portal.h"
#include "status_led.h"
#include "net_time.h"
#include "hal_time_freertos.h"
#include <Preferences.h>

bool connectWiFi(const String& ssid, const String& pass, uint32_t timeoutMs)
{
	if(ssid.isEmpty())
	{
		Serial.println("[WiFi] geen opgeslagen SSID");
		return false;
	}

	if(DEBUG_NET) Serial.printf("\r[WiFi] verbinden met SSID='%s'...\n", ssid.c_str());

	WiFi.mode(WIFI_STA);
	WiFi.disconnect(true,true);
	hal_delay_ms(50);
	WiFi.begin(ssid.c_str(), pass.c_str());

	uint32_t start=hal_millis(); bool blink=false;
	while(WiFi.status()!=WL_CONNECTED && (hal_millis()-start)<timeoutMs)
	{
		hal_delay_ms(300);
	}

	if(WiFi.status()==WL_CONNECTED)
	{
		if(DEBUG_NET){ Serial.print("[WiFi] OK IP="); Serial.println(WiFi.localIP()); }
		return true;
	}

	Serial.println("[WiFi] FAILED"); return false;
}


// Klein hulpfunctietje om met timeout te wachten
static bool waitFor(std::function<bool()> pred, uint32_t timeoutMs, uint32_t stepMs = 100)
{
	const uint32_t start = hal_millis();
	while (!pred())
	{
		if (hal_millis() - start >= timeoutMs) return false;
		hal_delay_ms(stepMs);
		yield();
	}
	return true;
}


static Preferences prefs;

bool wpsConnect(uint32_t timeoutMs)
{
	Serial.println("Starting WPS...");

	// Start WPS (Push Button Config)
	if (!WiFi.beginWPSConfig())
	{
		Serial.println("WPS failed to start.");
		return false;
	}

	uint32_t start = millis();
	while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs)
	{
		delay(500);
		Serial.print(".");
	}
	Serial.println();

	if (WiFi.status() == WL_CONNECTED)
	{
		Serial.printf("WPS success. SSID='%s' IP=%s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

		// SSID en wachtwoord opslaan in NVS
		prefs.begin(PREF_NS, false);
		prefs.putString("ssid", WiFi.SSID());
		prefs.putString("pass", WiFi.psk());
		prefs.end();

		return true;
	}
	else
	{
		Serial.println("WPS failed: no connection.");
		return false;
	}
}
