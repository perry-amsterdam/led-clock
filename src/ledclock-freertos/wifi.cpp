#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
extern WebServer server;
extern void handlePing();
#include "wifi.h"
#include "http_api.h"
#include "http_portal.h"
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

void disconnectWiFi(bool turnOffRadio, bool eraseCredentials)
{
    if (DEBUG_NET) { Serial.println("[WiFi] Disconnecting..."); }

    // Als je een WebServer gebruikt, stop hem netjes
    #ifdef WebServer_h
    // 'server' is extern hierboven gedeclareerd in dit bestand
    server.stop();
    #endif

    // ESP32 Arduino: WiFi.disconnect(wifioff=false, eraseap=false)
    // We doen eerst een normale disconnect om de link te verbreken.
    WiFi.disconnect(turnOffRadio /*wifioff*/, eraseCredentials /*eraseap*/);

    // Eventueel radio uitzetten om stroom te besparen
    if (turnOffRadio) {
        WiFi.mode(WIFI_OFF);
    } else {
        // Laat de stack in een nette staat; Station-mode zonder verbinding
        WiFi.mode(WIFI_STA);
    }

    // Kleine statusmelding
    if (WiFi.getMode() == WIFI_OFF) {
        if (DEBUG_NET) { Serial.println("[WiFi] Disconnected. Radio OFF"); }
    } else {
        if (DEBUG_NET) { Serial.println("[WiFi] Disconnected. Radio ON (STA idle)"); }
    }

    // (Optioneel) status LED update — alleen als je daar helpers voor hebt
    // ledRed();   // of ledOff(); comment laten staan als je geen rode LED hebt
}

