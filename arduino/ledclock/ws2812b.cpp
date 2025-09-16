#include "ws2812b.h"

// Kies n lib: Adafruit_NeoPixel (aanbevolen op ESP32-S3)
// Arduino Library Manager: "Adafruit NeoPixel"
#include <Adafruit_NeoPixel.h>

#ifndef LED_PIN
#define LED_PIN  8				 // Pas aan: je datapin
#endif
#ifndef LED_COUNT
#define LED_COUNT  84			 // Pas aan: aantal leds
#endif
#ifndef LED_BRIGHTNESS
#define LED_BRIGHTNESS  32
#endif

static Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Kleine helper om een pixel te zetten (met bounds check)
static inline void setPix(uint16_t i, uint32_t c)
{
	if(i < strip.numPixels()) strip.setPixelColor(i, c);
}


void ws2812bBegin()
{
	strip.begin();
	strip.setBrightness(LED_BRIGHTNESS);
	strip.clear();
	strip.show();
}


// Eenvoudige clock-demo:
// - seconde loopt als sweep (pixel index)
// - minuten = kleurcomponent G
// - uren = kleurcomponent R
// Pas aan naar jouw eigen layout/logica van de klok.
void ws2812bUpdate(const tm& now, time_t /*epoch*/) {
// Map tijd  kleuren/posities
uint8_t sec  = now.tm_sec;		 // 0..59
uint8_t min  = now.tm_min;		 // 0..59
uint8_t hour = now.tm_hour % 12; // 0..11

// Basis: dim achtergrond
strip.clear();

// Posities (als je een andere ring/segment indeling hebt, pas dit aan)
uint16_t posSec  = sec % LED_COUNT;
uint16_t posMin  = map(min, 0, 59, 0, LED_COUNT-1);
								 // uur met min-meeschuif
uint16_t posHour = map(hour * 5 + (min/12), 0, 59, 0, LED_COUNT-1);

// Kleuren (R,G,B)
uint8_t r = 200;				 // uren
uint8_t g = 180;				 // minuten
uint8_t b = 255;				 // seconden

// Minimaal anti-ghost: trek wat helderheid terug als posities samenvallen
auto col = [](uint8_t r,uint8_t g,uint8_t b){ return ((uint32_t)r<<16) | ((uint32_t)g<<8) | b; };

// Minuten-ring
setPix(posMin, col(0, g, 0));

// Uren-ring
setPix(posHour, col(r, 0, 0));

// Seconden-sweep
setPix(posSec, col(0, 0, b));

strip.show();
}


void ws2812bShow()
{
	strip.show();
}
