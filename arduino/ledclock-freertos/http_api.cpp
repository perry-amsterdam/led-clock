#include <Arduino.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include "globals.h"
#include "rtos.h"
#include "task_http.h"
#include "portal.h"
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
    if (MDNS.begin("ledclock")) {
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
