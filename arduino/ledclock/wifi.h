#pragma once
#include <Arduino.h>
#include "globals.h"

bool connectWiFi(const String& ssid, const String& pass, uint32_t timeoutMs = 15000);
