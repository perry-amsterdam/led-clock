#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <Adafruit_NeoPixel.h>
#include <time.h>
#include "globals.h"

// ===== Constant definitions =====
const uint8_t LED_BRIGHTNESS = 50;
const unsigned long TIME_PRINT_INTERVAL_SEC = 15;
const char* PREF_NS  = "wifi";
const char* PREF_SSID = "ssid";
const char* PREF_PASS = "pass";
const char* AP_SSID  = "ESP32-Setup";
const char* AP_PASS  = "configwifi";
const byte  DNS_PORT = 53;
const IPAddress AP_IP(192,168,4,1), AP_GW(192,168,4,1), AP_MASK(255,255,255,0);

const char* URL_TIMEINFO = "https://worldtimeapi.org/api/ip";
const char* URL_COUNTRY  = "https://ip-api.com/json";

const char* NTP1 = "europe.pool.ntp.org";
const char* NTP2 = "time.google.com";
const char* NTP3 = "pool.ntp.org";

// ===== Global objects =====
Preferences prefs;
WebServer server(80);
DNSServer dns;

// ===== Global state =====
String savedSsid, savedPass;

TimezoneInfo g_timezoneInfo = {
    "",   // iana
    "",   // tzString
    0,    // gmtOffsetSec
    0     // daylightSec
};

