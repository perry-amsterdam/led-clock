ESPTOOL := $(shell command -v esptool.py 2>/dev/null)
ESPTOOL_CMD := $(if $(ESPTOOL),$(ESPTOOL),$(PY) -m esptool)

BUILD_DIR ?= build
PY ?= python3
PORT ?= /dev/ttyACM0
CHIP ?= esp32s3

help:
	@echo "Beschikbare targets:"
	@echo "  make build         - Compileer de sketch met arduino-cli"
	@echo "  make upload        - Upload de sketch (PORT=$(PORT))"
	@echo "  make upload-clean  - Wis hele flash, bouw en upload (compleet schoon)"
	@echo "  make clean         - Verwijder de build directory"
	@echo "  make monitor       - Open seriële monitor (PORT=$(PORT))"
	@echo "  make erase-all     - Wis volledige flash (gebruikt esptool uit jouw venv/omgeving)"
	@echo "  make erase-nvs     - Wis alleen NVS (WiFi-credentials)"
	@echo "  make erase-ffat    - Wis alleen FFAT filesystem"
	@echo "  make deps          - Toon welke esptool gebruikt wordt / instructies"
	@echo "Variabelen: PORT=$(PORT), CHIP=$(CHIP), PY=$(PY)"

# Makefile voor ESP32-S3 N16R8 met Arduino CLI
# Gebruik:
#   make setup           # core installeren (eenmalig)
#   make build           # compile
#   make upload          # upload naar /dev/ttyACM0
#   make monitor         # seriële monitor
#   make all             # build + upload
#   make flash           # build + upload + monitor
#   make erase-flash     # VOLLEDIGE flash wissen (incl. filesystem/NVS)
#   make board-details   # toon exacte board-opties
#   make board-list      # toon alle ESP32 borden

# ======= Config =======
CORE        := esp32:esp32
BOARD       := esp32s3
BOARD_OPTS  := FlashSize=16M,PSRAM=enabled,PartitionScheme=app3M_fat9M_16MB

# Vast ingestelde poort en baudrate
PORT        := /dev/ttyACM0
BAUD        := 115200

# Paden
SKETCH      := .
BUILD_DIR   := build

# ======= Afleidingen =======
FQBN := $(CORE):$(BOARD):$(BOARD_OPTS)

# ======= Targets =======
.PHONY: setup libs build upload all monitor flash clean board-details board-list erase-flash help erase-all erase-nvs erase-ffat deps upload-clean
deps:
	if [ -n "$(ESPTOOL)" ]; then \
	  echo "✅ esptool.py gevonden: $(ESPTOOL)"; \
	  echo "Ik gebruik deze omgeving/venv automatisch."; \
	else \
	  echo "⚠️  esptool.py ontbreekt in PATH. Ik val terug op: $(PY) -m esptool"; \
	  echo "Installeer via:  sudo apt install python3-esptool"; \
	  echo "Of gebruik venv: python3 -m venv .venv && . .venv/bin/activate && pip install esptool"; \
	  echo "(Let op: zonder esptool werken erase-targets niet correct)"; \
	fi

setup:
	arduino-cli core update-index
	arduino-cli core install $(CORE)

libs:
	@echo "Plaats extra 'arduino-cli lib install ...' regels hier indien nodig"

build:
	arduino-cli compile --fqbn $(FQBN) --build-path $(BUILD_DIR) $(SKETCH)

upload: build
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) --input-dir $(BUILD_DIR)

all: build upload

monitor:
	arduino-cli monitor -p $(PORT) -c baudrate=$(BAUD)

# Build + upload + automatisch monitor starten
flash: upload
	@sleep 1
	@$(MAKE) monitor

clean:
	rm -rf "$(BUILD_DIR)"

board-details:
	@echo ">>> Board details voor $(CORE):$(BOARD)"
	arduino-cli board details --fqbn "$(CORE):$(BOARD)" --full
	@echo
	@echo ">>> Opties die je in BOARD_OPTS kunt gebruiken (namen exact overnemen)"
	arduino-cli board details --fqbn "$(CORE):$(BOARD)"

board-list:
	@echo ">>> Aangesloten borden:"
	arduino-cli board list
	@echo
	@echo ">>> Alle ESP32 varianten:"
	arduino-cli board listall esp32 | sed -n '1,120p' || true

# ---- Gevraagd: filesystem wissen ----
# Volledige chip erase (wist ook filesystem/NVS). Vereist: python3 -m esptool
erase-flash:
	python3 -m esptool --chip esp32s3 --port $(PORT) erase_flash

erase-all:
	$(ESPTOOL_CMD) --chip $(CHIP) --port $(PORT) erase_flash

erase-nvs:
	$(ESPTOOL_CMD) --chip $(CHIP) --port $(PORT) erase_region 0x9000 0x5000

erase-ffat:
	$(ESPTOOL_CMD) --chip $(CHIP) --port $(PORT) erase_region 0x610000 0x9E0000

upload-clean:
	$(MAKE) erase-all
	$(MAKE) build
	$(MAKE) upload