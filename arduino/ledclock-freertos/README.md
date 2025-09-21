# LEDClock — FreeRTOS Refactor (ESP32‑S3 / Arduino‑ESP32)

This package contains a task-based FreeRTOS scaffold for your LED clock on ESP32‑S3. It replaces a monolithic `loop()` with five tasks coordinated via an EventGroup and a small LED command queue.

## What’s inside
- `src/main_freertos.cpp`: starts tasks in `setup()`; `loop()` idle.
- `src/rtos/*`: EventGroup + LED queue, priorities, helpers.
- `src/tasks/*`: tasks for LED, Wi‑Fi, time/NTP, captive portal, and rendering.
- `src/compat/hal_time_freertos.h`: RTOS-friendly `hal_millis()`/`hal_delay_ms()`.

## Expected existing modules (from your original project)
Make sure these headers/functions exist in your project (or adapt the includes to your actual names):

- `globals.h`
  - Provides `PREF_NS` (Preferences namespace), `dns` (DNSServer), `server` (WebServer), etc.
  - Your project‑wide globals and configuration.

- `status_led.h`
  - `void ledBegin();`
  - `void ledOff();`
  - `void ledBlue();`
  - `void ledGreen();`
  - `void ledColor(uint8_t r,uint8_t g,uint8_t b);`

- `wifi.h`
  - `void connectWiFi(const String& ssid, const String& pass);`

- `portal.h`
  - `void startPortal();`
  - `void stopPortal();`
  - `extern DNSServer dns;`
  - `extern WebServer server;`

- `net_time.h`
  - `bool setupTimeFromInternet(bool firstBoot);`
  - `bool getLocalTime(struct tm* info, uint32_t ms = 5000);`
  - `void netTimeMaintain();`

- `ws2812b.h`
  - `void ws2812bBegin();`
  - `void ws2812bUpdate(const struct tm& now, time_t epoch);`

If your actual function names differ, adjust the includes or rename in the corresponding task files.

## Usage
1. Copy the `src/` subtree into your project (merge folders).
2. Remove code from your `.ino` (or keep it empty) since tasks are created in `setup()` of `main_freertos.cpp`.
3. Build for **ESP32‑S3** with the Arduino‑ESP32 core.

## Notes
- Task core affinity: Wi‑Fi/Time/LED on core 1; Portal/Render on core 0 (tweak as needed).
- Animation rate: ~30 FPS by default (`task_render` uses 33 ms tick). Adjust in `ws2812bUpdate` or the task delay.

Enjoy!
