// status_led.cpp
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "globals.h"			 // bevat extern Adafruit_NeoPixel pixel;
#include "status_led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

Adafruit_NeoPixel pixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// -----------------------------------------------------------------------------
// Basis-implementatie status-LED
// -----------------------------------------------------------------------------

void ledBegin()
{
	// Initialiseer de enkele status-pixel
	pixel.begin();
	#ifdef LED_BRIGHTNESS
	pixel.setBrightness(LED_BRIGHTNESS);
	#endif
	pixel.clear();
	pixel.show();
}


// Stelt de status-LED (index 0) in op (r,g,b) en toont direct
void ledColor(uint8_t r, uint8_t g, uint8_t b)
{
	pixel.setPixelColor(0, pixel.Color(r, g, b));
	pixel.show();
}


// Convenience helpers
void ledOff()   { ledColor(0,   0,   0); }
void ledRed()   { ledColor(50, 0,   0); }
void ledGreen() { ledColor(0,   50, 0); }
void ledBlue()  { ledColor(0,   0,  50); }

// -----------------------------------------------------------------------------
// Pulse-helpers (blokkerend binnen de status-LED taak)
// -----------------------------------------------------------------------------

// Generieke pulse (optioneel te gebruiken elders)
void ledPulse(uint8_t r, uint8_t g, uint8_t b, uint16_t on_ms, uint16_t off_ms)
{
	ledColor(r, g, b);
	vTaskDelay(pdMS_TO_TICKS(on_ms));
	ledOff();
	vTaskDelay(pdMS_TO_TICKS(off_ms));
}


// Blauw pulse: 120 ms aan, 880 ms uit
void ledBluePulse()
{
	ledBlue();
	vTaskDelay(pdMS_TO_TICKS(120));
	ledOff();
	vTaskDelay(pdMS_TO_TICKS(880));
}


// Groen pulse: 120 ms aan, 380 ms uit
void ledGreenPulse()
{
	ledGreen();
	vTaskDelay(pdMS_TO_TICKS(120));
	ledOff();
	vTaskDelay(pdMS_TO_TICKS(380));
}


// Rood pulse: 120 ms aan, 380 ms uit
void ledRedPulse()
{
	ledRed();
	vTaskDelay(pdMS_TO_TICKS(120));
	ledOff();
	vTaskDelay(pdMS_TO_TICKS(380));
}


// Zelftest: laat de status-LED rood, groen, blauw en uit zien
void ledSelfTest()
{
	ledRed();
	vTaskDelay(pdMS_TO_TICKS(500));
	ledGreen();
	vTaskDelay(pdMS_TO_TICKS(500));
	ledBlue();
	vTaskDelay(pdMS_TO_TICKS(500));
	ledOff();
}
