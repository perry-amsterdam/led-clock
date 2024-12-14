#include <Adafruit_NeoPixel.h>

// Pin and LED count configuration
#define LED_PIN    6
#define LED_COUNT  84

unsigned long startMillis;
unsigned long currentMillis;
unsigned long endMillis;

// Clock information
int seconden = 30;
int minuten = 15;
int uren = 6;

// Declare the NeoPixel strip object
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Setup function: runs once at startup
void setup() {
  Serial.begin(115200);  // Initialize Serial for debugging
  strip.begin();         // Initialize NeoPixel strip
  strip.show();          // Turn off all LEDs initially
  strip.setBrightness(50); // Set brightness to a medium level
}

// Loop function: runs repeatedly as long as the board is on
void loop() {
  startMillis = millis();

  // Display the rainbow effect as the background
  displayRainbow(10); // Pass delay in milliseconds

  // Retrieve and display the current time over the rainbow
  retrieveTime();
  updateClockDisplay();

  // Calculate and display loop duration
  endMillis = millis();
  int loop_time = (endMillis - startMillis);
  char output[100];
  sprintf(output, "Loop time: %d ms\n", loop_time);
  Serial.write(output);
}

// Function to display a rainbow effect across the strip
void displayRainbow(int wait) {
  static long firstPixelHue = 0;

  // Update the hue for the rainbow effect
  for (int i = 0; i < strip.numPixels(); i++) {
    int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue, 255, 100))); // Reduced brightness (value = 100)
  }

  // Send data to the strip
  strip.show();

  // Increment the hue for the next frame
  firstPixelHue += 128;
  firstPixelHue %= 5 * 65536; // Wrap around after one full cycle

  // Delay between frames
  delay(wait);
}

// Function to update the clock display on the strip
void updateClockDisplay() {
  // Overlay clock LEDs over the rainbow effect
  strip.setPixelColor((seconden + 24) % LED_COUNT, strip.Color(0, 0, 255)); // Blue for seconds
  strip.setPixelColor((minuten + 24) % LED_COUNT, strip.Color(0, 255, 0)); // Green for minutes
  strip.setPixelColor(uren % LED_COUNT, strip.Color(255, 0, 0));           // Red for hours
  strip.show();
}

// Function to retrieve and update the current time
void retrieveTime() {
  static unsigned long lastSecondUpdate = 0;

  // Check if a second has passed
  if (millis() - lastSecondUpdate >= 1000) {
    lastSecondUpdate += 1000; // Update the last second timestamp

    seconden++;
    if (seconden >= 60) {
      seconden = 0;
      minuten++;
      if (minuten >= 60) {
        minuten = 0;
        uren++;
        if (uren >= 24) {
          uren = 0;
        }
      }
    }
  }
}
