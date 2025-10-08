// http_api.h
#pragma once

#include <Arduino.h>
#include <WebServer.h>

// Publieke WebServer instantie (wordt gedefinieerd in http_api.cpp)
extern WebServer server;

/**
 * Registreert alle API-routes en start de HTTP server + mDNS.
 * - Routes: /api/ping (GET), /api/system/reboot (POST), /api/timezone (GET/POST)
 * - mDNS: ledclock.local (http service)
 */
void startApi();

/**
 * Stopt de HTTP server en mDNS.
 */
void stopApi();

/**
 * Start de FreeRTOS task die server.handleClient() runt.
 * (non-blocking webserver loop)
 */
void startHttpTask();

/**
 * Stopt de FreeRTOS task van de webserver.
 */
void stopHttpTask();
