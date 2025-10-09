#pragma once
#include <Arduino.h>

//String nvsReadTimezone();
//void nvsWriteTimezone(const String& tz);

// Geeft aan of de gebruiker via de API een timezone heeft gezet.
// Default = false (0) als nooit gezet.
bool tz_user_is_set();

// Leest de handmatig ingestelde IANA timezone string.
// Returnt true als een niet-lege TZ is gevonden.
bool tz_user_get(String& out);

// Slaat de handmatige IANA timezone op en zet het bit.
void tz_user_set(const String& tz);

// Leegt de handmatige TZ en reset het bit naar 0.
void tz_user_clear();
