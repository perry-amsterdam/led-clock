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

	//	// 1) TZ uit NVS proberen te lezen
	//	bool haveSavedTz = false;
	//	{
	//		Preferences p;
	//		if (p.begin("sys", true))// read-only
	//		{
	//			String tz = p.getString("tz", "");
	//			p.end();
	//			if (tz.length() > 0)
	//			{
	//				// bewaar in global (wordt al door API gebruikt)
	//				g_timezoneIANA = tz;
	//
	//				// runtime TZ instellen
	//				setenv("TZ", tz.c_str(), 1);
	//				tzset();
	//				haveSavedTz = true;
	//			}
	//		}
	//	}

	// NTP & TZ setup
	bool ok = setupTimeFromInternet(true);
	if (ok)
	{
		xEventGroupSetBits(g_sysEvents, EVT_TIME_READY);
		xEventGroupClearBits(g_sysEvents, EVT_TIME_UPDATE_RETRY);
		LOG_STACK_WATERMARK("time:ntp");
	}

	// Periodieke tijds-onderhoud/sync
	for(;;)
	{
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
