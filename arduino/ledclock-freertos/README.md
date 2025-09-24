Hier is een kant-en-klare **README.md** op basis van alle bestanden in je zip. Ik heb uitgelegd wat het project doet, hoe het is opgebouwd, hoe je het bouwt/flash’t, en waar je dingen kunt aanpassen.

---

# ESP32 FreeRTOS WS2812B LED Clock

Een ESP32-S3 (Arduino core v3+) project dat onder **FreeRTOS** draait en een **WS2812B (NeoPixel)** LED-klok aanstuurt.
De klok haalt zelf tijd en tijdzone op via internet, synchroniseert met NTP, en rendert de tijd als “ringen” (60-LED buitenring voor seconden/minuten, 24-LED binnenring voor uren).
Als Wi-Fi niet is geconfigureerd, start een **captive portal** zodat je eenvoudig netwerkgegevens kunt instellen.

## Features

* **FreeRTOS taken** voor LED-status, Wi-Fi, tijdsynchronisatie, captive portal en renderen.
* **Captive portal** (AP `ESP32-Setup`, wachtwoord `configwifi`) op `192.168.4.1` met pagina’s:

  * `/` (form voor SSID/wachtwoord + eenmalige SmartConfig)
  * `/save`, `/reset`, 404 handler
* **Wi-Fi in 3 stappen**:

  1. proberen met opgeslagen SSID/wachtwoord (NVS/Preferences),
  2. optioneel **SmartConfig** (éénmalig, als aangevinkt in het portal),
  3. zo niet: captive portal starten.
* **Tijd & tijdzone**:

  * Probeert IANA-tz en offsets op te halen via `worldtimeapi.org` (backup: `ip-api.com` voor landcode).
  * NTP via `0/1/2.europe.pool.ntp.org`.
  * Fallback TZ naar **Europa/Amsterdam** via POSIX-string als online lookup faalt.
* **Status-LED** (één NeoPixel) met duidelijke signalen:

  * **Blauw pulsen**: Wi-Fi verbinden bezig
  * **Rood pulsen**: fout / niet verbonden
  * **Groen pulsen**: Wi-Fi oké, tijd nog niet klaar
  * **Groen vast**: alles oké
* **Render-engine voor WS2812B** met instelbare ring-oriëntatie, trail-lengtes, tick-marks, kleuren, etc.

> **Let op – Arduino core v3+ vereist**
> In `wifi.cpp` staat een compile-guard die v2.x blokkeert. Wi-Fi provisioning gebruikt **SmartConfig** (core v3) i.p.v. de oude `beginWPSConfig()` API.

---

## Hardware

* **Board:** ESP32-S3 (werkt ook op andere ESP32 varianten met core v3+)
* **Status-LED:** 1x WS2812B op pin **LED\_PIN** (standaard 48 in `config.h`)
* **Klok-strip:** 84 leds totaal: **60** (buitenring) + **24** (binnenring) – configuratie in `ws2812b.cpp`

> Er zijn **twee LED-configuraties**:
>
> 1. **Status-pixel** (1 LED) via `globals.cpp`/`status_led.*` — pin & count in `config.h`
> 2. **Klok-strip** (60+24) via `ws2812b.cpp` — standaardwaarden bovenin het bestand (kun je aanpassen of via compiler-defines overschrijven)

---

## Software-architectuur

### Taken (FreeRTOS)

* `task_led`
  Leest event-bits en zet de **status-LED** (rood/blauw/ groen) – heartbeat bij “alles oké”.
* `task_wifi`
  Probeert verbinding met opgeslagen credentials. Zo niet, **SmartConfig** (als one-shot aangevinkt). Start anders het **captive portal**. Zet `EVT_WIFI_UP` bij succes.
* `task_time`
  Wacht op `EVT_WIFI_UP`. Haalt tz/offsets op (HTTP), configureert NTP en zet `EVT_TIME_READY`. Periodieke her-sync.
* `task_portal`
  Draait DNS/WebServer loop als `EVT_PORTAL_ON` gezet is.
* `task_render`
  Wacht op `EVT_TIME_READY`, initialiseert strip en **rendert \~30 FPS** (aanpasbaar).

### Event bits & queues

* `EVT_WIFI_UP`, `EVT_TIME_READY`, `EVT_PORTAL_ON` in `rtos.h`
* (Er is ook een LED-queue gedefinieerd, nu niet gebruikt — handig voor toekomstige animaties.)

### Bestanden & rollen (belangrijkste)

* **`ledclock-freertos.ino`** – setup (Serial, events, queues) + `xTaskCreatePinnedToCore(...)`
* **`globals.*` / `config.h`** – globale objecten (NeoPixel status, WebServer, DNSServer, Preferences) en constante URLs, NTP, AP-config.
* **`status_led.*`** – eenvoudige helpers voor status-pixel + pulse-patronen.
* **`wifi.*` & `task_wifi.*`** – verbinden, SmartConfig, portal-triggering.
* **`portal.*`** – captive portal HTML/CSS en HTTP-handlers.
* **`net_time.*`** – HTTP lookups (tz/country), NTP/TZ setup, onderhoud.
* **`ws2812b.*`** – klok-renderer (ringen, oriëntatie, trails, kleuren).
* **`rtos.*` / `hal_time_freertos.h`** – event group, (optionele) queues, `hal_millis/hal_delay`.

---

## Builden & flashen

### Vereisten

* **Arduino-ESP32 core v3.x**
  (Board Manager: `esp32` door Espressif, **versie 3 of hoger**)
* **Arduino-CLI** of VSCode/Arduino IDE
* Libraries:

  * **Adafruit NeoPixel**
  * (De rest komt uit de core: `WiFi`, `WebServer`, `DNSServer`, `Preferences`, etc.)

### Met het meegeleverde `Makefile`

Het project bevat een Makefile die Arduino-CLI aanroept (je gebruikte al `make build`):

```bash
# Bouwen
make build

# Uploaden (pas eventueel PORT aan, bv. /dev/ttyACM0 of /dev/ttyUSB0)
make upload PORT=/dev/ttyACM0

# Seriële monitor (standaard 115200 bps)
make monitor

# Opruimen
make clean

# Overzicht van targets/variabelen
make help
```

Belangrijke variabelen (kun je meegeven op de command-line):

* `SKETCH` (standaard `.`)
* `PORT` (seriële poort)
* `UPLOAD_SPEED` (921600 standaard)
* `MONITOR_BAUD` (115200)
* `BUILD_DIR` (bv. `build`)

> Tip: Board-selectie (FQBN) staat in de Makefile/targets; in jouw log zie ik `esp32:esp32:esp32s3` met 16MB flash en PSRAM enabled.

---

## In gebruik nemen

1. **Flash** de firmware.
2. **Eerste boot**:

   * Het device probeert met opgeslagen gegevens te verbinden (als aanwezig).
   * Zo niet, en **SmartConfig** is *one-shot* aangevinkt in het portal (zie hieronder), dan wacht het op **SmartConfig**.
   * Zo niet, start de **captive portal**:

     * AP: **ESP32-Setup**, wachtwoord **configwifi**
     * Verbind met die AP en ga naar **[http://192.168.4.1/](http://192.168.4.1/)**
     * Vul SSID + wachtwoord in en **opslaan**.
3. Na Wi-Fi verbinding: tijd & tijdzone ophalen, NTP synchroniseren.
4. **Status-LED** wordt **groen vast** zodra alles oké is.
5. **Render**: standaard wordt de **secondenring** getekend met korte trail. (Minuten/uren code staat klaar maar is standaard **uitgecommentarieerd** — zie tips hieronder.)

---

## Configuratie & aanpassingen

### Pins & aantallen

* **Status-LED (enkel pixel)**
  `config.h`:

  ```cpp
  #define LED_PIN    48    // pas aan (vaak 8 of 48)
  #define LED_COUNT  1     // status pixel count
  ```
* **Klok-strip (60 + 24 leds)**
  `ws2812b.cpp` (bovenin): `LED_PIN`, `LED_COUNT`, en ringlayout:

  ```cpp
  #define LED_PIN           8
  #define LED_COUNT         84  // 60 + 24
  #define RING60_OFFSET     0
  #define RING60_DIR        +1  // +1 clockwise, -1 counter
  #define RING24_OFFSET     0
  #define RING24_DIR        +1
  #define TRAIL_LENGTH_SEC  2
  #define TRAIL_LENGTH_MIN  1
  ```

  > Pas `*_OFFSET` aan om je “12-uur positie” te alignen met de fysieke bovenkant.

### Wi-Fi provisioning

* **SmartConfig one-shot**: In het portal kun je dit aanvinken. Bij de eerstvolgende reboot wacht het device op SmartConfig (ESP-Touch/Airkiss via telefoon-app). De vlag wordt na die poging automatisch gewist.
* **Opgeslagen SSID/wachtwoord** (NVS/Preferences): namespace `"wifi"` (keys: `"ssid"`, `"pass"`).

### Tijd & tijdzone

* URLs en NTP-pools staan in `globals.cpp`:

  ```cpp
  URL_TIMEINFO = "https://worldtimeapi.org/api/ip";
  URL_COUNTRY  = "https://ip-api.com/json";
  NTP1/2/3     = "0/1/2.europe.pool.ntp.org";
  ```
* Fallback TZ (bijv. offline): POSIX voor **Europa/Amsterdam** — pas aan indien nodig in `net_time.cpp`.

### Rendering

Open `ws2812b.cpp`:

* **Kleuren** (default):

  ```cpp
  // uur rood, minuut groen, seconde blauw
  const uint8_t rHour=180,gHour=0,bHour=0;
  const uint8_t rMin=0,gMin=160,bMin=0;
  const uint8_t rSec=0,gSec=0,bSec=180;
  ```
* **Wat wordt getekend**: standaard zijn **minuten** en **uren** uit-gecommentarieerd om het simpel te houden. Schakel ze in door deze regels te **de-commentariëren**:

  ```cpp
  // drawMinuteTicks();
  // drawHand60(posMin, rMin, gMin, bMin, TRAIL_LENGTH_MIN);
  // addPix(idx24(posHour), rHour, gHour, bHour);
  ```
* **Tickmarks / “minute progress”** op de urenring zijn als voorbeeld aanwezig (eveneens uit-gecommentarieerd).

---

## Troubleshooting

* **Compile-error**: `'WiFiClass' has no member named 'beginWPSConfig'`
  Dit project gebruikt **SmartConfig** en vereist **Arduino-ESP32 core v3+**. Installeer/upgrade de esp32-core naar v3 (Board Manager) en bouw opnieuw.
* **Geen portal zichtbaar**
  Controleer of `EVT_PORTAL_ON` gezet wordt (seriële logs). Verbind met AP **ESP32-Setup**, ga naar **[http://192.168.4.1/](http://192.168.4.1/)**.
  Let op: sommige telefoons forceren een captive portal popup; open anders handmatig een browser.
* **Strip toont niets**
  Verifieer voedingsspanning, **GND gedeeld**, juiste **data-pin** en aantal leds. Pas `LED_PIN/LED_COUNT` in `ws2812b.cpp` aan op jouw hardware.
* **Tijd blijft onjuist**
  Check Wi-Fi verbinding en dat `worldtimeapi.org`/NTP bereikbaar is. Zonder internet gebruikt de code de fallback-TZ en kan initialisatie iets langer duren (log: `[TZ]`).

---

## Uitbreidingen / TODO-ideeën

* Minutes/Hours standaard inschakelen en configurabel maken via compile-time flags.
* **Brightness**/nachtstand, dimmen op tijd/dagdeel.
* **Kleurthema’s** en trails instelbaar via NVS of web-UI.
* OTA-updates (ArduinoOTA / ESP OTA).
* Wi-Fi manager met scanlijst in het portal.
* MQTT/bus integratie (bijv. status publiceren of thema’s kiezen).

---

## Credits

Gebouwd met **Arduino-ESP32 core v3+**, **FreeRTOS** en **Adafruit NeoPixel**.
Projectstructuur en code: taken gescheiden per domein (Wi-Fi, tijd, portal, render, status-LED) voor overzichtelijkheid en stabiliteit.

---

### Korte samenvatting voor de lezer

* Flash de firmware → verbind via portal of SmartConfig → klok haalt tijd op → ringen renderen seconden/minuten/uren.
* Alles is losgekoppeld in FreeRTOS-taken; je kunt per onderdeel makkelijk verder bouwen.
* Pas **pinnen/aantal leds** en **ring-oriëntatie** aan in `config.h` (status-pixel) en `ws2812b.cpp` (klokstrip).
* **Arduino core v3+ is verplicht**.

