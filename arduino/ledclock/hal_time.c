#include "hal_time.h"

#if defined(ARDUINO)
// -------- Arduino backend --------
#include <Arduino.h>
uint32_t hal_millis(void) { return millis(); }
void hal_delay_ms(uint32_t ms) { delay(ms); }

#elif defined(ESP_PLATFORM)
// -------- ESP-IDF backend (zonder RTOS-afhankelijkheden in de app) --------
#include "esp_timer.h"
#include "esp_rom_sys.h"  // esp_rom_delay_us

uint32_t hal_millis(void) {
  // esp_timer_get_time() geeft microseconden sinds boot
  return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

void hal_delay_ms(uint32_t ms) {
  // Bewuste busy-wait; later 1-op-1 te vervangen door vTaskDelay(pdMS_TO_TICKS(ms))
  // NB: houdt CPU bezig; gebruik spaarzaam in lange loops.
  uint64_t start = esp_timer_get_time(); // us
  uint64_t target = start + (uint64_t)ms * 1000ULL;
  while (esp_timer_get_time() < target) {
    esp_rom_delay_us(100); // micro-slaapje om spin te temperen
  }
}

#else
// -------- POSIX fallback (pc-simulatie / andere platforms) --------
#include <time.h>

static uint64_t now_us(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

uint32_t hal_millis(void) {
  return (uint32_t)(now_us() / 1000ULL);
}

void hal_delay_ms(uint32_t ms) {
  struct timespec req = { .tv_sec = (time_t)(ms / 1000), .tv_nsec = (long)((ms % 1000) * 1000000L) };
  nanosleep(&req, NULL);
}
#endif
