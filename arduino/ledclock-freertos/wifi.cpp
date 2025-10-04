#include <Arduino.h>
#include <ESPmDNS.h>
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

		// --- mDNS: ledclock.local (STA) ---
		if (!MDNS.begin("ledclock"))
		{
			Serial.println("[mDNS] start failed");
		}
		else
		{
			Serial.println("[mDNS] ledclock.local started");
			MDNS.addService("http", "tcp", 80);
		}

		if(DEBUG_NET){ Serial.print("[WiFi] OK IP="); Serial.println(WiFi.localIP()); }
		ledGreen(); return true;
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


bool wpsConnect(uint32_t timeoutMs)
{
	// Alleen core v3+: voorkom per ongeluk bouwen met v2.x
	#if !(defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3))
	#error "Dit project vereist Arduino-ESP32 core v3+. Update je ESP32 core."
	#endif

	Serial.println("[WiFi] SmartConfig starten (core v3+)");
	WiFi.mode(WIFI_STA);
	WiFi.disconnect(true, true);
	WiFi.beginSmartConfig();

	// Wacht tot credentials binnen zijn (via ESP-Touch/AirKiss in de telefoon-app)
	if (!waitFor([]
	{
		return WiFi.smartConfigDone();
	}
	, timeoutMs))
	{
		Serial.println("[WiFi] SmartConfig timeout (geen credentials ontvangen)");
		return false;
	}
	Serial.println("[WiFi] Credentials ontvangen; verbinden...");

	// Wacht op daadwerkelijke connect + IP
	if (!waitFor([]
	{
		return WiFi.status() == WL_CONNECTED;
	}
	, timeoutMs))
	{
		Serial.println("[WiFi] Verbinden mislukt na SmartConfig");
		return false;
	}

	Serial.printf("[WiFi] OK  SSID='%s'  IP=%s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
	ledGreen();

	// SSID bewaren voor UI; wachtwoord laat je aan SmartConfig over
	Preferences prefs;
	prefs.begin(PREF_NS, false);
	prefs.putString("ssid", WiFi.SSID());
	prefs.end();

	return true;
}
