#pragma once
#include <Arduino.h>
#include "globals.h"

bool connectWiFi(const String& ssid, const String& pass, uint32_t timeoutMs = 15000);

// Verbreek Wi-Fi. turnOffRadio=true zet WiFi.mode(WIFI_OFF).
// eraseCredentials=true wist opgeslagen AP-gegevens (ESP32/Arduino: WiFi.disconnect(..., /*eraseap=*/true)).
void disconnectWiFi(bool turnOffRadio = false, bool eraseCredentials = false);

