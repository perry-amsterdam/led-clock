#include <Arduino.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include "globals.h"
#include "rtos.h"
#include "task_http.h"
#include "portal.h"
#include "hal_time_freertos.h"
#include "http_api.h"
extern void handlePing();
static bool s_api_running = false;

void startApi()
{
	if (s_api_running) return;
	xEventGroupClearBits(g_sysEvents, EVT_PORTAL_ON);
	stopPortal();
	server.on("/api/ping", HTTP_GET, handlePing);
	server.begin();
	startHttpTask();
	MDNS.end();
	if (MDNS.begin("ledclock"))
	{
		MDNS.setInstanceName("LED Clock");
		MDNS.addService("http", "tcp", 80);
	}
	s_api_running = true;
}


void stopApi()
{
	if (!s_api_running) return;
	stopHttpTask();
	server.stop();
	MDNS.end();
	s_api_running = false;
}


void handlePing()
{
	server.sendHeader("Access-Control-Allow-Origin", "*");
	server.sendHeader("Cache-Control", "no-store");

	unsigned long uptime = hal_millis();
	size_t freeHeap = ESP.getFreeHeap();

	String mode = "UNKNOWN";
	wifi_mode_t wm = WiFi.getMode();
	if (wm & WIFI_AP)
	{
		mode = "AP";
	}
	else if (wm & WIFI_STA)
	{
		mode = "STA";
	}

	time_t tnow = time(nullptr);
	unsigned long long now_ms = (unsigned long long)tnow * 1000ULL;

	String json = "{";
	json += "\"pong\":true,";
	json += "\"now\":" + String((unsigned long long)now_ms) + ",";
	json += "\"uptime_ms\":" + String(uptime) + ",";
	json += "\"heap_free\":" + String((unsigned long)freeHeap) + ",";
	json += "\"wifi_mode\":\"" + mode + "\"";
	json += "}";

	server.send(200, "application/json", json);
}
