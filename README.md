# Led clock build with wemos d1 mini and ws2812b leds.

I am building a led clock from ws2812b leds, picture frame from ikea, some laser cutting, 3d printing, a wemos d1 mini and some arduino code. This project is to learn to code arduino, lasercutting and some 3d printing. The face of the clock wil be simple, later when the clock is finished i wil ask a groep of creative people that make art, to do something nice with the clock face.

## Clock setup

The clock must be connected to internet to retrieve time information from a ntp (time server). To connect to the interet the clock must connect to a wifi ap. When the clock starts up for the first time, it can not connect to a wifi network. When the clock can not connect to a wifi network itself wil become a wireless acces point, the clock wil also become captive portal. A website to setup the clock, connect to internet an set a time zone. To do that you connect to de clock wifi AP with a phone/table/latop, after connecting to the AP you can browse to www.clocksetup to setup the clock.

![GitHub Logo](images/led-clock-face.svg)

- Parts used and link to where you can buy it: 
    - [x] [Wemos d1 mini](https://wiki.wemos.cc/products:d1:d1_mini)
    - [x] [Picture frame ikea 23x23cm](https://www.ikea.com/nl/nl/p/sannahed-fotolijst-wit-00459116/)
    - [x] [RGB LED Ring WS2811 ic Built-in RGB DC5V](https://nl.aliexpress.com/item/32808302785.html?spm=a2g0o.productlist.main.3.611d6920dI6tu2&algo_pvid=460d9791-20b6-4035-93b0-249b08dbe596&algo_exp_id=460d9791-20b6-4035-93b0-249b08dbe596-1&pdp_npi=4%40dis%21EUR%218.02%215.29%21%21%218.27%215.45%21%40211b664d17327929306932519eff7c%2112000038357392441%21sea%21NL%213900209338%21ABX&curPageLogUid=iWddPClHIu3S&utparam-url=scene%3Asearch%7Cquery_from%3A)
    - [x] [Carton from van der linden](https://www.vanderlindewebshop.com/nl (clock face))
    - [x] large capacitor (1000 µF, 6.3V or higher
    - [x] 300 to 500 Ohm resistor
    - [ ] [logic level shifter](https://www.aliexpress.com/item/32690305593.html?scm=1007.22893.125764.0&pvid=18a7fa26-fe89-4671-9f94-3089decc64a8&onelink_thrd=0.0&onelink_page_from=ITEM_DETAIL&onelink_item_to=32690305593&onelink_duration=0.855963&onelink_status=noneresult&onelink_item_from=32690305593&onelink_page_to=ITEM_DETAIL&aff_platform=link-c-tool&cpt=1559000965592&sk=B2vRnAe&aff_trace_key=cfd080f5559840aa8140893bf65e942c-1559000965592-03480-B2vRnAe&terminal_id=7269e94812514d06b6235e744390b183)
    - [ ] [barrel jack](https://nl.aliexpress.com/af/barrel-jack.html?SearchText=barrel+jack&d=y&initiative_id=SB_20190605015720&origin=n&catId=0&isViewCP=y&jump=afs)
    - [ ] https://learn.sparkfun.com/tutorials/connector-basics/power-connectors
- Software used :
  - [LibreCAD](https://librecad.org/)
  - [Openscad](https://www.openscad.org/)
  - [Kicad EDA](https://www.kicad.org/)
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
  
 - Github links :
   - [d1_mini_kicad](https://github.com/jerome-labidurie/d1_mini_kicad)
   - [An Arduino library for ESP8266/ESP32 WLAN configuration at runtime with the Web interface](https://github.com/Hieromon/AutoConnect)
 - Wemos example code :
    - [Soft access point](https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-examples.html)
    - [Scans for available WiFi networks](https://www.arduino.cc/en/Reference/WiFiScanNetworks)
    - [Connect Your ESP8266 To Any Available Wi-Fi network](https://ubidots.com/blog/connect-your-esp8266-to-any-available-wi-fi-network/)

Software :
- Install arduino libary : NTPClient
# Important Guidelines for Connecting NeoPixel Strips

To ensure the longevity and reliable performance of your NeoPixel strip, follow these essential precautions:

## 1. Add a Capacitor
Before connecting the NeoPixel strip to any power source, it is crucial to include a large capacitor across the **+** and **–** terminals.  
- **Recommended specification**: 1000 µF, 6.3V or higher.  
- **Purpose**: This prevents the initial surge of current from damaging the LEDs during power-up.

## 2. Install a Resistor on the Data Line
Protect the data input of your first NeoPixel by adding a resistor between the microcontroller’s data pin and the data input of the strip.  
- **Recommended value**: 300 to 500 Ohms.  
- **Purpose**: This helps suppress voltage spikes, which could otherwise harm the first pixel.

## 3. Use a Logic Level Shifter
If your microcontroller operates at a lower voltage (e.g., 3.3V), use a logic level shifter to step up the signal to the required level for NeoPixels (typically 5V).  
- **Purpose**: Ensures proper communication and prevents signal issues with the first pixel.

By following these guidelines, you can protect your NeoPixel strip and improve its reliability in your projects.

## Authors

* **Perry Couprie** - *Initial work* - [perry-amsterdam](https://github.com/perry-amsterdam)

![GitHub Logo](https://www.ikea.com/nl/nl/images/products/ribba-fotolijst-wit__0638327_PE698851_S4.JPG)
![GitHub Logo](images/ws2812b-leds.jpeg)


