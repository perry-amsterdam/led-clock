#include <Arduino.h>
#include "rtos.h"
#include "hal_time_freertos.h"
#include "globals.h"
#include "status_led.h"
#include "ws2812b.h"
#include "task_led.h"
#include "task_wifi.h"
#include "task_time.h"
#include "task_portal.h"
#include "task_render.h"

#if !defined(ESP_ARDUINO_VERSION_MAJOR) || (ESP_ARDUINO_VERSION_MAJOR < 3)
#error "This project requires Arduino-ESP32 core v3.0.0 or higher"
#endif

void setup()
{

	// Clear the minuts/seconds and hour leds.
	ws2812bBegin();

	// Clear the status led.
	ledOff();

	// Wacht 5 seconden.
	if (DEBUG_START_DELAY)
	{
		hal_delay_ms(5000);
	}

	Serial.begin(115200);
	hal_delay_ms(50);
	Serial.println("\r[Boot] Start (FreeRTOS)");
	ledBegin();
	ledSelfTest();				 // korte test van de status-LED

	// Create RTOS primitives
	g_sysEvents = xEventGroupCreate();
	g_ledQueue  = xQueueCreate(LED_QUEUE_LEN, sizeof(LedCmd));

	// Create tasks
	xTaskCreatePinnedToCore(task_led,    "led",    STACK_LED,    nullptr, PRIO_LED,    nullptr, 1);
	xTaskCreatePinnedToCore(task_wifi,   "wifi",   STACK_WIFI,   nullptr, PRIO_WIFI,   nullptr, 1);
	xTaskCreatePinnedToCore(task_time,   "time",   STACK_TIME,   nullptr, PRIO_TIME,   nullptr, 1);
	xTaskCreatePinnedToCore(task_portal, "portal", STACK_PORTAL, nullptr, PRIO_PORTAL, nullptr, 0);
	xTaskCreatePinnedToCore(task_render, "render", STACK_RENDER, nullptr, PRIO_RENDER, nullptr, 0);

	// --- Stack debug boot summary ---
	hal_delay_ms(500);			 // even wachten zodat alle tasks gestart zijn
	if (STACK_DEBUG)
	{
		Serial.println("[stack] first samples:");
	}
	LOG_STACK_WATERMARK("time");
	LOG_STACK_WATERMARK("wifi");
	LOG_STACK_WATERMARK("render");
	LOG_STACK_WATERMARK("led");
	LOG_STACK_WATERMARK("portal");
}


void loop()
{
	// Nothing to do here; tasks run under FreeRTOS.
	vTaskDelay(pdMS_TO_TICKS(1000));
}
