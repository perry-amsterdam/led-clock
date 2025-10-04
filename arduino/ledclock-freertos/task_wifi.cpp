#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>

#include "rtos.h"
#include "globals.h"
#include "task_wifi.h"
#include "task_http.h"
#include "wifi.h"
#include "task_portal.h"
#include "portal.h"
#include "hal_time_freertos.h"

// Externs from other units
extern WebServer server;
extern void handlePing();

// Local state
static bool s_services_started = false;
static bool s_portal_on        = false;

static void start_services_once()
{
    if (s_services_started) return;

    // Register routes BEFORE begin()
    server.on("/api/ping", HTTP_GET, handlePing);

    server.begin();
    Serial.println("[HTTP] API server listening on port 80");
    startHttpTask();

    // Clean mDNS lifecycle to handle reconnects gracefully
    MDNS.end();
    if (!MDNS.begin("ledclock"))
    {
        Serial.println("[mDNS] ERROR: MDNS.begin(\"ledclock\") failed");
    }
    else
    {
        MDNS.setInstanceName("LED Clock");
        MDNS.addService("http", "tcp", 80);
        Serial.println("[mDNS] OK: ledclock.local with _http._tcp on 80");
    }

    s_services_started = true;
}

static void stop_services_if_running()
{
    if (!s_services_started) return;
    Serial.println("[Net] Stopping API and mDNS");
    MDNS.end();
    server.stop();
    s_services_started = false;
}

static void ensure_portal_on()
{
    if (s_portal_on) return;
    Serial.println("[Portal] Starting captive portal");
    xEventGroupSetBits(g_sysEvents, EVT_PORTAL_ON);
    startPortal();
    s_portal_on = true;
}

static void ensure_portal_off()
{
    if (!s_portal_on) return;
    Serial.println("[Portal] Stopping captive portal");
    stopPortal();
    s_portal_on = false;
}

// ===== Wi-Fi supervisory task =====
void task_wifi(void*)
{
    // Optional: attempt connection using saved credentials if available
    // (Saved in Preferences by other parts of the code)
    // If WPS or other flows are used elsewhere, they can still bring Wi-Fi up;
    // we only react to status here and manage the dependent services.

    for (;;)
    {
        wl_status_t st = WiFi.status();
        if (st == WL_CONNECTED)
        {
            // Notify other tasks and bring up services
            xEventGroupSetBits(g_sysEvents, EVT_WIFI_UP);
            Serial.printf("[Net] WiFi up \xE2\x9C\x93  SSID='%s'  IP=%s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
            ensure_portal_off();   // Portal not needed while STA is up
            start_services_once(); // REST API + mDNS

            LOG_STACK_WATERMARK("wifi:ip");
        }
        else
        {
            // Wi-Fi down -> ensure services are stopped and portal is on
            stop_services_if_running();
            ensure_portal_on();
        }

        hal_delay_ms(250);
    }
}
