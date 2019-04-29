
#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        6		

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 48			 

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Time (in milliseconds) to pause between pixels
#define DELAYVAL 50				 

int seconden = 30;
int minuten = 15;
int uren = 6;

int led_count = 6;

void reset_leds()
{

	for (int i = 0; i < 30; i++)
	{
		pixels.setPixelColor(i, pixels.Color(0, 20, 0));
	}

	for (int i = 30; i < 42; i++)
	{
		pixels.setPixelColor(i, pixels.Color(0, 0, 20));
	}

	for (int i = 42; i < 48; i++)
	{
		pixels.setPixelColor(i, pixels.Color(50, 50, 0));
	}
}

void setup()
{

	pixels.begin();
	for (int i = 0; i < 30; i++)
	{
		pixels.setPixelColor(i, pixels.Color(0, 0, 0));
	}
	pixels.show();
}


void loop()
{

	if (seconden < 29)
	{
		seconden++;
	}
	else
	{
		seconden = 0;
		if (minuten < 29)
		{
			minuten++;

		}
		else
		{
			minuten = 0;
			if (uren < 11)
			{
				uren++;
			}
			else
			{
				uren = 0;
			}
		}
	}

	// Set background color.
	reset_leds();

	// set clock leds.
	pixels.setPixelColor(seconden, pixels.Color(150, 0, 150));
	pixels.setPixelColor((minuten), pixels.Color(200, 0, 0));
	pixels.setPixelColor(uren + 30, pixels.Color(200, 200, 200));

	// Set fun lights.
	for (int i = 42; i < (42 + led_count); i++)
	{
		pixels.setPixelColor(i, pixels.Color(0, 50, 50));
	}

	led_count++;
	if (led_count > 5)
	{
		led_count = 0;
	}

	pixels.show();

	delay(DELAYVAL);
}
