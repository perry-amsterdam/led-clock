#include <Arduino.h>
#include "rtos.h"
#include "net_time.h"
#include "globals.h"
#include "hal_time_freertos.h"
#include <Preferences.h>

//extern String g_timezoneIANA;	 //  als deze al in globals staat, is dit genoeg
void task_time(void*)
{
	// Wait for WiFi
	vTaskDelay(pdMS_TO_TICKS(100));
	LOG_STACK_WATERMARK("time:init");
	rtos_wait_bits(EVT_WIFI_UP);
	LOG_STACK_WATERMARK("time:wifi");

	// NTP & TZ setup
	bool ok = false;
	for (int i = 0; (!ok && i < 10) ; i++) {
		ok = setupTimeFromInternet(true);
		if (ok)
		{
			xEventGroupSetBits(g_sysEvents, EVT_TIME_READY);
			xEventGroupClearBits(g_sysEvents, EVT_TIME_UPDATE_RETRY);
			LOG_STACK_WATERMARK("time:ntp");
		}
		hal_delay_ms(1000);
	}

	static uint32_t last = 0;

	// Periodieke tijds-onderhoud/sync
	for(;;)
	{

		// run roughly once per minute
		uint32_t now_ms = hal_millis();
		if (now_ms - last > 60000)
		{
			last = now_ms;

			// If epoch looks invalid (< 8 hours since boot default), schedule time update.
			time_t now = time(nullptr);
			if (now < 8 * 3600)
			{
				xEventGroupSetBits(g_sysEvents, EVT_TIME_UPDATE_RETRY);
			}
		}

		// A time update was scheduled.
		EventBits_t bits = xEventGroupGetBits(g_sysEvents);
		if (bits & EVT_TIME_UPDATE_RETRY)
		{
			bool ok = setupTimeFromInternet(true);
			if (ok)
			{
				xEventGroupClearBits(g_sysEvents, EVT_TIME_UPDATE_RETRY);
			}
		}

		hal_delay_ms(1000);
	}
}
