#include <Arduino.h>
#include "rtos.h"
#include "ws2812b.h"
#include <Adafruit_NeoPixel.h>
#include "net_time.h"
#include <time.h>
#include "hal_time_freertos.h"

void task_render(void*)
{
	vTaskDelay(pdMS_TO_TICKS(50));
	LOG_STACK_WATERMARK("render:init");

	ws2812bBegin();

	int direction = 0;

	EventBits_t bits = xEventGroupGetBits(g_sysEvents);
	while (!(bits & EVT_TIME_READY))
	{
		if (bits & EVT_PORTAL_ON)
		{
			// Captive portal actief  rood pulse
			drawStatusTicks(25, 0, 0);
		}
		else if (!(bits & EVT_WIFI_UP))
		{
			// WiFi is niet verbonden  blauw pulse
			drawStatusTicks(0, 0, 25);
		}
		else if (!(bits & EVT_TIME_READY))
		{
			// WiFi verbonden, maar tijd nog niet gesynchroniseerd  groen pulse
			drawStatusTicks(0, 25, 0);
		}

		hal_delay_ms(500);
		bits = xEventGroupGetBits(g_sysEvents);
	}

	for(;;)
	{
		LOG_STACK_WATERMARK("render:loop");
		struct tm now;
		time_t epoch = time(nullptr);
		if (getLocalTime(&now))
		{
			#if DEBUG_TIMING
			unsigned long start = micros();
			ws2812bUpdate(now, epoch);
			unsigned long duration = micros() - start;
			Serial.printf("ws2812bUpdate took %.3f ms\n", duration / 1000.0);
			#else
			ws2812bUpdate(now, epoch);
			#endif
		}

		// ~4 FPS animation.
		hal_delay_ms(500);
	}
}
