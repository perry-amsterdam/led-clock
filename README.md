# led-clock
Led clock build with arduino and ws2812b.

- parts : 
    - https://wiki.wemos.cc/products:d1:d1_mini
    - https://www.ikea.com/nl/nl/catalog/products/00378403/
    - https://www.aliexpress.com/item/32809169128.html?productId=32809169128&productSubject=Addressable-pixel-WS2812B-Ring-1-8-12-16-24-32-40-48-60-93-241-LEDs&spm=a2g0s.9042311.0.0.38f94c4dSoYWRi
    - https://www.adafruit.com/product/1787

- Documentation :
  - https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf
  - Voltage regulator : https://www.youtube.com/watch?v=GSzVs7_aW-Y

- Kicad :
  - Reference designator : https://en.wikipedia.org/wiki/Reference_designator
  - An Intro to KiCad : https://www.youtube.com/watch?v=vaCVh2SAZY4


Before connecting a NeoPixel strip to ANY source of power, we very strongly recommend adding a large capacitor (1000 µF, 6.3V or higher) across the + and – terminals. This prevents the initial onrush of current from damaging the pixels.

Adding a 300 to 500 Ohm resistor between your microcontroller's data pin and the data input on the first NeoPixel can help prevent voltage spikes that might otherwise damage your first pixel. Please add one between your micro and NeoPixels!

Use a logic level shifter to step up the signal from the microcontroller to the first pixel.
