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
	for (int i = 0; (!ok && i < 10) ; i++)
	{
		ok = setupTimeFromInternet(true);
		if (ok)
		{
			xEventGroupSetBits(g_sysEvents, EVT_TIME_READY);
			xEventGroupClearBits(g_sysEvents, EVT_TIME_UPDATE_RETRY);
			LOG_STACK_WATERMARK("time:ntp");
		}
		hal_delay_ms(1000);
	}

	uint32_t last = 0;
    uint32_t last_ms = hal_millis();
    int last_trigger_yday = -1;  // -1 = nog nooit getriggerd

	// Periodieke tijds-onderhoud/sync
	for(;;)
	{

		// run roughly once per minute
        uint32_t now_ms = hal_millis();
        if ((uint32_t)(now_ms - last_ms) >= 60000u) {
            last_ms = now_ms;
			last = now_ms;

			// If epoch looks invalid (< 8 hours since boot default), schedule time update.
			time_t now = time(nullptr);
			if (now < 8 * 3600)
			{
				xEventGroupSetBits(g_sysEvents, EVT_TIME_UPDATE_RETRY);
			}

			// 1b) 03:00 lokale tijd trigger (eenmaal per dag)
            struct tm lt;
            if (localtime_r(&now, &lt)) {

                // tolerant window 03:00..03:05
                bool in_window = (lt.tm_hour == 3 && lt.tm_min <= 5);
                if (in_window) {
                    if (lt.tm_yday != last_trigger_yday) { 
                        xEventGroupSetBits(g_sysEvents, EVT_TIME_UPDATE_RETRY);
                        last_trigger_yday = lt.tm_yday;
                        Serial.printf("Nachtelijke resync gepland op dag %d\n", lt.tm_yday);
                    }
                } 
            } else {

                // lokale tijd niet beschikbaar is, forceer time update retry.
                xEventGroupSetBits(g_sysEvents, EVT_TIME_UPDATE_RETRY);
            }
		}

		// A time update was scheduled.
		//EventBits_t bits = xEventGroupGetBits(g_sysEvents);
		if (xEventGroupGetBits(g_sysEvents) & EVT_TIME_UPDATE_RETRY)
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
