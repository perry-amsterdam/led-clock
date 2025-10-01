#pragma once
#include <Arduino.h>
#include <time.h>				 // nodig voor struct tm (tijdstructuur uit C standaardbibliotheek)

// ---------------------------------------------------
// Configuratieopties
// ---------------------------------------------------
// Deze library gaat uit van twee LED-ringen:
// - Een ring met 60 leds  seconden + minuten
// - Een ring met 24 leds  uren
//
// Je kunt onderstaande defines overschrijven vr je deze header include,
// zodat de hardware aansluit op jouw project.
//
// Voorbeeld:
//   #define LED_PIN_60   8   // datapin voor ring met 60 leds
//   #define LED_PIN_24   9   // datapin voor ring met 24 leds
//
// Helderheid instellen (0..255)
#define GLOBAL_BRIGHTNESS  32

// Orintatie per ring (om de wijzers goed uit te lijnen)
// OFFSET = waar 0 komt te staan, DIR = draairichting (+1 of -1)
#define RING60_OFFSET  0
#define RING60_DIR     +1
#define RING24_OFFSET  0
#define RING24_DIR     +1

// Trail-instellingen (hoe lang de "staart" van de wijzer is)
#define TRAIL_LENGTH_SEC  2
#define TRAIL_LENGTH_MIN  1

// Kleuren (RGB per wijzer) - kan naar wens aangepast worden
/*
#define COLOR_SEC_R   0
#define COLOR_SEC_G   0
#define COLOR_SEC_B   150

#define COLOR_MIN_R   0
#define COLOR_MIN_G   80
#define COLOR_MIN_B   0

#define COLOR_HOUR_R  140
#define COLOR_HOUR_G  0
#define COLOR_HOUR_B  0
*/

// ---------------------------------------------------
// Publieke API
// ---------------------------------------------------

// Initialiseer de LED-strips (aanroepen in setup())
void ws2812bBegin();

// Update klokweergave op basis van een tijdstructuur
// - now: tijdstructuur (bv. lokale tijd of UTC)
// - epoch: seconden sinds UNIX epoch (optioneel, kan gebruikt worden voor animaties)
void ws2812bUpdate(const tm& now, time_t epoch);

// Handmatig forceren van refresh (bij eigen aanpassingen)
void ws2812bShow();
