---

# README – ESP32-S3 KiCad Library Import

Deze handleiding legt uit hoe je de officiële **Espressif KiCad libraries** installeert en gebruikt voor de **ESP32-S3-WROOM-1-N16R8** module.

---

## 📦 1. Library downloaden

De ESP32-symbolen en footprints worden onderhouden door Espressif zelf.

Download of clone de officiële repo:

```bash
git clone https://github.com/espressif/kicad-libraries.git
```

De repo bevat onder andere:

* **Symboolbibliotheek**: `ESP32.kicad_sym`
* **Footprintbibliotheek**: `ESP32.pretty/`

---

## ⚙️ 2. Symbol Library toevoegen

1. Open **KiCad** → menu **Preferences → Manage Symbol Libraries…**
2. Kies tabblad **Project specific libraries** (aanbevolen, dan blijft je project portable).
3. Klik **Add existing library** (map-icoon met een plusje).
4. Navigeer naar de gedownloade repo → kies bestand:

   ```
   kicad-libraries/symbols/ESP32.kicad_sym
   ```
5. Geef het een herkenbare naam, bijv. `ESP32`.

---

## ⚙️ 3. Footprint Library toevoegen

1. Open **Preferences → Manage Footprint Libraries…**
2. Kies weer **Project specific libraries**.
3. Klik **Add existing library**.
4. Navigeer naar de repo → map:

   ```
   kicad-libraries/footprints/ESP32.pretty
   ```
5. Kies deze map en geef de naam `ESP32`.

---

## 🖼 4. Component gebruiken

1. Open je schema-editor.
2. Voeg een nieuw component toe (`A`-toets).
3. Zoek naar **ESP32-S3-WROOM-1** in de `ESP32` library.
4. Plaats het symbool in je schema.
5. Controleer dat de footprint correct gekoppeld is:

   * `ESP32-S3-WROOM-1` → footprint uit `ESP32.pretty`.

---

## 📐 5. Layout & Design Rules

* **Antennegebied vrijhouden**: geen koper, groundplane of componenten onder het antenne-gedeelte van de module.
* **Decoupling condensatoren**: plaats meerdere 100 nF + 10 µF zo dicht mogelijk bij de 3V3 pin.
* **UART & BOOT**: zorg voor toegang tot `EN`, `IO0`, `TXD0`, `RXD0` voor programmering.
* Raadpleeg het [ESP32-S3-WROOM-1 datasheet](https://www.espressif.com/en/support/documents/technical-documents) voor het aanbevolen land pattern en referentieschema.

---

## ✅ Samenvatting

Met deze stappen heb je:

* Het **ESP32-S3 symbool** in je schema-editor.
* De juiste **footprint** in je PCB-editor.
* Altijd up-to-date libraries rechtstreeks van Espressif.

