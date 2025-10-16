#include "theme_storage_bridge.h"

bool themeStorageLoad(char* out, size_t max) {
    (void)out; (void)max;
    return false; // geen opslag beschikbaar in deze default build
}

bool themeStorageSave(const char* id) {
    (void)id;
    return false;
}

bool themeStorageClear() {
    return false;
}
