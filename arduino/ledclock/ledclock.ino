 /*
  led-clock.ino
  Copyright (c) 2019, Perry Couprie
  https://github.com/perry-amsterdam/led-clock
  This software is released under the MIT License.
  https://opensource.org/licenses/MIT
*/

#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        6		

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 84			 

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Time (in milliseconds) to pause between pixels
#define DELAYVAL 998			 

int seconden = 30;
int minuten = 15;
int uren = 6;

unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
unsigned long endMillis;

void clock_background_leds()
{
	for (int i = 0; i < 84; i++)
	{
		pixels.setPixelColor(i, pixels.Color(0,0,0));
	}
	for (int i = 0; i < 24; i=i+3)
	{
		pixels.setPixelColor(i, pixels.Color(5,0,0));
	}
	for (int i = 0; i < 60; i=i+5)
	{
		pixels.setPixelColor(i+24, pixels.Color(5,0,0));
	}
}

void setup()
{
	//start Serial in case we need to print debugging info
	Serial.begin(115200);
	
	// get the current "time" (actually the number of milliseconds since the program started)
	startMillis = millis();

	pixels.begin();
}


void loop()
{

	// Get start time in mili seconds.
	startMillis = millis();

	// Do clock calculation, wil be replaced by ntp code.
	if (seconden < 59)
	{
		seconden++;
	}
	else
	{
		seconden = 0;
		if (minuten < 59)
		{
			minuten++;

		}
		else
		{
			minuten = 0;
			if (uren < 24)
			{
				uren++;
			}
			else
			{
				uren = 0;
			}
		}
	}

	// Set background leds.
	clock_background_leds();

	// set clock seconds, minutes, hours.
	pixels.setPixelColor(seconden + 24, pixels.Color(0, 20, 0));
	pixels.setPixelColor(minuten + 24, pixels.Color(0, 20, 0));
	pixels.setPixelColor(uren, pixels.Color(0, 20, 0));

	// Display all leds.
	pixels.show();

	// Get Duration of code.
	endMillis = millis();
	int loop_time = (endMillis - startMillis);

	// Display time duration of loop code.
	char output[100];
	sprintf(output, "loop time %d\n", loop_time);
	Serial.write(output); 

	delay(DELAYVAL);
}

