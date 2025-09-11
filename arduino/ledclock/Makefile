# Makefile voor ESP32-S3 N16R8 met Arduino CLI
# Gebruik:
#   make setup           # core installeren (eenmalig)
#   make build           # compile
#   make upload PORT=/dev/ttyUSB0   # upload
#   make monitor PORT=/dev/ttyUSB0  # seriële monitor
#   make all PORT=/dev/ttyUSB0      # build + upload
#   make board-details   # toon exacte board-opties
#   make board-list      # toon alle ESP32 borden

# ======= Config =======
CORE          ?= esp32:esp32
BOARD         ?= esp32s3                       # "ESP32S3 Dev Module"
FQBN_BASE     := $(CORE):$(BOARD)

# Kies de juiste opties voor jouw module:
# - FlashSize=16M  -> 16MB flash
# - PSRAM=enabled  -> 8MB PSRAM aan
# - PartitionScheme=app3M_fat9M_16MB  -> handige 16MB indeling (3MB app / 9MB FATFS)
#   (pas dit aan naar jouw wens: huge_app, min_spiffs, etc.)
BOARD_OPTS    ?= FlashSize=16M,PSRAM=enabled,PartitionScheme=app3M_fat9M_16MB

# Poort & snelheid
PORT          ?=                                  # bv. /dev/ttyUSB0 of /dev/ttyACM0
BAUD          ?= 115200

# Paden
SKETCH        ?= .                                 # map of .ino-bestand
BUILD_DIR     ?= build

# ======= Interne afleidingen =======
# Volledige FQBN met opties (opties mogen ook bij --board-options)
FQBN := $(FQBN_BASE):$(BOARD_OPTS)

# ======= Targets =======
.PHONY: setup build upload all clean monitor board-details board-list libs

setup:
	arduino-cli core update-index
	arduino-cli core install $(CORE)

libs:
	@echo "Plaats extra 'arduino-cli lib install ...' regels hier indien nodig"

build:
	@test -n "$(FQBN_BASE)" || (echo "FQBN_BASE leeg"; exit 1)
	arduino-cli compile \
		--fqbn "$(FQBN)" \
		--build-path "$(BUILD_DIR)" \
		"$(SKETCH)"

upload: build
	@test -n "$(PORT)" || (echo "Geef de seriële poort mee: make upload PORT=/dev/ttyUSB0"; exit 2)
	arduino-cli upload \
		--port "$(PORT)" \
		--fqbn "$(FQBN)" \
		--input-dir "$(BUILD_DIR)" \
		"$(SKETCH)"

all: build upload

monitor:
	@test -n "$(PORT)" || (echo "Geef de seriële poort mee: make monitor PORT=/dev/ttyUSB0"; exit 2)
	arduino-cli monitor -p "$(PORT)" -c baudrate=$(BAUD)

board-details:
	@echo ">>> Board details voor $(FQBN_BASE)"
	arduino-cli board details --fqbn "$(FQBN_BASE)" --full
	@echo
	@echo ">>> Opties die je in BOARD_OPTS kunt gebruiken (namen exact overnemen)"
	arduino-cli board details --fqbn "$(FQBN_BASE)"

board-list:
	@echo ">>> Aangesloten borden:"
	arduino-cli board list
	@echo
	@echo ">>> Alle ESP32 varianten:"
	arduino-cli board listall esp32 | sed -n '1,120p' || true

clean:
	rm -rf "$(BUILD_DIR)"

