// task_led.cpp
#include "task_led.h"
#include "status_led.h"
#include "rtos.h"
#include "hal_time_freertos.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

extern EventGroupHandle_t g_sysEvents;

void task_led(void* arg)
{
	(void)arg;

	for (;;)
	{
		LOG_STACK_WATERMARK("led:loop");
		EventBits_t bits = xEventGroupGetBits(g_sysEvents);

		if (bits & EVT_PORTAL_ON)
		{
			// Captive portal actief  rood pulse
			ledRedPulse();
		}
		else if (!(bits & EVT_WIFI_UP))
		{
			// WiFi is niet verbonden  blauw pulse
			ledBluePulse();
		}
		else if (!(bits & EVT_TIME_READY))
		{
			// WiFi verbonden, maar tijd nog niet gesynchroniseerd  groen pulse
			ledGreenPulse();
		}
		else
		{
			// Alles OK (groene)
			ledGreen();
		}
		hal_delay_ms(500);
	}
}
