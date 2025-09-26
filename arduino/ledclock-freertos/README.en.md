[Dutch version](README.en.md)

# LED Clock with ESP32, FreeRTOS & WS2812B

![GitHub License](https://img.shields.io/github/license/<your-username>/<your-repo>)  
![Platform ESP32](https://img.shields.io/badge/platform-ESP32-orange)  
![WS2812B LEDs](https://img.shields.io/badge/LEDs-WS2812B-green)

A **DIY LED clock** built with WS2812B LEDs, an IKEA photo frame, laser-cut parts, and an **ESP32** running on **FreeRTOS**.  
The hardware (clock face, laser cut) is ready ✅. The software runs on a modern ESP32 + FreeRTOS platform and is fully configurable through a captive Wi-Fi portal.

---

## ✨ Features

* Automatic time via **NTP** synchronization  
* **Timezone detection** through IP geolocation  
* **mDNS** discovery on the local network  
* **WPS** connection option (Wi-Fi Protected Setup)  

* Wi-Fi setup: if no connection is found, the clock starts an **Access Point + captive portal**  
* Written in **C/C++ with FreeRTOS**  
* Extensible platform: animations, integrations, and artistic variants  
* Future **API server** + **Flutter app** for remote control  

### FreeRTOS Tasks
- `task_wifi` – connect to Wi-Fi and handle reconnects  
- `task_time` – synchronize and monitor time  
- `task_render` – render the clock on the LED strip  
- `task_portal` – captive portal when no connection  
- `task_led` – update status LED  

---

## 🖥️ Software Setup

1. Install the required tools:  
   * [Arduino CLI](https://arduino.github.io/arduino-cli/latest/) or [Arduino IDE](https://www.arduino.cc/en/software)  
   * ESP32 board support (via Board Manager)  
   * `make` (for the Makefile workflow)  
   * [Flutter](https://flutter.dev/) (optional, for the mobile app)  

2. Use the **Makefile** for a quick workflow:

   ```bash
   make deps       # install esp32 core + libs (Adafruit NeoPixel)
   make build      # compile
   make upload     # flash (PORT=/dev/ttyUSB0)
   make monitor    # serial monitor (115200 baud)
   ```

   Variables:  
   - `PORT` – default `/dev/ttyACM0`  
   - `BOARD` – default `esp32:esp32:esp32s3`  
   - `FLASH_OPTS` – flash/PSRAM/partition settings  

   Example:  
   ```bash
   make PORT=/dev/ttyUSB0 MONITOR_BAUD=115200 monitor
   ```

3. First boot → configure Wi-Fi via the captive portal (`ESP32-Setup` / `configwifi`).

---

## ⚙️ Hardware Setup

* **Clock face** (laser-cut ✅):  
  ![Clock Face](images/led-clock-face.svg)

* **WS2812B connectors**:  
  ![Connector image](images/ws2812b-connecters.png)

* **Parts list**:  
  * IKEA frame 23×23 cm  
  * WS2812B LED strip or ring (5V)  
  * ESP32 devkit (WROOM/WROVER/S3)  
  * Cardboard/laser-cut front plate  

### Connections

- **GPIO 8** → DIN of the **clock LED strip**  
- **GPIO 48** → DIN of the **status LED**  
- **5V & GND** shared between ESP32 and all LEDs  

### 📷 Wiring Diagram

![Wiring Diagram](images/wiring_diagram.png)

> ℹ️ The status LED (GPIO 48) is already on the ESP32-S3 DevKit itself (built-in RGB LED).

### 📋 Pinout Table

| GPIO | Function             | Note                                      |
|------|----------------------|-------------------------------------------|
| 8    | Clock LEDs DIN       | Data-in of WS2812B strip/matrix           |
| 48   | Status LED (onboard) | Built-in RGB LED on ESP32 DevKit          |
| 5V   | Power LEDs           | Ensure enough current capacity            |
| GND  | Ground               | Must be shared with LED power supply      |

### 🔌 Power Consumption (estimates)

| LEDs | 100% brightness (mA) | 100% (A) | ~30% brightness (mA) | ~30% (A) |
|-----:|----------------------:|---------:|----------------------:|---------:|
| 1    | 60                    | 0.06     | 18                    | 0.018    |
| 8    | 480                   | 0.48     | 144                   | 0.144    |
| 30   | 1800                  | 1.80     | 540                   | 0.54     |
| 60   | 3600                  | 3.60     | 1080                  | 1.08     |
| 100  | 6000                  | 6.00     | 1800                  | 1.80     |

**Tips**  
- Choose a power supply with **20–30% margin**  
- Use a **level shifter** for long strips  
- **GND** of the power supply must be connected to ESP32  
- Set `LED_BRIGHTNESS` lower to reduce power and heat  

### NeoPixel Guidelines
1. **Capacitor** – 1000µF, 6.3V+ between V+ and GND  
2. **Resistor** – 300–500Ω in the data line  
3. **Level shifter** – recommended for 3.3V → 5V  

---

### 📡 Wi-Fi via WPS

Besides the captive portal, the clock also supports **WPS (Wi-Fi Protected Setup)** for connecting to your network.  
This is useful if you don’t want to enter Wi-Fi credentials manually.

- **WPS PBC (Push Button Configuration)**: press the WPS button on your router and start the clock in WPS mode.  
- If the ESP32 successfully performs a WPS handshake, the credentials are automatically saved in the chip’s NVS memory.  
- On the next boot, the clock will use the stored credentials directly to connect.  

⚠️ Note: not every ESP32 Arduino core supports WPS.  
Since **ESP32 Arduino core v3+**, `WiFi.beginWPSConfig()` was removed. For those versions, use the captive portal instead.

---

## 🚀 Usage

1. Flash the firmware.  
2. First boot: connect to **ESP32-Setup**.  
3. Open `http://192.168.4.1` and configure Wi-Fi.  
4. ESP32 connects → retrieves time → renders clock on LED strip (GPIO 8).  
5. Status LED (GPIO 48) shows connection status.  

---

## 📸 Images

![Frame](https://www.ikea.com/nl/nl/images/products/ribba-fotolijst-wit__0638327_PE698851_S4.JPG)  
![WS2812B LEDs](images/ws2812b-leds.jpeg)

---

## 🗺️ Roadmap

* [x] Laser-cut clock face  
* [x] Wi-Fi captive portal & NTP synchronization  
* [x] Automatic timezone detection via IP  
* [x] mDNS service  
* [x] Wi-Fi WPS  
* [ ] API server on ESP32  
* [ ] Flutter app integration  
* [ ] Artistic clock variants  
* [ ] Animations & LED effects  
* [ ] Integration with Home Assistant / MQTT  
* [ ] Public release of PCB & 3D files  

---

## 🤝 Contributing

1. Fork the repo and create a feature branch:
   ```bash
   git checkout -b feature/new-feature
   ```
2. Commit your changes with clear messages  
3. Open a Pull Request 🚀  

Tips: document hardware/software changes, add schematics or images, keep commit messages short.

---

## 👤 Author

* **Perry Couprie** – initial development  
  GitHub: [perry-amsterdam](https://github.com/perry-amsterdam)

---

## 📐 CAD Tools

For designing parts such as the clock face and case, you can use free open-source CAD tools:

### LibreCAD
* 2D CAD software, ideal for drawing the **laser-cut front plate**.  
* Website: [https://librecad.org](https://librecad.org)  

**Install on Ubuntu:**
```bash
sudo apt update
sudo apt install librecad
```

### OpenSCAD
* 3D CAD software, suitable for designing **cases and 3D-printable parts**.  
* Website: [https://openscad.org](https://openscad.org)  

**Install on Ubuntu:**
```bash
sudo apt update
sudo apt install openscad
```

Both programs are lightweight and available in the standard Ubuntu repositories.
