---

# LED Clock with ESP32, Zephyr & WS2812B

![GitHub License](https://img.shields.io/github/license/<jouw-username>/<jouw-repo>)
![Build with Zephyr](https://img.shields.io/badge/built%20with-Zephyr-blue)
![Platform ESP32](https://img.shields.io/badge/platform-ESP32-orange)
![WS2812B LEDs](https://img.shields.io/badge/LEDs-WS2812B-green)

Een **DIY LED-klok** gebouwd met WS2812B LEDs, een IKEA fotolijst, lasergesneden onderdelen en een **ESP32** draaiend op **Zephyr RTOS**.
De eerste hardware (klokfront, lasercut) is klaar ✅. De software en verdere integratie worden opnieuw gedaan, nu volledig gebaseerd op **ESP32 in plaats van Arduino**.

---

## ✨ Features

* Internet-synchronisatie via **NTP**
* Automatische **Wi-Fi setup portal** bij eerste gebruik
* Firmware geschreven in **Zephyr RTOS** (C)
* Platform voor uitbreidingen: animaties, slimme integraties, artistieke varianten van de klok face

---

## 🖥️ Software Setup

### Zephyr Development Environment

1. Clone de repo:

   ```bash
   git clone https://github.com/perry-amsterdam/led-clock.git
   cd led-clock
   ```

2. Voer het setup-script uit:

   ```bash
   ./setup-zephyr-env.sh
   ```

   * `--minimal` → installeert alleen Zephyr native
   * `--clean` → verwijdert de hele omgeving
   * Daarna gebruik je:

     * `zephyr-env [PATH]` → activeer Zephyr omgeving en spring naar project
     * `esp-env` → activeer ESP32 toolchain

3. Test build:

   ```bash
   zephyr-env
   cd zephyr/samples/hello_world
   west build -b native_sim -t run
   ```

---

## ⚙️ Hardware Setup

* **Clock front** (lasergesneden ✅):
  ![Clock Face](images/led-clock-face.svg)

* **Onderdelenlijst**:

  * [x] [IKEA frame 23x23cm](https://www.ikea.com/nl/nl/p/sannahed-fotolijst-wit-00459116/)
  * [x] [WS2812B LED Ring (5V)](https://nl.aliexpress.com/item/32808302785.html)
  * [x] Kartonnen/lasergesneden frontplaat
  * [ ] ESP32 devkit (WROOM of WROVER)

---

## 🛠️ Tools & Software

* [LibreCAD](https://librecad.org/) – ontwerp klokplaat
* [OpenSCAD](https://www.openscad.org/) – 3D-modellen
* [KiCad EDA](https://www.kicad.org/) – PCB design (optioneel voor uitbreidingen)
* [Zephyr RTOS](https://zephyrproject.org/) – firmware voor de ESP32

---

## 🔌 Belangrijke richtlijnen voor NeoPixels

1. **Capacitor** – 1000µF, 6.3V+ tussen V+ en GND
2. **Resistor** – 300–500Ω in de datalijn
3. **Level shifter** – indien MCU op 3.3V werkt (ESP32 → 5V voor WS2812B)

➡️ Dit verlengt de levensduur en voorkomt dat de eerste LED stukgaat.

---

## 📸 Afbeeldingen

![Frame](https://www.ikea.com/nl/nl/images/products/ribba-fotolijst-wit__0638327_PE698851_S4.JPG)
![WS2812B LEDs](images/ws2812b-leds.jpeg)

---

## 🗺️ Project Roadmap

* [x] Clock front lasercutten
* [ ] ESP32 hardware integratie
* [ ] Wi-Fi captive portal & NTP synchronisatie
* [ ] Clock face design uitbreiden met artistieke varianten
* [ ] Animaties en extra LED-effecten toevoegen
* [ ] Integratie met slimme thuisplatformen (Home Assistant / MQTT)
* [ ] Public release van PCB design & 3D files

---

## 🤝 Contributing

Wil je meebouwen aan dit project? Super!

* Fork de repo en maak je eigen branch:

  ```bash
  git checkout -b feature/nieuwe-functie
  ```
* Doe je aanpassingen en commit met duidelijke messages
* Stuur een Pull Request 🚀

**Tips voor bijdragen:**

* Documenteer nieuwe hardware of software duidelijk
* Voeg afbeeldingen of schema’s toe waar mogelijk
* Houd commit messages kort en informatief

---

## 👤 Auteur

* **Perry Couprie** – initiële ontwikkeling

  * GitHub: [perry-amsterdam](https://github.com/perry-amsterdam)

