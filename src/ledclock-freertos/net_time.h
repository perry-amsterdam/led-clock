#pragma once
#include <Arduino.h>
#include "globals.h"

String extractJsonString(const String& json, const String& key);
void dumpPreview(const String& payload);
bool fetchTimeInfo(String& tzIana, int& gmtOffsetSec, int& daylightOffsetSec, bool acceptAllHttps = false);
bool setupTimeFromInternet(bool acceptAllHttps = false);
void netTimeMaintain();
