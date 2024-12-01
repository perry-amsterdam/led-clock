# LED Clock Build with Wemos D1 Mini and WS2812B LEDs

I’m building an LED clock using WS2812B LEDs, a picture frame from IKEA, laser cutting, 3D printing, a Wemos D1 Mini, and Arduino programming. This project is not only about creating a functional clock but also serves as a hands-on way to learn and experiment with Arduino programming, laser cutting, and 3D printing techniques.

The clock face will start with a simple, functional design. Once the initial build is complete, I plan to collaborate with a group of talented artists to give the clock face an artistic makeover.

The software behind the clock is designed to be flexible, allowing programmers to modify it and change the clock’s behavior. This opens up possibilities for customization and unique features, making the clock as much a platform for creativity as it is a timepiece.
## Clock Setup

The clock will connect to the internet to retrieve time information from an NTP (Network Time Protocol) server. To do this, it needs to connect to a Wi-Fi access point (AP).  

### First-Time Setup
If the clock cannot connect to a Wi-Fi network during its initial setup, it will automatically act as a wireless access point and captive portal. A setup webpage will allow users to configure the clock's connection to the internet and set the time zone.  

### How to Configure:
1. Connect your phone, tablet, or laptop to the clock's Wi-Fi AP.  
2. Open a browser and navigate to **www.clocksetup**.  
3. Use the setup page to:
   - Connect the clock to your Wi-Fi network.  
   - Configure the time zone.  

Once configured, the clock will switch back to its normal mode and sync time from the NTP server.

# Clock front:

![GitHub Logo](images/led-clock-face.svg)

- Parts used and link to where you can buy it: 
    - [x] [Wemos d1 mini](https://wiki.wemos.cc/products:d1:d1_mini)
    - [x] [Picture frame ikea 23x23cm](https://www.ikea.com/nl/nl/p/sannahed-fotolijst-wit-00459116/)
    - [x] [RGB LED Ring WS2811 ic Built-in RGB DC5V](https://nl.aliexpress.com/item/32808302785.html?spm=a2g0o.productlist.main.3.611d6920dI6tu2&algo_pvid=460d9791-20b6-4035-93b0-249b08dbe596&algo_exp_id=460d9791-20b6-4035-93b0-249b08dbe596-1&pdp_npi=4%40dis%21EUR%218.02%215.29%21%21%218.27%215.45%21%40211b664d17327929306932519eff7c%2112000038357392441%21sea%21NL%213900209338%21ABX&curPageLogUid=iWddPClHIu3S&utparam-url=scene%3Asearch%7Cquery_from%3A)
    - [x] [Carton from van der linden](https://www.vanderlindewebshop.com/nl (clock face))
    - [x] large capacitor (1000 µF, 6.3V or higher
    - [x] 300 to 500 Ohm resistor

- Software used :
  - [LibreCAD](https://librecad.org/)
  - [Openscad](https://www.openscad.org/)
  - [Kicad EDA](https://www.kicad.org/)
  - [Arduino Software (IDE)](https://www.arduino.cc/en/Main/Software)
  - Libaries :
    
https://github.com/JChristensen/Timezone)
- Documentation :
  - [Markdown syntax](https://guides.github.com/features/mastering-markdown/)
  - [ws2812b](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)
  - [ws8212b connecters](https://www.aliexpress.com/popular/connector-ws2812b.html)
  - [Voltage regulator](https://www.youtube.com/watch?v=GSzVs7_aW-Y)
  - [Powering NeoPixels](https://learn.adafruit.com/adafruit-neopixel-uberguide/powering-neopixels)
  
 - Github links :
   - [d1_mini_kicad](https://github.com/jerome-labidurie/d1_mini_kicad)
   - [An Arduino library for ESP8266/ESP32 WLAN configuration at runtime with the Web interface](https://github.com/Hieromon/AutoConnect)
 
# Important Guidelines for Connecting NeoPixel Strips

To ensure the longevity and reliable performance of your NeoPixel strip, follow these essential precautions:

1. Add a Capacitor
Before connecting the NeoPixel strip to any power source, it is crucial to include a large capacitor across the **+** and **–** terminals.  
- **Recommended specification**: 1000 µF, 6.3V or higher.  
- **Purpose**: This prevents the initial surge of current from damaging the LEDs during power-up.

2. Install a Resistor on the Data Line
Protect the data input of your first NeoPixel by adding a resistor between the microcontroller’s data pin and the data input of the strip.  
- **Recommended value**: 300 to 500 Ohms.  
- **Purpose**: This helps suppress voltage spikes, which could otherwise harm the first pixel.

3. Use a Logic Level Shifter
If your microcontroller operates at a lower voltage (e.g., 3.3V), use a logic level shifter to step up the signal to the required level for NeoPixels (typically 5V).  
- **Purpose**: Ensures proper communication and prevents signal issues with the first pixel.

By following these guidelines, you can protect your NeoPixel strip and improve its reliability in your projects.

## Authors

* **Perry Couprie** - *Initial work* - [perry-amsterdam](https://github.com/perry-amsterdam)

![GitHub Logo](https://www.ikea.com/nl/nl/images/products/ribba-fotolijst-wit__0638327_PE698851_S4.JPG)
![GitHub Logo](images/ws2812b-leds.jpeg)


