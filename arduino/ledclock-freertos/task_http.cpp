
#include <Arduino.h>
#include <WebServer.h>
#include "rtos.h"

extern WebServer server;

static TaskHandle_t httpTaskHandle = nullptr;

static void httpTask(void*)
{
	for (;;)
	{
		server.handleClient();

		// ~100 Hz
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}


void startHttpTask()
{
	if (httpTaskHandle == nullptr)
	{
		xTaskCreatePinnedToCore(httpTask, "http", 4096, nullptr, 1, &httpTaskHandle, 1);
	}
}
