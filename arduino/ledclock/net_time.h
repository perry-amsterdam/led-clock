#pragma once
#include "globals.h"

String extractJsonString(const String& src, const char* key);
void dumpPreview(const String& json);
bool fetchTimeInfo();
bool fetchCountryCode();
bool setupTimeFromInternet();
