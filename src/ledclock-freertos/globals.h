#ifndef GLOBALS_H
#define GLOBALS_H

#pragma once
#include <Adafruit_NeoPixel.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <WiFi.h>
#include <atomic>

extern std::atomic<bool> gPowerSaveMode;

void enterPowerSaveMode();
void exitPowerSaveMode();
bool isPowerSaveMode();

// ==== Configuration, debug flags, pins ====

#define DEBUG_TZ 1
#define DEBUG_NET 1
#define DEBUG_TIMING 1
#define DEBUG_START_DELAY 0
#define STACK_DEBUG 0			 // zet op 0 om stack-logs uit te zetten

#define LED_PIN    48			 // < PAS AAN (vaak 8 of 48)
#define LED_COUNT  1

extern const uint8_t LED_BRIGHTNESS;
extern const unsigned long TIME_PRINT_INTERVAL_SEC;

// Wi-Fi / AP
extern const char* PREF_NS;
extern const char* PREF_SSID;
extern const char* PREF_PASS;
extern const char* AP_SSID;
extern const char* AP_PASS;
extern const byte DNS_PORT;
extern const IPAddress AP_IP, AP_GW, AP_MASK;

// Time & HTTP endpoints
extern const char* URL_TIMEINFO;
extern const char* URL_COUNTRY;
extern const char* NTP1;
extern const char* NTP2;
extern const char* NTP3;

#if defined(ARDUINO)
#define millis DO_NOT_USE_millis__use_hal_millis
#define delay  DO_NOT_USE_delay__use_hal_delay_ms
#endif

extern Adafruit_NeoPixel pixel;
extern Preferences prefs;
extern WebServer server;
extern DNSServer dns;

// state
extern String savedSsid, savedPass;

// --- Tijdzone configuratie ---
struct TimezoneInfo
{
	String iana;				 // bijv. "Europe/Amsterdam"
	String tzString;			 // bijv. "CET-1CEST,M3.5.0,M10.5.0/3"
	int gmtOffsetSec;			 // offset in seconden t.o.v. UTC
	int daylightSec;			 // zomertijd offset in seconden
};

extern TimezoneInfo g_timezoneInfo;
#endif							 // GLOBALS_H
