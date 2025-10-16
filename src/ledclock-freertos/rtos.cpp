#include "rtos.h"
EventGroupHandle_t g_sysEvents = nullptr;
//QueueHandle_t      g_ledQueue  = nullptr;

void rtos_wait_bits(EventBits_t bits, TickType_t timeoutTicks)
{
	xEventGroupWaitBits(g_sysEvents, bits, pdFALSE, pdTRUE, timeoutTicks);
}


bool rtos_test_bits(EventBits_t bits)
{
	EventBits_t v = xEventGroupGetBits(g_sysEvents);
	return (v & bits) == bits;
}
