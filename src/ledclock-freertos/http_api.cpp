#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <time.h>
extern "C"
{
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
}


#include "globals.h"
#include "theme_registry.h"
#include <theme_manager.h>
#include "http_api.h"
#include "hal_time_freertos.h"
#include "config_storage.h"
#include "task_mdns.h"

// ======================================================
// Globals
// ======================================================
extern DNSServer dns;			 // captive portal DNS server
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


static void sendJson(WebServer& server, int code, const String& json)
{
	server.send(code, "application/json", json);
}


// ======================================================
// Compacte wereldlijst met veelgebruikte tijdzones
// ======================================================
static const char* kTimezones[] PROGMEM =
{
	// ---- Universeel ----
	"UTC",

	// ---- Europa ----
	"Europe/Amsterdam", "Europe/London", "Europe/Paris", "Europe/Berlin",
	"Europe/Brussels", "Europe/Madrid", "Europe/Rome", "Europe/Warsaw",
	"Europe/Athens", "Europe/Istanbul", "Europe/Moscow",
	"Europe/Zurich", "Europe/Stockholm",

	// ---- Noord-Amerika ----
	"America/New_York", "America/Chicago", "America/Denver", "America/Los_Angeles",
	"America/Toronto", "America/Mexico_City",

	// ---- Midden- & Zuid-Amerika ----
	"America/Lima", "America/Bogota", "America/Santiago", "America/Buenos_Aires",
	"America/Sao_Paulo", "America/Montevideo", "America/Paramaribo", "America/Costa_Rica",
	"America/Curacao",

	// ---- Afrika ----
	"Africa/Cairo", "Africa/Johannesburg", "Africa/Nairobi",

	// ---- Azi ----
	"Asia/Dubai", "Asia/Karachi", "Asia/Kolkata", "Asia/Bangkok",
	"Asia/Singapore", "Asia/Hong_Kong", "Asia/Tokyo", "Asia/Seoul",

	// ---- Oceani ----
	"Australia/Sydney", "Pacific/Auckland", "Pacific/Honolulu"
};
static constexpr size_t kTimezoneCount = sizeof(kTimezones) / sizeof(kTimezones[0]);

// Put near kTimezones[] and kTimezoneCount
static bool isValidIanaTimezone(const String& tz)
{
	for (size_t i = 0; i < kTimezoneCount; ++i)
	{
		// Read from PROGMEM safely
		const char* item = (const char*)pgm_read_ptr(&kTimezones[i]);

		// exact match, case-sensitive (IANA)
		if (tz.equals(item)) return true;
	}
	return false;
}


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
// /api/timezone (GET)
// ======================================================
static void apiHandleTimezoneGet()
{
	// Prefer runtime globals filled at startup; fallback to NVS
	String tz;
	if (g_timezoneIANA.length())
	{
		tz = g_timezoneIANA;
	}
	else
	{
		tz_user_get(tz);
	}

	// Compute current total UTC offset from localtime vs gmtime if time is valid
	time_t now = time(nullptr);
	long off = 0;
	if (now > 8 * 3600)			 // sanity check
	{
		struct tm lt = *localtime(&now);
		struct tm gmt = *gmtime(&now);
		off = (long)difftime(mktime(&lt), mktime(&gmt));
	}
	else
	{
		// if time not valid yet, use stored offsets if we have them
		off = (long)g_gmtOffsetSec + (long)g_daylightSec;
	}

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

	// Basic shape check (optional, keeps error messages clearer)
	if (tz.length() < 3 || tz.length() > 64 || tz.indexOf(' ') >= 0 || tz.indexOf('\\') >= 0)
	{
		sendJson(400, "{\"success\":false,\"message\":\"Invalid timezone format\"}");
		return;
	}

	//  IANA validation against our compiled list
	if (!isValidIanaTimezone(tz))
	{
		sendJson(400, "{\"success\":false,\"message\":\"Unknown timezone. Use /api/timezones to list supported IANA names.\"}");
		return;
	}

	// Save & apply
	tz_user_set(tz.c_str());
	setenv("TZ", tz.c_str(), 1);
	tzset();

	sendJson(200, "{\"success\":true,\"message\":\"Timezone updated successfully\"}");
}


// ======================================================
// /api/timezone (DELETE)
// ======================================================
static void apiHandleTimezoneDelete()
{
	if (server.method() != HTTP_DELETE)
	{
		sendJson(405, "{\"success\":false,\"message\":\"Method Not Allowed\"}");
		return;
	}

	// Clear user timezone in NVS en runtime
	tz_user_clear();
	g_timezoneIANA = "";

	// Bouw response zoals GET
	String tz;
	if (g_timezoneIANA.length())
	{
		tz = g_timezoneIANA;
	}
	else
	{
		tz_user_get(tz);
	}

	time_t now = time(nullptr);
	long off = 0;
	if (now > 8 * 3600)			 // sanity check
	{
		struct tm lt = *localtime(&now);
		struct tm gmt = *gmtime(&now);
		off = (long)difftime(mktime(&lt), mktime(&gmt));
	}
	else
	{
		off = (long)g_gmtOffsetSec + (long)g_daylightSec;
	}

	String json = "{";
	json += "\"success\":true";
	json += ",\"message\":\"Timezone cleared; using default\"";
	json += ",\"timezone\":\"" + (tz.length() ? tz : String("")) + "\"";
	json += ",\"utc_offset_sec\":" + String((long)off);
	json += "}";
	sendJson(200, json);
}


//
// ========== Themes: LIST ==========
//
static void apiHandleThemesList()
{
	size_t count = 0;
	const Theme* const* themes = themeList(&count);

	const Theme* def = themeDefault();
	const Theme* cur = themeCurrent();

	String json = "[";
	for (size_t i = 0; i < count; ++i)
	{
		const Theme* t = themes[i];
		if (!t) continue;

		if (i > 0) json += ",";	 // komma tussen items

		json += "{";
		json += "\"id\":\"" + String(t->id) + "\"";
		json += ",\"name\":\"" + String(t->name) + "\"";
		json += ",\"is_default\":" + String(t == def ? "true" : "false");
		json += ",\"is_active\":"  + String(t == cur ? "true" : "false");
		json += "}";
	}
	json += "]";

	server.send(200, "application/json", json);
}


//
// ========== Theme: GET current ==========
//
//static void apiHandleThemeGet()
//{
//	auto& reg = ThemeRegistry::get();
//	const Theme* def = reg.getDefault();
//	const Theme* cur = reg.getActive();
//
//	// Probeer ook te laten zien of er een user override is opgeslagen
//	String saved = loadThemeId();
//	bool has_override = saved.length() > 0;
//
//	String json = "{";
//	json += "\"active_id\":\"" + String(cur ? cur->id : "") + "\"";
//	json += ",\"active_name\":\"" + String(cur ? cur->name : "") + "\"";
//	json += ",\"is_default\":" + String(cur == def ? "true" : "false");
//	json += ",\"has_saved_override\":" + String(has_override ? "true" : "false");
//	if (has_override)
//	{
//		json += ",\"saved_override_id\":\"" + saved + "\"";
//	}
//	json += "}";
//
//	server.send(200, "application/json", json);
//}

////
//// ========== Theme: SET (POST) ==========
//// Ondersteunt: /api/theme?id=<theme_id>  (query) of als form-field "id"
////
//static void apiHandleThemeSet()
//{
//	String id = server.arg("id");// query of form-field
//								 // fallback alias
//	if (id.isEmpty()) id = server.arg("name");
//
//	if (id.isEmpty())
//	{
//		server.send(400, "application/json", "{\"error\":\"missing id parameter\"}");
//		return;
//	}
//
//	auto& reg = ThemeRegistry::get();
//	const Theme* t = reg.findById(id.c_str());
//	if (!t)
//	{
//		server.send(404, "application/json", "{\"error\":\"unknown theme id\"}");
//		return;
//	}
//
//	reg.setActive(t);			 // maak dit theme actief
//	saveThemeId(id);			 // bewaar als user override
//
//	String json = "{";
//	json += "\"ok\":true";
//	json += ",\"active_id\":\"" + String(t->id) + "\"";
//	json += ",\"active_name\":\"" + String(t->name) + "\"";
//	json += "}";
//	server.send(200, "application/json", json);
//}
//
//
////
//// ========== Theme: CLEAR (DELETE) ==========
//// Verwijdert de user override; default wordt weer gebruikt.
////
//static void apiHandleThemeClear()
//{
//	clearSavedTheme();
//	auto& reg = ThemeRegistry::get();
//	const Theme* def = reg.getDefault();
//	if (def) reg.setActive(def); // forceer default als actief (optioneel)
//
//	String json = "{";
//	json += "\"ok\":true";
//	json += ",\"active_id\":\"" + String(def ? def->id : "") + "\"";
//	json += ",\"active_name\":\"" + String(def ? def->name : "") + "\"";
//	json += ",\"is_default\":true";
//	json += "}";
//	server.send(200, "application/json", json);
//}

// ======================================================
// startApi / stopApi / startHttpTask / stopHttpTask
// ======================================================
void startApi()
{
	if (s_api_running) return;

	Serial.println("[HTTP] Starting API...");

	// Api system calls.
	server.on("/api/ping", HTTP_GET, apiHandlePing);
	server.on("/api/system/reboot", HTTP_POST, apiHandleReboot);

	// Api timezone calls.
	server.on("/api/timezone", HTTP_GET, apiHandleTimezoneGet);
	server.on("/api/timezone", HTTP_POST, apiHandleTimezonePost);
	server.on("/api/timezone", HTTP_DELETE, apiHandleTimezoneDelete);
	server.on("/api/timezones", HTTP_GET, apiHandleTimezonesGet);

	//	// Api themes calls.
	server.on("/api/themes", HTTP_GET,  apiHandleThemesList);
	//	server.on("/api/theme",  HTTP_GET,  apiHandleThemeGet);
	//	server.on("/api/theme",  HTTP_POST, apiHandleThemeSet);
	//	server.on("/api/theme",  HTTP_DELETE, apiHandleThemeClear);

	server.begin();
	startHttpTask();
	s_api_running = true;

	// Start mDNS
	mdnsStop();
	if (!mdnsStart("ledclock"))
	{
		Serial.println("[mDNS] Fout bij starten van mDNS");
	}
	else
	{
		mdnsAddHttpService(80);
		Serial.println("[mDNS] Service gestart: http://ledclock.local");
	}

	Serial.println("[HTTP] API gestart");
}


void stopApi()
{
	if (!s_api_running) return;
	Serial.println("[HTTP] Stopt API...");
	stopHttpTask();
	server.close();
	mdnsStop();
	s_api_running = false;
	Serial.println("[HTTP] API gestopt");
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
		vTaskDelay(pdMS_TO_TICKS(2));
	}
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
