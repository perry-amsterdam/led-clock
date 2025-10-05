#include "http.h"
#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WebServer.h>
extern WebServer server;
extern void handlePing();
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
		ledGreen(); return true;
	}

	Serial.println("[WiFi] FAILED"); return false;
}


//// Klein hulpfunctietje om met timeout te wachten
//static bool waitFor(std::function<bool()> pred, uint32_t timeoutMs, uint32_t stepMs = 100)
//{
//	const uint32_t start = hal_millis();
//	while (!pred())
//	{
//		if (hal_millis() - start >= timeoutMs) return false;
//		hal_delay_ms(stepMs);
//		yield();
//	}
//	return true;
//}
