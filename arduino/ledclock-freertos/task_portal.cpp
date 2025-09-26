#include <Arduino.h>
#include "rtos.h"
#include "task_portal.h"
#include "globals.h"

void task_portal(void*)
{
	vTaskDelay(pdMS_TO_TICKS(50));
	LOG_STACK_WATERMARK("portal:init");
	for(;;)
	{
		LOG_STACK_WATERMARK("portal:loop");
		if(rtos_test_bits(EVT_PORTAL_ON))
		{
			dns.processNextRequest();
			server.handleClient();
		}
		else
		{
			vTaskDelay(pdMS_TO_TICKS(200));
		}
	}
}
