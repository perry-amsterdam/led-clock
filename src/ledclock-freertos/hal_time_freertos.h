#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static inline uint32_t hal_millis(){ return (uint32_t)(xTaskGetTickCount()*portTICK_PERIOD_MS); }
static inline void hal_delay_ms(uint32_t ms){ vTaskDelay(pdMS_TO_TICKS(ms)); }
