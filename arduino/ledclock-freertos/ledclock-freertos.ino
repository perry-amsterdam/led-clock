#include <Arduino.h>
#include "rtos.h"
#include "hal_time_freertos.h"
#include "globals.h"
#include "status_led.h"
#include "task_led.h"
#include "task_wifi.h"
#include "task_time.h"
#include "task_portal.h"
#include "task_render.h"

void setup()
{
	Serial.begin(115200);
	hal_delay_ms(50);
	Serial.println("\r[Boot] Start (FreeRTOS)");
	ledBegin();

	// Create RTOS primitives
	g_sysEvents = xEventGroupCreate();
	g_ledQueue  = xQueueCreate(LED_QUEUE_LEN, sizeof(LedCmd));

	// Create tasks
	xTaskCreatePinnedToCore(task_led,    "led",    STACK_LED,    nullptr, PRIO_LED,    nullptr, 1);
	xTaskCreatePinnedToCore(task_wifi,   "wifi",   STACK_WIFI,   nullptr, PRIO_WIFI,   nullptr, 1);
	xTaskCreatePinnedToCore(task_time,   "time",   STACK_TIME,   nullptr, PRIO_TIME,   nullptr, 1);
	xTaskCreatePinnedToCore(task_portal, "portal", STACK_PORTAL, nullptr, PRIO_PORTAL, nullptr, 0);
	xTaskCreatePinnedToCore(task_render, "render", STACK_RENDER, nullptr, PRIO_RENDER, nullptr, 0);
}


void loop()
{
	// Nothing to do here; tasks run under FreeRTOS.
	vTaskDelay(pdMS_TO_TICKS(1000));
}
