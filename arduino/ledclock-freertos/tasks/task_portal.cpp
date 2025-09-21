#include <Arduino.h>
#include "rtos/rtos.h"
#include "portal.h"
#include "globals.h"

void task_portal(void*)
{
	for(;;)
	{
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
