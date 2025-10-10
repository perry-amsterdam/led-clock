
#include "mdns_task.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// ---- Internal command queue ----
enum class MdnsCmd : uint8_t
{
	Start,
	Stop,
	AddHttp,
	Announce
};

struct MdnsMsg
{
	MdnsCmd cmd;
	uint16_t port;				 // for AddHttp
	char hostname[33];			 // for Start (ESP mDNS limit is typically 32 chars)
};

static TaskHandle_t sTask = nullptr;
static QueueHandle_t sQueue = nullptr;

static void mdnsTask(void* arg)
{
	(void)arg;
	MdnsMsg msg{};

	for (;;)
	{
		if (xQueueReceive(sQueue, &msg, portMAX_DELAY) == pdTRUE)
		{
			switch (msg.cmd)
			{
				case MdnsCmd::Start:
				{
					// End any previous instance first (safe to call even if not started)
					MDNS.end();

					// Only attempt start if we have an interface up; for STA ensure WiFi is connected
					// (AP mode also works; library handles interface selection)
					bool hasIf = (WiFi.isConnected() || WiFi.getMode() & WIFI_AP);
					if (!hasIf)
					{
						// No interface yet; small delay to avoid tight loop
						vTaskDelay(pdMS_TO_TICKS(100));
					}
					if (!MDNS.begin(msg.hostname))
					{
						Serial.println("[mDNS] Failed to start mDNS");
					}
					else
					{
						Serial.printf("[mDNS] Started as %s.local\n", msg.hostname);
					}
				} break;

				case MdnsCmd::Stop:
				{
					MDNS.end();
					Serial.println("[mDNS] Stopped");
				} break;

				case MdnsCmd::AddHttp:
				{
					// Advertise HTTP service
					// NOTE: addService is safe to call multiple times; subsequent calls update the service
					if (!MDNS.addService("http", "tcp", msg.port))
					{
						Serial.printf("[mDNS] Failed to add HTTP service on port %u\n", msg.port);
					}
					else
					{
						Serial.printf("[mDNS] HTTP service: _http._tcp on port %u\n", msg.port);
					}
				} break;

				case MdnsCmd::Announce:
				{
					// Force an extra announcement (ESP32 API)
					MDNS.announce();
					Serial.println("[mDNS] Announce sent");
				} break;
			}
		}
	}
}


// ---- Public API ----
extern "C" void mdnsTaskInit(uint32_t stackSize, UBaseType_t priority)
{
	if (!sQueue)
	{
		sQueue = xQueueCreate(8, sizeof(MdnsMsg));
	}
	if (!sTask && sQueue)
	{
		xTaskCreatePinnedToCore(mdnsTask, "mdns_task", stackSize, nullptr, priority, &sTask, APP_CPU_NUM);
	}
}


static bool enqueue(const MdnsMsg& in)
{
	if (!sQueue) return false;
	return xQueueSend(sQueue, &in, pdMS_TO_TICKS(50)) == pdTRUE;
}


extern "C" bool mdnsStart(const char* hostname)
{
	if (!hostname) return false;
	MdnsMsg m{};
	m.cmd = MdnsCmd::Start;
	strncpy(m.hostname, hostname, sizeof(m.hostname) - 1);
	m.hostname[sizeof(m.hostname) - 1] = '\0';
	return enqueue(m);
}


extern "C" bool mdnsStop()
{
	MdnsMsg m{};
	m.cmd = MdnsCmd::Stop;
	return enqueue(m);
}


extern "C" bool mdnsAddHttpService(uint16_t port)
{
	MdnsMsg m{};
	m.cmd = MdnsCmd::AddHttp;
	m.port = port;
	return enqueue(m);
}


extern "C" bool mdnsAnnounce()
{
	MdnsMsg m{};
	m.cmd = MdnsCmd::Announce;
	return enqueue(m);
}
