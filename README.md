# Led clock build with wemos d1 mini and ws2812b leds.

I am building a led clock from ws2812b leds, picture frame from ikea, some laser cutting, 3d printing, a wemos d1 mini and some arduino code. This project is to learn to code arduino, lasercutting and some 3d printing. The face of the clock wil be simple, later when the clock is finished i wil ask a groep of creative people that make art, to do something nice with the clock face.

## Clock setup

The clock must be connected to internet to retrieve time information from a ntp (time server). To connect to the interet the clock must connect to a wifi ap. When the clock starts up for the first time, it can not connect to a wifi network. When the clock can not connect to a wifi network itself wil become a wireless acces point, the clock wil also become captive portal. A website to setup the clock, connect to internet an set a time zone. To do that you connect to de clock wifi AP with a phone/table/latop, after connecting to the AP you can browse to www.clocksetup to setup the clock.

![GitHub Logo](images/led-clock-face.svg)

- Parts used and link to where you can buy it: 
    - [x] [Wemos d1 mini](https://wiki.wemos.cc/products:d1:d1_mini)
    - [x] [Picture frame ikea 23x23cm](https://www.ikea.com/nl/nl/catalog/products/00378403/)
    - [ ] [RGB LED Ring WS2811 ic Built-in RGB DC5V](https://www.aliexpress.com/item/32809169128.html?productId=32809169128&productSubject=Addressable-pixel-WS2812B-Ring-1-8-12-16-24-32-40-48-60-93-241-LEDs&spm=a2g0s.9042311.0.0.38f94c4dSoYWRi)
    - [x] [Carton from van der linden](https://www.vanderlindewebshop.com/nl (clock face))
    Things to get : 
    - [ ] large capacitor (1000 µF, 6.3V or higher
    - [ ] [Electrolytic Capacitor 1000uf 16v](https://www.aliexpress.com/item/A1-free-shipping-50pcs-Aluminum-electrolytic-capacitor-1000uf-16v-8-16-Electrolytic-capacitor/32418323423.html?spm=2114.search0104.3.1.7ac44ff0z6KCGE&ws_ab_test=searchweb0_0,searchweb201602_1_10065_5729911_10068_319_317_10696_5728811_453_10084_454_10083_10618_10304_10307_10820_10301_10821_537_536_5730811_10843_5733211_328_5733311_10059_10884_5733411_10887_5730011_100031_5731011_321_322_10103_5729011_5733611_5733111_5733511-5729911,searchweb201603_55,ppcSwitch_0&algo_expid=30318461-2622-44fb-a704-914d5081f388-0&algo_pvid=30318461-2622-44fb-a704-914d5081f388)

    - [ ] 300 to 500 Ohm resistor
    - [ ] logic level shifter 74AHCT125 - Quad Level-Shifter (3V to 5V)   
- Software used :
  - [LibreCAD](https://librecad.org/)
  - [Openscad](https://www.openscad.org/)
  - [Kicad EDA](http://kicad-pcb.org/)
  - [Arduino Software (IDE)](https://www.arduino.cc/en/Main/Software)
  - Libaries :
    - [Ping library for ESP8266 Arduino core](https://github.com/dancol90/ESP8266Ping)
    - [ESP8266 Wifi connection and configuration manager](https://github.com/mariacarlinahernandez/ConfigManager)
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
  - [ Arduino, ESP8266 & Raspberry Pi stuff : MQTT for beginners](https://arduinodiy.wordpress.com/2017/11/24/mqtt-for-beginners/)
  - [Contiki: The Open Source OS for the Internet of Things](http://www.contiki-os.org/)
  - [Riot-os: The friendly Operating System for the Internet of Things](http://www.riot-os.org/)
  - [CoAP RFC 7252 Constrained Application Protocol](https://coap.technology/)
  - [How to sniff Zigbee traffic](http://www.zigbee2mqtt.io/how_tos/how_to_sniff_zigbee_traffic.html)
  - [Wireshark ZigBee Sniffer](https://www.cd-jackson.com/downloads/ZigBeeWiresharkSniffer.pdf)
  - [wireless analysis of 868 mhz traffic with an rtl-sdr and the traffic detective software[(https://www.rtl-sdr.com/wireless-analysis-of-868-mhz-traffic-with-an-rtl-sdr-and-the-traffic-detective-software/)
  
 - Github links :
   - [d1_mini_kicad](https://github.com/jerome-labidurie/d1_mini_kicad)
   - [An Arduino library for ESP8266/ESP32 WLAN configuration at runtime with the Web interface](https://github.com/Hieromon/AutoConnect)
 - Wemos example code :
    - [Soft access point](https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-examples.html)
    - [Scans for available WiFi networks](https://www.arduino.cc/en/Reference/WiFiScanNetworks)
    - [Connect Your ESP8266 To Any Available Wi-Fi network](https://ubidots.com/blog/connect-your-esp8266-to-any-available-wi-fi-network/)

## Some misc bla bla : 

Before connecting a NeoPixel strip to ANY source of power, we very strongly recommend adding a large capacitor (1000 µF, 6.3V or higher) across the + and – terminals. This prevents the initial onrush of current from damaging the pixels.

Adding a 300 to 500 Ohm resistor between your microcontroller's data pin and the data input on the first NeoPixel can help prevent voltage spikes that might otherwise damage your first pixel. Please add one between your micro and NeoPixels!

Use a logic level shifter to step up the signal from the microcontroller to the first pixel.

## Authors

* **Perry Couprie** - *Initial work* - [perry-amsterdam](https://github.com/perry-amsterdam)

![GitHub Logo](https://www.ikea.com/nl/nl/images/products/ribba-fotolijst-wit__0638327_PE698851_S4.JPG)
![GitHub Logo](images/ws2812b-leds.jpeg)


