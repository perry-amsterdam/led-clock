// ledclock (split version)
// Board: ESP32-S3 (e.g., S3-N16R8)

#include <Arduino.h>
#include "config.h"
#include "globals.h"
#include "status_led.h"
#include "wifi.h"
#include "net_time.h"
#include "portal.h"

void setup()
{
	Serial.begin(115200);
	delay(5000);
	Serial.println("\n[Boot] Start");

	ledBegin(); ledBlue();

	prefs.begin(PREF_NS,true);
	savedSsid = prefs.getString("ssid","");
	savedPass = prefs.getString("pass","");
	prefs.end();
	if(DEBUG_NET) Serial.printf("[Boot] stored SSID='%s' len(pass)=%u\n", savedSsid.c_str(), savedPass.length());

	if(connectWiFi(savedSsid, savedPass))
	{
		// >>> HIER: TZ opvragen en NTP instellen bij opstart <<<
		if(setupTimeFromInternet(/*acceptAllHttps=*/true))
		{
			lastPrintMs = millis();
      g_timeReady = true;
		}
		else
		{
			Serial.println("[Boot] NTP/TZ setup mislukt");
		}
	}
	else
	{
		startPortal();
	}
}


void loop()
{
	// Portal afhandelen als AP actief is
	if(WiFi.getMode()==WIFI_AP || WiFi.getMode()==WIFI_AP_STA)
	{
		dns.processNextRequest(); server.handleClient();
	}

	// Periodiek tijd printen (alleen als NTP ok)
	if(WiFi.status()==WL_CONNECTED && g_timeReady)
	{
		unsigned long nowMs = millis();
		if(nowMs - lastPrintMs >= TIME_PRINT_INTERVAL_SEC*1000UL)
		{

			lastPrintMs = nowMs;

			struct tm t; if(getLocalTime(&t))
			{
				char buf[64]; strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",&t);
				time_t epoch = time(nullptr);
				Serial.printf("\r[Time] %s | epoch=%ld | TZ=%s | CC=%s\n", buf, (long)epoch, g_timezoneIANA.c_str(), g_countryCode.length()? g_countryCode.c_str():"(?)");

        // Hier komt de code voor de ws2812b leds.

			}
			else
			{
				Serial.println("[Time] getLocalTime() fail \342\200\224 probeer resync?");
			}
		}
	}
}
