// Led clock example.
#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    6

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 84

unsigned long startMillis;
unsigned long currentMillis;
unsigned long endMillis;

// Clock informatie.
int seconden = 30;
int minuten = 15;
int uren = 6;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// setup() function -- runs once at startup --------------------------------
void setup() {

  //start Serial in case we need to print debugging info
  Serial.begin(115200);

  strip.begin();
  strip.show();
  strip.setBrightness(50);
}

// loop() function -- runs repeatedly as long as board is on ---------------
void loop() {

  startMillis = millis();

  rainbow(10);

  // Get Duration of code.
  endMillis = millis();
  int loop_time = (endMillis - startMillis);

  char output[100];
  sprintf(output, "loop time %d\n", loop_time);
  Serial.write(output);
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {

  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
    for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...

      // Rainbow leds.
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue, 10, 59)));

      // Clock leds.
      strip.setPixelColor(seconden + 24, strip.Color(0, 0, 255),10,255);
      strip.setPixelColor(minuten + 24, strip.Color(0, 255, 0),10,255);
      strip.setPixelColor(uren, strip.Color(255, 0, 0),10,255);
    }

    // Calculate seconden, minuten en uren.
    if (seconden > 59)
    {
      seconden = 0;
      minuten++;
      if (minuten > 59)
      {
        minuten = 0;
        uren++;
        if (uren > 23)
        {
          uren = 0;
        }
      }
    }
    else
    {
      seconden++;
    }

    // Send led info to leds.
    strip.show(); // Update strip with new contents
    delay(wait * 20); // Pause for a moment
  }
}
