// ledclock 
// Board: ESP32-S3 (e.g., S3-N16R8)

#include <Arduino.h>
#include "config.h"
#include "globals.h"
#include "status_led.h"
#include "wifi.h"
#include "net_time.h"
#include "portal.h"
#include "hal_time.h"
#include "ws2812b.h"
#include <time.h>
#include <ESPmDNS.h>

// Optional compile-time config
// Uncomment and adjust before including ws2812b.cpp in your build system if needed.
#define LED_PIN 8
#define LED_COUNT 84
#define LED_BRIGHTNESS 48
#define RING60_OFFSET 0
#define RING60_DIR +1
#define RING24_OFFSET 0
#define RING24_DIR +1

void setup()
{
	Serial.begin(115200);
	hal_delay_ms(5000);
	Serial.println("\r[Boot] Start");

	ledBegin(); ledBlue();

	prefs.begin(PREF_NS,true);
	savedSsid = prefs.getString("ssid","");
	savedPass = prefs.getString("pass","");
	prefs.end();
	if(DEBUG_NET) Serial.printf("[Boot] stored SSID='%s' len(pass)=%u\n", savedSsid.c_str(), savedPass.length());

	if(connectWiFi(savedSsid, savedPass))
	{

		// Setup mdns for ledclock.local
		if (MDNS.begin("ledclock"))
		{
			Serial.println("\rmDNS responder started: http://ledclock.local/");
			// optional: announce your webserver
			MDNS.addService("http", "tcp", 80);
		}
		else
		{
			Serial.println("\rError setting up MDNS responder!");
		}

		// >>> HIER: TZ opvragen en NTP instellen bij opstart <<<
		if(setupTimeFromInternet(/*acceptAllHttps=*/true))
		{
			lastPrintMs = hal_millis();
			g_timeReady = true;
			ws2812bBegin();
		}
		else
		{
			Serial.println("\r[Boot] NTP/TZ setup mislukt");
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
		unsigned long nowMs = hal_millis();
		if(nowMs - lastPrintMs >= TIME_PRINT_INTERVAL_SEC*1000UL)
		{

			lastPrintMs = nowMs;

			struct tm now;

			if(getLocalTime(&now))
			{
				char buf[64]; strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",&now);
				time_t epoch = time(nullptr);
				Serial.printf("\r[Time] %s | epoch=%ld\n", buf, (long)epoch);

				// Optionally pass epoch if you want smooth animations tied to wall-clock seconds.
				ws2812bUpdate(now, epoch);
			}
			else
			{
				Serial.println("\r[Time] getLocalTime() fail \342\200\224 probeer resync?");
			}
		}
	}
}
