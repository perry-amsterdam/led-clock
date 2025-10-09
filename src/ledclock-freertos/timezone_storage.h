#pragma once
#include <Arduino.h>

String nvsReadTimezone();
void nvsWriteTimezone(const String& tz);
