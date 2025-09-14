# ===== 10Telecom ESP32-S3 Makefile (cleaned) =====
# Tooling
ARDUINO_CLI ?= arduino-cli
PY ?= python3

# Ports & chip
PORT ?= /dev/ttyACM0
CHIP ?= esp32s3

# Build options
BOARD ?= esp32:esp32:esp32s3
FLASH_OPTS ?= FlashSize=16M,PSRAM=enabled,PartitionScheme=app3M_fat9M_16MB,UploadSpeed=921600
FQBN := $(BOARD):$(FLASH_OPTS)

SKETCH ?= .
BUILD_DIR ?= build
UPLOAD_SPEED ?= 921600
MONITOR_BAUD ?= 115200

# esptool detection (uses system esptool if present, otherwise python module)
ESPTOOL := $(shell which esptool 2>/dev/null)
ESPTOOL_CMD := $(if $(ESPTOOL),$(ESPTOOL),$(PY) -m esptool)

# (Optional) Filesystem region for "app3M_fat9M_16MB" partition scheme
# Adjust if you change the partition scheme!
FS_START ?= 0x290000
FS_SIZE  ?= 0x960000

.PHONY: help build upload flash upload-clean erase-all erase-all-no-stub erase-fs monitor clean core-version fqbn

help:
	@echo "Beschikbare targets:"
	@echo "  make build          - Compileer de sketch met arduino-cli"
	@echo "  make upload         - Upload de sketch (PORT=$(PORT))"
	@echo "  make flash          - Alias: build + upload"
	@echo "  make upload-clean   - Wis hele flash, bouw en upload opnieuw"
	@echo "  make erase-all      - Wis de volledige flash"
	@echo "  make erase-all-no-stub - Wis flash zonder stub (fallback)"
	@echo "  make erase-fs       - Wis alléén de FATFS partitie (LET OP: data kwijt)"
	@echo "  make monitor        - Open seriële monitor ($(MONITOR_BAUD) baud)"
	@echo "  make clean          - Verwijder de build directory"
	@echo "  make fqbn           - Toon de volledige FQBN"

fqbn:
	@echo "$(FQBN)"

core-version:
	$(ARDUINO_CLI) version && $(ARDUINO_CLI) core list

build:
	$(ARDUINO_CLI) compile \
		--fqbn "$(FQBN)" \
		--build-path "$(BUILD_DIR)" \
		--build-property compiler.cpp.extra_flags="-DDEBUG_TZ" \
		--build-property compiler.c.extra_flags="-DDEBUG_TZ" \
		"$(SKETCH)"

upload: build
	$(ARDUINO_CLI) upload \
		--fqbn "$(FQBN)" \
		--port "$(PORT)" \
		--input-dir "$(BUILD_DIR)" \
		--verify \

flash: upload

upload-clean: erase-all build upload

erase-all:
	$(ESPTOOL_CMD) --chip $(CHIP) --port "$(PORT)" erase_flash

erase-all-no-stub:
	$(ESPTOOL_CMD) --chip $(CHIP) --port "$(PORT)" --no-stub erase_flash

erase-fs:
	@echo "⚠️  Wissen van filesystem regio: start=$(FS_START), size=$(FS_SIZE)"
	$(ESPTOOL_CMD) --chip $(CHIP) --port "$(PORT)" erase_region $(FS_START) $(FS_SIZE)

monitor:
	$(ARDUINO_CLI) monitor -p "$(PORT)" --config baudrate=$(MONITOR_BAUD)

clean:
	rm -rf "$(BUILD_DIR)"
