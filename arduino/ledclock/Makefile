ESPTOOL := $(shell which esptool 2>/dev/null)
ESPTOOL_CMD := $(if $(ESPTOOL),$(ESPTOOL),$(PY) -m esptool)

BUILD_DIR ?= build
PY ?= python3
PORT ?= /dev/ttyACM0
CHIP ?= esp32s3

help:
	@echo "Beschikbare targets:"
	@echo "  make build         - Compileer de sketch met arduino-cli"
	@echo "  make upload        - Upload de sketch (PORT=$(PORT))"
	@echo "  make upload-clean  - Wis hele flash, bouw en upload opnieuw"
	@echo "  make erase-all     - Wis de volledige flash"
	@echo "  make erase-all-no-stub - Wis flash zonder stub (fallback)"
	@echo "  make monitor       - Seriële monitor openen"
	@echo ""
	@echo "Dependencies installeren:"
	@echo "  sudo apt install -y pipx"
	@echo "  pipx ensurepath"
	@echo "  pipx install esptool"
	@echo "  curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh"

build:
	arduino-cli compile \		--fqbn "esp32:esp32:esp32s3:FlashSize=16M,PSRAM=enabled,PartitionScheme=app3M_fat9M_16MB" \		--build-path "$(BUILD_DIR)" \		.

upload:
	arduino-cli upload -p $(PORT) --fqbn esp32:esp32:esp32s3 .

upload-clean: erase-all build upload

erase-all:
	$(ESPTOOL_CMD) --chip $(CHIP) --port $(PORT) erase_flash

erase-all-no-stub:
	$(ESPTOOL_CMD) --chip $(CHIP) --port $(PORT) --no-stub erase_flash

monitor:
	arduino-cli monitor -p $(PORT) --config baudrate=115200

erase-fs:
	$(ESPTOOL_CMD) --chip $(CHIP) --port $(PORT) erase_region 0x290000 0x960000
