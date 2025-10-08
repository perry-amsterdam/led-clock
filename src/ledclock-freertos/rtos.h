#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>

// ----- Event bits -----
constexpr EventBits_t EVT_WIFI_UP      = 1 << 0;
constexpr EventBits_t EVT_TIME_READY   = 1 << 1;
constexpr EventBits_t EVT_PORTAL_ON    = 1 << 2;

extern EventGroupHandle_t g_sysEvents;

// ----- LED command queue -----
struct LedCmd { uint8_t r,g,b; uint16_t hold_ms; };
constexpr size_t LED_QUEUE_LEN = 8;
extern QueueHandle_t g_ledQueue;

// Utility helpers
void rtos_wait_bits(EventBits_t bits, TickType_t timeoutTicks = portMAX_DELAY);
bool rtos_test_bits (EventBits_t bits);

// Task stack sizes & priorities (tuned for ESP32S3)
constexpr uint32_t STACK_LED     = 2048;
constexpr uint32_t STACK_WIFI    = 4096;
constexpr uint32_t STACK_TIME    = 6144;
constexpr uint32_t STACK_PORTAL  = 4096;
constexpr uint32_t STACK_RENDER  = 4096;

constexpr UBaseType_t PRIO_LED    = 2;
constexpr UBaseType_t PRIO_WIFI   = 3;
constexpr UBaseType_t PRIO_TIME   = 3;
constexpr UBaseType_t PRIO_PORTAL = 2;
constexpr UBaseType_t PRIO_RENDER = 2;

#if STACK_DEBUG
#define LOG_STACK_WATERMARK(TAG) do { \
		UBaseType_t __hw = uxTaskGetStackHighWaterMark(NULL); \
		/* uxTaskGetStackHighWaterMark geeft woorden terug (4 bytes/word)*/ \
		size_t __bytes = (size_t)__hw * 4; \
		Serial.printf("[%-8s] stack min free ~= %u bytes\n", \
		(TAG), (unsigned)__bytes); \
	} while(0)
#else
#define LOG_STACK_WATERMARK(TAG) do {} while(0)
#endif
