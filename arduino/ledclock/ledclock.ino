
#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        6		

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 48			 

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Time (in milliseconds) to pause between pixels
#define DELAYVAL 998			 

int seconden = 30;
int minuten = 15;
int uren = 6;

int led_count = 6;

unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
unsigned long endMillis;

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
}

void setup()
{
	//start Serial in case we need to print debugging info
	Serial.begin(115200);
	
	// get the current "time" (actually the number of milliseconds since the program started)
	startMillis = millis();

	pixels.begin();
	for (int i = 0; i < 30; i++)
	{
		pixels.setPixelColor(i, pixels.Color(0, 0, 0));
	}
	pixels.show();
}


void loop()
{

	startMillis = millis();

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

	pixels.show();

	// Get Duration of code.
	endMillis = millis();
	int loop_time = (endMillis - startMillis);

	delay(DELAYVAL);

	char output[100];
	sprintf(output, "loop time %d\n", loop_time);

	Serial.write(output); 
}
