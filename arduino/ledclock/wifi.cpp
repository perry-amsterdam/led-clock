#include <Arduino.h>
#include <WiFi.h>
#include "wifi.h"
#include "portal.h"
#include "status_led.h"
#include "net_time.h"

bool connectWiFi(const String& ssid, const String& pass, uint32_t timeoutMs)
{
	if(ssid.isEmpty()){ Serial.println("[WiFi] geen opgeslagen SSID"); return false; }
	if(DEBUG_NET) Serial.printf("[WiFi] verbinden met SSID='%s'...\n", ssid.c_str());
	ledBlue();
	WiFi.mode(WIFI_STA);
	WiFi.disconnect(true,true);
	delay(50);
	WiFi.begin(ssid.c_str(), pass.c_str());

	uint32_t start=millis(); bool blink=false;
	while(WiFi.status()!=WL_CONNECTED && (millis()-start)<timeoutMs)
	{
		blink=!blink; if(blink) ledBlue(); else ledOff();
		delay(300); Serial.print('.');
	}
	Serial.println();

	if(WiFi.status()==WL_CONNECTED)
	{
		if(DEBUG_NET){ Serial.print("[WiFi] OK IP="); Serial.println(WiFi.localIP()); }
		ledGreen(); return true;
	}
	Serial.println("[WiFi] FAILED"); ledRed(); return false;
}
