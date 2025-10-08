#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <time.h>
extern "C"
{
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
}


#include "globals.h"
#include "http_api.h"
#include "hal_time_freertos.h"

// ======================================================
// Globals
// ======================================================
extern DNSServer dns;	// captive portal DNS server
static TaskHandle_t httpTaskHandle = nullptr;
static bool s_api_running = false;

// ======================================================
// Helper functies
// ======================================================

static String wifiModeStr()
{
	wifi_mode_t m = WiFi.getMode();
	switch (m)
	{
		case WIFI_MODE_NULL: return "OFF";
		case WIFI_MODE_STA:  return "STA";
		case WIFI_MODE_AP:   return "AP";
		case WIFI_MODE_APSTA:return "APSTA";
		default: return "UNKNOWN";
	}
}


static void sendJson(int code, const String& json)
{
	server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
	server.sendHeader("Pragma", "no-cache");
	server.send(code, "application/json", json);
}


// ======================================================
// /api/timezones (GET) - compacte lijst (uitbreidbaar)
// ======================================================
static const char* kTimezones[] PROGMEM =
{
	"UTC",

	// Europe
	"Europe/Amsterdam", "Europe/London", "Europe/Berlin", "Europe/Paris",
	"Europe/Madrid", "Europe/Rome", "Europe/Warsaw", "Europe/Athens",
	"Europe/Istanbul", "Europe/Moscow", "Europe/Zurich", "Europe/Stockholm",

	// America
	"America/New_York", "America/Chicago", "America/Denver", "America/Los_Angeles",
	"America/Phoenix", "America/Toronto", "America/Mexico_City", "America/Sao_Paulo",

	// Africa
	"Africa/Cairo", "Africa/Johannesburg", "Africa/Nairobi",

	// Asia
	"Asia/Dubai", "Asia/Karachi", "Asia/Kolkata", "Asia/Bangkok",
	"Asia/Singapore", "Asia/Hong_Kong", "Asia/Taipei",
	"Asia/Shanghai", "Asia/Tokyo", "Asia/Seoul",

	// Oceania / Pacific
	"Australia/Sydney", "Australia/Melbourne", "Australia/Perth",
	"Pacific/Auckland", "Pacific/Honolulu",

	// Atlantic
	"Atlantic/Reykjavik"
};
static constexpr size_t kTimezoneCount = sizeof(kTimezones) / sizeof(kTimezones[0]);

static void apiHandleTimezonesGet()
{
	server.sendHeader("Access-Control-Allow-Origin", "*");
	server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
	server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

	String body;
	body.reserve(2048);
	body += "{\"timezones\":[";
	for (size_t i = 0; i < kTimezoneCount; ++i)
	{
		if (i) body += ',';
		body += '"';

		// PROGMEM safe
		const char* tz = (const char*)pgm_read_ptr(&kTimezones[i]);
		body += tz;
		body += '"';
	}
	body += "]}";

	server.send(200, "application/json; charset=utf-8", body);
}


// ======================================================
// /api/ping
// ======================================================
static void apiHandlePing()
{
	uint64_t epoch_ms = (uint64_t)(time(nullptr)) * 1000ULL;
	String json = "{";
	json += "\"pong\":true";
	json += ",\"now\":" + String((long long)epoch_ms);
	json += ",\"uptime_ms\":" + String((uint32_t)hal_millis());
	json += ",\"heap_free\":" + String((int)ESP.getFreeHeap());
	json += ",\"wifi_mode\":\"" + wifiModeStr() + "\"";
	json += "}";
	sendJson(200, json);
}


// ======================================================
// /api/system/reboot (POST)
// ======================================================
static void apiHandleReboot()
{
	sendJson(200, "{\"rebooting\":true,\"message\":\"Rebooting system...\"}");
	xTaskCreate(
		[](void*)
	{
		vTaskDelay(pdMS_TO_TICKS(250));
			ESP.restart();
			vTaskDelete(nullptr);
	},
		"reboot_task", 2048, nullptr, tskIDLE_PRIORITY + 3, nullptr
		);
}


// ======================================================
// Tijdzone helpers
// ======================================================
static String nvsReadTimezone()
{
	Preferences p;
	if (!p.begin("sys", true)) return "";
	String tz = p.getString("tz", "");
	p.end();
	return tz;
}


static void nvsWriteTimezone(const String& tz)
{
	Preferences p;
	if (p.begin("sys", false))
	{
		p.putString("tz", tz);
		p.end();
	}
}


// ======================================================
// /api/timezone (GET)
// ======================================================
static void apiHandleTimezoneGet()
{
	String tz = g_timezoneIANA;
	if (tz.length() == 0) tz = nvsReadTimezone();

	time_t now = time(nullptr);
	struct tm lt;
	localtime_r(&now, &lt);

	long off = 0;
	#if defined(__USE_MISC) || defined(__GLIBC__) || defined(__APPLE__)
	off = lt.tm_gmtoff;
	#else
	struct tm gmt = *gmtime(&now);
	off = (long)difftime(mktime(&lt), mktime(&gmt));
	#endif

	String json = "{";
	json += "\"timezone\":\"" + (tz.length() ? tz : String("")) + "\"";
	json += ",\"utc_offset_sec\":" + String((long)off);
	json += "}";
	sendJson(200, json);
}


// ======================================================
// /api/timezone (POST)
// ======================================================
static void apiHandleTimezonePost()
{
	if (server.method() != HTTP_POST)
	{
		sendJson(405, "{\"success\":false,\"message\":\"Method Not Allowed\"}");
		return;
	}
	if (!server.hasArg("plain"))
	{
		sendJson(400, "{\"success\":false,\"message\":\"Missing body\"}");
		return;
	}
	String body = server.arg("plain");
	int k = body.indexOf("\"timezone\"");
	if (k < 0)
	{
		sendJson(400, "{\"success\":false,\"message\":\"Missing 'timezone'\"}");
		return;
	}
	k = body.indexOf(':', k);
	int q1 = body.indexOf('"', k);
	int q2 = (q1 >= 0) ? body.indexOf('"', q1 + 1) : -1;
	if (q1 < 0 || q2 <= q1)
	{
		sendJson(400, "{\"success\":false,\"message\":\"Invalid 'timezone'\"}");
		return;
	}
	String tz = body.substring(q1 + 1, q2);
	tz.trim();

	if (tz.length() < 3 || tz.length() > 64)
	{
		sendJson(400, "{\"success\":false,\"message\":\"Invalid timezone format\"}");
		return;
	}

	g_timezoneIANA = tz;
	nvsWriteTimezone(tz);

	sendJson(200, "{\"success\":true,\"message\":\"Timezone updated successfully\"}");
}


// ======================================================
// HTTP task (FreeRTOS)
// ======================================================
static void httpTask(void* arg)
{
	for (;;)
	{
		server.handleClient();
		dns.processNextRequest();
		#if defined(ESPmDNS_H)
		MDNS.update();
		#endif
		vTaskDelay(pdMS_TO_TICKS(2));
	}
}


// ======================================================
// startApi / stopApi / startHttpTask / stopHttpTask
// ======================================================
void startApi()
{
	if (s_api_running) return;

	Serial.println("[HTTP] Starting API...");
	server.on("/api/ping", HTTP_GET, apiHandlePing);
	server.on("/api/system/reboot", HTTP_POST, apiHandleReboot);
	server.on("/api/timezone", HTTP_GET, apiHandleTimezoneGet);
	server.on("/api/timezone", HTTP_POST, apiHandleTimezonePost);
	server.on("/api/timezones", HTTP_GET, apiHandleTimezonesGet);

	server.begin();
	startHttpTask();
	s_api_running = true;

	// Start mDNS
	if (!MDNS.begin("ledclock"))
	{
		Serial.println("[mDNS] Fout bij starten van mDNS");
	}
	else
	{
		MDNS.addService("http", "tcp", 80);
		Serial.println("[mDNS] Service gestart: http://ledclock.local");
	}

	Serial.println("[HTTP] API gestart");
}


void stopApi()
{
	if (!s_api_running) return;
	Serial.println("[HTTP] Stopt API...");
	server.close();
	MDNS.end();
	s_api_running = false;
	Serial.println("[HTTP] API gestopt");
}


void startHttpTask()
{
	if (httpTaskHandle == nullptr)
	{
		xTaskCreatePinnedToCore(httpTask, "http", 4096, nullptr, 1, &httpTaskHandle, 1);
		Serial.println("[HTTP] Task gestart");
	}
}


void stopHttpTask()
{
	if (httpTaskHandle != nullptr)
	{
		TaskHandle_t t = httpTaskHandle;
		httpTaskHandle = nullptr;
		vTaskDelete(t);
		Serial.println("[HTTP] Task gestopt");
	}
}
