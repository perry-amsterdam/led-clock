#include <Arduino.h>
#include "rtos.h"
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
		}

		// ~30 FPS animation (adjust inside ws2812bUpdate if static)
		vTaskDelay(pdMS_TO_TICKS(33));
	}
}
