#pragma once
#include <Adafruit_NeoPixel.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <WiFi.h>

#include "config.h"

extern Adafruit_NeoPixel pixel;
extern Preferences prefs;
extern WebServer server;
extern DNSServer dns;

// state
extern String savedSsid, savedPass;
extern String g_timezoneIANA;
extern String g_countryCode;
extern int    g_gmtOffsetSec;
extern int    g_daylightSec;
extern bool   g_timeReady;
extern unsigned long lastPrintMs;
