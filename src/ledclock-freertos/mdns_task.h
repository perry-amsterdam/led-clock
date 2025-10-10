
#pragma once

#include <Arduino.h>

/**
 * Dedicated mDNS task for ESP32 (ESPmDNS).
 *
 * Why?
 *  - Centralizes all MDNS.begin()/addService()/end() calls in one place.
 *  - Prevents use of the (non-existent on ESP32) MDNS.update().
 *  - Lets other modules (wifi/http/portal) send commands without worrying about timing.
 *
 * Usage (typical):
 *   mdnsTaskInit();                           // once at boot
 *   mdnsStart("ledclock");                    // after WiFi STA connected or AP started
 *   mdnsAddHttpService(80);                   // when HTTP server is up
 *   mdnsAnnounce();                           // optional: force announce after IP change
 *   mdnsStop();                               // before shutting down WiFi or switching modes
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

	// Trigger a manual announce (optional). Returns true if the command was enqueued.
	bool mdnsAnnounce();

	#ifdef __cplusplus
}								 // extern "C"
#endif
