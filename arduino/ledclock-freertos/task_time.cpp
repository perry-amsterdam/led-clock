#include <Arduino.h>
#include "rtos.h"
#include "net_time.h"
#include "globals.h"
#include "hal_time_freertos.h"

void task_time(void*)
{

	// Wait for WiFi
	vTaskDelay(pdMS_TO_TICKS(100));
	LOG_STACK_WATERMARK("time:init");
	rtos_wait_bits(EVT_WIFI_UP);
	LOG_STACK_WATERMARK("time:wifi");

	// NTP & TZ setup
	if(setupTimeFromInternet(true))
	{
		xEventGroupSetBits(g_sysEvents, EVT_TIME_READY);
		LOG_STACK_WATERMARK("time:ntp");
	}

	// Periodic time checks / resyncs
	for(;;)
	{
		netTimeMaintain();
		hal_delay_ms(1000);
	}
}
