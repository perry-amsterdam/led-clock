#pragma once

// ==== Configuration, debug flags, pins ====

#define DEBUG_TZ 1
#define DEBUG_NET 1
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
