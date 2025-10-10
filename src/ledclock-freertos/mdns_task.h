
#pragma once

#include <Arduino.h>

/**
 * Dedicated mDNS task for ESP32 (ESPmDNS).
 *
 * Public API:
 *   mdnsTaskInit();                           // once at boot
 *   mdnsStart("ledclock");                    // start (after WiFi STA connected or AP up)
 *   mdnsAddHttpService(80);                   // advertise HTTP
 *   mdnsStop();                               // stop
 *   mdnsRestart(nullptr or "ledclock");       // stop + start (uses last hostname if nullptr)
 *
 * Notes:
 *   - No MDNS.update() (ESP8266-only) and no MDNS.announce() (not in ESP32 3.3.x).
 *   - A fresh MDNS.begin(...) implicitly sends the necessary announcements.
 */

#ifdef __cplusplus
extern "C"
{
	#endif

	// Initialize task (idempotent). You can override stack/prio if needed.
	void mdnsTaskInit(uint32_t stackSize = 4096, UBaseType_t priority = (tskIDLE_PRIORITY + 1));

	// Start/Restart mDNS with hostname (e.g., "ledclock"). Safe to call multiple times.
	// Returns true if the command was enqueued.
	bool mdnsStart(const char* hostname);

	// Stop mDNS cleanly (MDNS.end()). Returns true if the command was enqueued.
	bool mdnsStop();

	// Add HTTP service on given port (e.g., 80). Returns true if the command was enqueued.
	bool mdnsAddHttpService(uint16_t port);

	// Restart mDNS (stop + start). If hostname is nullptr, the last used hostname is reused.
	// Returns true if the command was enqueued.
	bool mdnsRestart(const char* hostname);

	#ifdef __cplusplus
}								 // extern "C"
#endif
