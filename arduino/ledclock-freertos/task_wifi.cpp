#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "rtos.h"
#include "globals.h"
#include "task_wifi.h"
#include "wifi.h"
#include "task_portal.h"
#include "portal.h"
#include "hal_time_freertos.h"

//static void push_led(uint8_t r,uint8_t g,uint8_t b,uint16_t ms)
//{
//	LedCmd c
//	{
//		r,g,b,ms
//	};
//	xQueueSend(g_ledQueue,&c,0);
//}

void task_wifi(void*)
{

	Preferences prefs;
	
	// Load saved credentials
	prefs.begin(PREF_NS,true);
	String ssid = prefs.getString("ssid","");
	String pass = prefs.getString("pass","");
	prefs.end();

	if(ssid.isEmpty())
	{
		// Start portal immediately
		xEventGroupSetBits(g_sysEvents, EVT_PORTAL_ON);
		startPortal();
	}

	for(;;)
	{
		if(!rtos_test_bits(EVT_WIFI_UP) && !rtos_test_bits(EVT_PORTAL_ON))
		{
			//push_led(0,0,32,50);
			connectWiFi(ssid, pass);

			// Wait up to 10s
			uint32_t t0 = hal_millis();
			while(hal_millis()-t0 < 10000 && WiFi.status()!=WL_CONNECTED)
			{
				hal_delay_ms(50);
			}

			if(WiFi.status()==WL_CONNECTED)
			{
				xEventGroupSetBits(g_sysEvents, EVT_WIFI_UP);
			}
			else
			{
				// Fallback to portal
				xEventGroupSetBits(g_sysEvents, EVT_PORTAL_ON);
				startPortal();
			}
		}
		hal_delay_ms(250);
	}
}
