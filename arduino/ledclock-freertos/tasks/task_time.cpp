#include <Arduino.h>
#include "rtos/rtos.h"
#include "net_time.h"
#include "globals.h"

void task_time(void*)
{

	// Wait for WiFi
	rtos_wait_bits(EVT_WIFI_UP);

	// NTP & TZ setup
	if(setupTimeFromInternet(true))
	{
		xEventGroupSetBits(g_sysEvents, EVT_TIME_READY);
	}

	// Periodic time checks / resyncs
	for(;;)
	{
		netTimeMaintain();
		hal_delay_ms(1000);
	}
}
