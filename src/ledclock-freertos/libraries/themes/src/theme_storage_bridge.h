#pragma once
#include <cstddef>

// Storage bridge voor theme-id persistence.
// Standaard-implementaties (in .cpp) doen niets en geven false terug.
// Je kunt deze functies in je applicatie overschrijven door eigen
// implementaties te linken die dezelfde symbolen exporteren.

bool themeStorageLoad(char* out, size_t max);   // vult out met id, return true bij succes
bool themeStorageSave(const char* id);          // slaat id op, return true bij succes
bool themeStorageClear();                       // wist opslag, return true bij succes
