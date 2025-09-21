#include <Arduino.h>
#include "rtos/rtos.h"
#include "ws2812b.h"
#include "net_time.h"
#include <time.h>

void task_render(void*)
{

	// Wait until time is ready (prevents jumping display)
	rtos_wait_bits(EVT_TIME_READY);
	ws2812bBegin();

	for(;;)
	{
		struct tm now;
		time_t epoch = time(nullptr);
		if(getLocalTime(&now))
		{
			ws2812bUpdate(now, epoch);
		}
		// ~30 FPS animation (adjust inside ws2812bUpdate if static)
		vTaskDelay(pdMS_TO_TICKS(33));
	}
}
