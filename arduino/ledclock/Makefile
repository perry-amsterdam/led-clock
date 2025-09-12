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
.PHONY: setup libs build upload all monitor flash clean board-details board-list erase-flash

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
# Volledige chip erase (wist ook filesystem/NVS). Vereist: esptool.py
erase-flash:
	esptool.py --chip esp32s3 --port $(PORT) erase_flash

