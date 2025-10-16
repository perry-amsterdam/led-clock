# ConfigStorageESP32

A micro library that stores two bits of app state in ESP32 NVS (Preferences):
- User **timezone** (IANA tz string, e.g. `Europe/Amsterdam`)
- **Theme id** (arbitrary string)

## API

```cpp
// Timezone
bool tz_user_is_set();
bool tz_user_get(String& out);    // out set when true
bool tz_user_set(const String& tz);
bool tz_user_clear();

// Theme id
bool theme_is_set();
bool saveThemeId(const String& id);
bool loadThemeId(String& out);    // out set when true
bool clearSavedTheme();
```

## Dependencies
- Arduino core for ESP32
- `Preferences.h` (bundled with Arduino-ESP32)

## Example
See `examples/TimezoneAndTheme/TimezoneAndTheme.ino`.