# Led clock build with wemos d1 mini and ws2812b leds.

I am building a led clock from ws2812b leds, picture frame from ikea, some laser cutting, 3d printing, a wemos d1 mini and some arduino code. This project is to learn to code arduino, lasercutting and some 3d printing. The face of the clock wil be simple, later when the clock is finished i wil ask a groep of creative people that make art, to do something nice with the clock face.


![GitHub Logo](images/led-clock-face.svg)

- parts : 
    - [Wemos d1 mini](https://wiki.wemos.cc/products:d1:d1_mini)
    - [Picture frame ikea 23x23cm](https://www.ikea.com/nl/nl/catalog/products/00378403/)
    - [RGB LED Ring WS2811 ic Built-in RGB DC5V](https://www.aliexpress.com/item/32809169128.html?productId=32809169128&productSubject=Addressable-pixel-WS2812B-Ring-1-8-12-16-24-32-40-48-60-93-241-LEDs&spm=a2g0s.9042311.0.0.38f94c4dSoYWRi)
    - [Level-Shifter (3V to 5V)]( https://www.adafruit.com/product/1787)
    - [Carton from van der linden](https://www.vanderlindewebshop.com/nl (clock face))
    
- Documentation :
  - [Markdown syntax](https://guides.github.com/features/mastering-markdown/)
  - [ws2812b](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)
  - [ws8212b connecters](https://www.aliexpress.com/popular/connector-ws2812b.html)
  - [Voltage regulator](https://www.youtube.com/watch?v=GSzVs7_aW-Y)
  - [Powering NeoPixels](https://learn.adafruit.com/adafruit-neopixel-uberguide/powering-neopixels)
  - Kicad :
    - [Reference designator](https://en.wikipedia.org/wiki/Reference_designator)
    - [An Intro to KiCad](https://www.youtube.com/watch?v=vaCVh2SAZY4)

- Bookmarks :
  - [Thingivers](https://www.thingiverse.com/)
  - [Hackster IO](https://www.hackster.io/)
    
- Software used :
  - [LibreCAD](https://librecad.org/)
  - [Openscad](https://www.openscad.org/)
  - [Kicad EDA](http://kicad-pcb.org/)
  - [Arduino Software (IDE)](https://www.arduino.cc/en/Main/Software)
  
 - Github links :
   - [d1_mini_kicad](https://github.com/jerome-labidurie/d1_mini_kicad)
   - [ESP8266 Wifi connection and configuration manager](https://github.com/mariacarlinahernandez/ConfigManager)
   - [Ping library for ESP8266 Arduino core](https://github.com/dancol90/ESP8266Ping)
   
 - Wemos example code :
    - [Soft access point](https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-examples.html)
    - [Scans for available WiFi networks](https://www.arduino.cc/en/Reference/WiFiScanNetworks)
    - [An Arduino library for ESP8266/ESP32 WLAN configuration at runtime with the Web interface](https://github.com/Hieromon/AutoConnect)
    - [Connect Your ESP8266 To Any Available Wi-Fi network](https://ubidots.com/blog/connect-your-esp8266-to-any-available-wi-fi-network/)

## Some misc bla bla : 

Before connecting a NeoPixel strip to ANY source of power, we very strongly recommend adding a large capacitor (1000 µF, 6.3V or higher) across the + and – terminals. This prevents the initial onrush of current from damaging the pixels.

Adding a 300 to 500 Ohm resistor between your microcontroller's data pin and the data input on the first NeoPixel can help prevent voltage spikes that might otherwise damage your first pixel. Please add one between your micro and NeoPixels!

Use a logic level shifter to step up the signal from the microcontroller to the first pixel.

Things to get : 
- [ ] large capacitor (1000 µF, 6.3V or higher
- [ ] 300 to 500 Ohm resistor
- [ ] logic level shifter 74AHCT125 - Quad Level-Shifter (3V to 5V)   
- [x] Wemos D1 mini

## Authors

* **Perry Couprie** - *Initial work* - [perry-amsterdam](https://github.com/perry-amsterdam)

![GitHub Logo](https://www.ikea.com/nl/nl/images/products/ribba-fotolijst-wit__0638327_PE698851_S4.JPG)
![GitHub Logo](images/ws2812b-leds.jpeg)


