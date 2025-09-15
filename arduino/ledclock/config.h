// config.sample.h
#pragma once

// ---- Wi-Fi instellingen ----
#define WIFI_SSID   "your_wifi_ssid"
#define WIFI_PASS   "your_wifi_password"

// ---- mDNS / hostname ----
#define HOSTNAME    "ledclock"

// ---- Tijdzone & NTP servers ----
// CET/CEST voorbeeld (NL/BE): laatste zondag van maart/okt tot zomer-/wintertijd
#define TZ_INFO     "CET-1CEST,M3.5.0/2,M10.5.0/3"
// ---- NTP servers (declared in globals.cpp) ----
#ifdef __cplusplus
extern const char* const NTP1;
extern const char* const NTP2;
extern const char* const NTP3;
#endif

