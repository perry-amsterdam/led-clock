#include <Arduino.h>
#include "rtos.h"
#include "status_led.h"
#include "task_led.h"
#include "hal_time_freertos.h"

static inline void ledBluePulse()
{
	ledBlue();  hal_delay_ms(120);
	ledOff();   hal_delay_ms(880);
}


static inline void ledGreenPulse()
{
	ledGreen(); hal_delay_ms(120);
	ledOff();   hal_delay_ms(380);
}


static inline void ledRedPulse()
{
	ledRed(); hal_delay_ms(120);
	ledOff();   hal_delay_ms(380);
}


void task_led(void*)
{
	ledBegin();
	for(;;)
	{
		// Non-blocking: check for queued color commands first
		LedCmd cmd;
		//if (xQueueReceive(g_ledQueue, &cmd, pdMS_TO_TICKS(10)) == pdPASS)
		//{
		//	ledColor(cmd.r,cmd.g,cmd.b); hal_delay_ms(cmd.hold_ms);
		//	continue;
		//}

		//		if(rtos_test_bits(EVT_PORTAL_ON))
		//		{
		//			ledRedPulse();
		//		} else {
		//
		//			if(!rtos_test_bits(EVT_WIFI_UP))
		//			{
		//				ledBluePulse();
		//			} else {
		//				if(rtos_test_bits(EVT_TIME_READY))
		//				{
		//					ledGreenPulse();
		//				}
		//			}
		//		}
	}
}
