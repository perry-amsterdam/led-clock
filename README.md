# Led clock build with wemos d1 mini and ws2812b leds.

I am building a led clock from ws2812b leds, picture frame from ikea, some laser cutting, 3d printing, a wemos d1 mini and some arduino code. This project is to learn to code arduino, lasercutting and some 3d printing. The face of the clock wil be simple, later when the clock is finished i wil ask a groep of creative people that make art, to do something nice with the clock face.

## Clock setup

The clock must be connected to internet to retrieve time information from a ntp (time server). To connect to the interet the clock must connect to a wifi ap. When the clock starts up for the first time, it can not connect to a wifi network. When the clock can not connect to a wifi network itself wil become a wireless acces point, the clock wil also become captive portal. A website to setup the clock, connect to internet an set a time zone. To do that you connect to de clock wifi AP with a phone/table/latop, after connecting to the AP you can browse to www.clocksetup to setup the clock.

![GitHub Logo](images/led-clock-face.svg)

- Parts used and link to where you can buy it: 
    - [x] [Wemos d1 mini](https://wiki.wemos.cc/products:d1:d1_mini)
    - [x] [Picture frame ikea 23x23cm](https://www.ikea.com/nl/nl/catalog/products/00378403/)
    - [x] [RGB LED Ring WS2811 ic Built-in RGB DC5V](https://www.aliexpress.com/item/32809169128.html?productId=32809169128&productSubject=Addressable-pixel-WS2812B-Ring-1-8-12-16-24-32-40-48-60-93-241-LEDs&spm=a2g0s.9042311.0.0.38f94c4dSoYWRi)
    - [x] [Carton from van der linden](https://www.vanderlindewebshop.com/nl (clock face))
    - [x] large capacitor (1000 µF, 6.3V or higher
    - [x] 300 to 500 Ohm resistor
    - [ ] [logic level shifter](https://www.aliexpress.com/item/32690305593.html?scm=1007.22893.125764.0&pvid=18a7fa26-fe89-4671-9f94-3089decc64a8&onelink_thrd=0.0&onelink_page_from=ITEM_DETAIL&onelink_item_to=32690305593&onelink_duration=0.855963&onelink_status=noneresult&onelink_item_from=32690305593&onelink_page_to=ITEM_DETAIL&aff_platform=link-c-tool&cpt=1559000965592&sk=B2vRnAe&aff_trace_key=cfd080f5559840aa8140893bf65e942c-1559000965592-03480-B2vRnAe&terminal_id=7269e94812514d06b6235e744390b183)   
- Software used :
  - [LibreCAD](https://librecad.org/)
  - [Openscad](https://www.openscad.org/)
  - [Kicad EDA](http://kicad-pcb.org/)
  - [Arduino Software (IDE)](https://www.arduino.cc/en/Main/Software)
  - Libaries :
    - [Ping library for ESP8266 Arduino core](https://github.com/dancol90/ESP8266Ping)
    - [ESP8266 Wifi connection and configuration manager](https://github.com/mariacarlinahernandez/ConfigManager)
    - [Time library for Arduino](https://github.com/PaulStoffregen/Time)
    - [Arduino library to facilitate time zone conversions and automatic daylight saving (summer) time adjustments.](https://github.com/JChristensen/Timezone)
- Documentation :
  - [Markdown syntax](https://guides.github.com/features/mastering-markdown/)
  - [ws2812b](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)
  - [ws8212b connecters](https://www.aliexpress.com/popular/connector-ws2812b.html)
  - [Voltage regulator](https://www.youtube.com/watch?v=GSzVs7_aW-Y)
  - [Powering NeoPixels](https://learn.adafruit.com/adafruit-neopixel-uberguide/powering-neopixels)
  - Kicad :
    - [Reference designator](https://en.wikipedia.org/wiki/Reference_designator)
      
 - Github links :
   - [d1_mini_kicad](https://github.com/jerome-labidurie/d1_mini_kicad)
   - [An Arduino library for ESP8266/ESP32 WLAN configuration at runtime with the Web interface](https://github.com/Hieromon/AutoConnect)
 - Wemos example code :
    - [Soft access point](https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-examples.html)
    - [Scans for available WiFi networks](https://www.arduino.cc/en/Reference/WiFiScanNetworks)
    - [Connect Your ESP8266 To Any Available Wi-Fi network](https://ubidots.com/blog/connect-your-esp8266-to-any-available-wi-fi-network/)
- Bookmarks :
  - [Thingivers](https://www.thingiverse.com/)
  - [Hackster IO](https://www.hackster.io/)
  - [Instructables is a community for people who like to make things. ](https://www.instructables.com/)
  - [ Arduino, ESP8266 & Raspberry Pi stuff : MQTT for beginners](https://arduinodiy.wordpress.com/2017/11/24/mqtt-for-beginners/)
  - [CoAP RFC 7252 Constrained Application Protocol](https://coap.technology/)
  - Embedded OS
    - [Contiki: The Open Source OS for the Internet of Things](http://www.contiki-os.org/)
    - [Riot-os: The friendly Operating System for the Internet of Things](http://www.riot-os.org/)
  - Zigbee
    - [How to sniff Zigbee traffic](http://www.zigbee2mqtt.io/how_tos/how_to_sniff_zigbee_traffic.html)
    - [Wireshark ZigBee Sniffer](https://www.cd-jackson.com/downloads/ZigBeeWiresharkSniffer.pdf)
    - [CC2531 stick](https://nl.aliexpress.com/af/cc2531.html?d=y&origin=n&spm=a2g0z.search0604.8.7.784a2a7043niRm&jump=afs&productId=32982628286&SearchText=cc2531&initiative_id=QRW_20190505060041&isViewCP=y) 
    - https://www.instructables.com/id/Solar-Powered-WiFi-Weather-Station/?utm_content=buffera44cb&utm_medium=social&utm_source=pinterest.com&utm_campaign=buffer
    - https://www.switchdoc.com/2015/10/iot-esp8266-timer-tutorial-arduino-ide/
    
  
Software :
- Install arduino libary : NTPClient

## Some misc bla bla : 

Before connecting a NeoPixel strip to ANY source of power, we very strongly recommend adding a large capacitor (1000 µF, 6.3V or higher) across the + and – terminals. This prevents the initial onrush of current from damaging the pixels.

Adding a 300 to 500 Ohm resistor between your microcontroller's data pin and the data input on the first NeoPixel can help prevent voltage spikes that might otherwise damage your first pixel. Please add one between your micro and NeoPixels!

Use a logic level shifter to step up the signal from the microcontroller to the first pixel.

## Authors

* **Perry Couprie** - *Initial work* - [perry-amsterdam](https://github.com/perry-amsterdam)

![GitHub Logo](https://www.ikea.com/nl/nl/images/products/ribba-fotolijst-wit__0638327_PE698851_S4.JPG)
![GitHub Logo](images/ws2812b-leds.jpeg)


