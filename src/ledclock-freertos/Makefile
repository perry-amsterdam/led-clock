# ====== User-configurable ======
SKETCH        ?= .
PORT          ?= /dev/ttyACM0
UPLOAD_SPEED  ?= 921600
MONITOR_BAUD  ?= 115200
BUILD_DIR     ?= build
LIBS_FILE     ?= libs.txt
ARDUINO_CLI = arduino-cli
SKETCH_DIR ?= $(CURDIR)
LIBRARIES_DIR ?= $(CURDIR)/libraries

# Arduino core ID and base board
CORE_ID       ?= esp32:esp32
BOARD_BASE    ?= esp32s3

# ====== Board menu options (confirmed working on your setup) ======
# You can override FLASH_OPTS at make-time if needed.
FLASH_OPTS    ?= FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB,UploadSpeed=$(UPLOAD_SPEED)

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
	$(ARDUINO_CLI) compile --libraries "$(LIBRARIES_DIR)" --fqbn "$(FQBN)" --build-path "$(BUILD_DIR)" "$(SKETCH)"

# ----- Upload/Flash (reuses artifacts from BUILD_DIR) -----
flash: build ## Upload (flash) prebuilt artifacts from $(BUILD_DIR)
	$(ARDUINO_CLI) upload -p $(PORT) -p "$(PORT)" --fqbn "$(FQBN)" --input-dir "$(BUILD_DIR)"

upload: flash ## Alias for flash
	@true

# ----- Serial Monitor -----
monitor: ## Open serial monitor
	arduino-cli monitor -p "$(PORT)" -c baudrate=$(MONITOR_BAUD)

# ----- Maintenance -----
erase: ## Erase full flash (⚠ wipes everything)
	esptool --port "$(PORT)" erase-flash

erase-fs: ## Erase NVS/filesystem region (adjust for your partition scheme if needed)
	esptool --port "$(PORT)" erase_region 0x9000 0x6000

clean: ## Remove build directory
	rm -rf "$(BUILD_DIR)"

format: ## Format all .cpp, .h and .ino files with bcpp
	for file in $(shell ls *.cpp *.h *.ino 2>/dev/null); do \
		bcpp -t $$file > $$file.new; \
		mv -f -v $$file.new $$file; \
	done

# ----- Help -----
# =========================================================
# 🧹 Linting & Code Quality
# =========================================================

SRC_DIRS := src include
CLANG_TIDY_FLAGS := -- -std=c++17 -Iinclude

.PHONY: lint lint-cppcheck lint-clang install-linters

lint: lint-cppcheck lint-clang ## Run both cppcheck and clang-tidy
	@echo "✅ Linting complete."

lint-cppcheck: ## Run cppcheck on source files
	@echo "🔍 Running cppcheck..."
	@cppcheck --enable=all --inconclusive --std=c++17 --quiet $(SRC_DIRS) || true

lint-clang: ## Run clang-tidy on source files
	@echo "🔍 Running clang-tidy..."
	@find $(SRC_DIRS) -name '*.cpp' | xargs -r clang-tidy $(CLANG_TIDY_FLAGS) || true

install-linters: ## Install cppcheck and clang-tidy via apt
	@echo "📦 Installing cppcheck and clang-tidy..."
	sudo apt update && sudo apt install -y cppcheck clang-tidy

help: ## Show this help
	@echo "Available targets (vars: SKETCH, PORT, UPLOAD_SPEED, MONITOR_BAUD, BUILD_DIR, FLASH_OPTS, LIBS_FILE):"
	@grep -E '^[a-zA-Z0-9_-]+:.*?## ' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  %-14s %s\n", $$1, $$2}'


# ====== Extra tools ======

.PHONY: lint cppcheck check
check: lint cppcheck


.PHONY: verbose
verbose:
	$(ARDUINO_CLI) compile -v --libraries "$(LIBRARIES_DIR)" --build-path "$(BUILD_DIR)" "$(SKETCH_DIR)"
