# ====== User-configurable ======
# Sketch folder (directory containing your .ino)
SKETCH        ?= .
# Serial port for the board
PORT          ?= /dev/ttyACM0
# Upload speed
UPLOAD_SPEED  ?= 921600
# Serial monitor baudrate
MONITOR_BAUD  ?= 115200
# Build directory
BUILD_DIR     ?= build
# Optional libraries manifest (one library name per line)
LIBS_FILE     ?= libs.txt

# Arduino core ID and base board
CORE_ID       ?= esp32:esp32
BOARD_BASE    ?= esp32s3

# ====== Board menu options (confirmed working on your setup) ======
# You can override FLASH_OPTS at make-time if needed.
FLASH_OPTS    ?= FlashMode=opi,FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB

# Fully qualified board name
FQBN          ?= $(CORE_ID):$(BOARD_BASE):$(FLASH_OPTS)
# Base FQBN without menu options (for board-details etc.)
FQBN_BASE     ?= $(CORE_ID):$(BOARD_BASE)

# ====== Phony targets ======
.PHONY: all deps board-details build flash upload monitor erase erase-fs clean format help

# Default target
all: build

# ----- Dependencies (core + optional libs) -----
deps: ## Install ESP32 core and libraries from $(LIBS_FILE) if present
	arduino-cli core update-index
	arduino-cli core install $(CORE_ID)
	@if [ -f "$(LIBS_FILE)" ]; then \
		echo "Installing libraries from $(LIBS_FILE)..."; \
		arduino-cli lib install $$(tr '\n' ' ' < "$(LIBS_FILE)"); \
	else \
		echo "No $(LIBS_FILE) found. Skipping library install."; \
	fi

# ----- Inspect available board options for your installed core -----
board-details: ## Show board menu options for $(FQBN_BASE)
	arduino-cli board details -b $(FQBN_BASE)

# ----- Build -----
build: ## Compile the sketch into $(BUILD_DIR)
	@mkdir -p "$(BUILD_DIR)"
	@echo "Building for FQBN: $(FQBN)"
	arduino-cli compile --fqbn "$(FQBN)" --build-path "$(BUILD_DIR)" "$(SKETCH)"

# ----- Upload/Flash (reuses artifacts from BUILD_DIR) -----
flash: build ## Upload (flash) prebuilt artifacts from $(BUILD_DIR)
	arduino-cli upload -p "$(PORT)" --fqbn "$(FQBN)" --input-dir "$(BUILD_DIR)"

upload: flash ## Alias for flash
	@true

# ----- Serial Monitor -----
monitor: ## Open serial monitor
	arduino-cli monitor -p "$(PORT)" -c baudrate=$(MONITOR_BAUD)

# ----- Maintenance -----
erase: ## Erase full flash (⚠ wipes everything)
	esptool.py --port "$(PORT)" erase_flash

erase-fs: ## Erase NVS/filesystem region (adjust for your partition scheme if needed)
	esptool.py --port "$(PORT)" erase_region 0x9000 0x6000

clean: ## Remove build directory
	rm -rf "$(BUILD_DIR)"

format: ## Run clang-format on all C/C++ headers/sources
	@command -v clang-format >/dev/null 2>&1 && \
		find . -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) -print0 | xargs -0 clang-format -i || \
		echo "clang-format not found, skipping."

# ----- Help -----
help: ## Show this help
	@echo "Available targets (vars: SKETCH, PORT, UPLOAD_SPEED, MONITOR_BAUD, BUILD_DIR, FLASH_OPTS, LIBS_FILE):"
	@grep -E '^[a-zA-Z0-9_-]+:.*?## ' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  %-14s %s\n", $$1, $$2}'

