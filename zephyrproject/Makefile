# Makefile for LED Clock with Zephyr
# Workspace lives in ./zephyrproject
# App lives in ./zephyrproject/app

# Zorg dat 'west' altijd gevonden wordt
export PATH := $(HOME)/.local/bin:$(PATH)

ZEPHYR_DIR := $(CURDIR)/
APP_DIR := $(ZEPHYR_DIR)/app
BOARD ?= native_sim
WEST := west
SETUP_SCRIPT := $(CURDIR)/bin/setup-zephyr-env.sh

.PHONY: help setup-dev setup-dev-minimal init build build-esp32 flash clean

help:
	@echo "Available targets:"
	@echo "  make setup-dev         Install software dependencies (Zephyr SDK, ESP-IDF, etc.)"
	@echo "  make setup-dev-minimal Install only Zephyr (skip ESP32 toolchain)"
	@echo "  make init              Initialize Zephyr workspace in ./zephyrproject"
	@echo "  make build             Build app for $(BOARD) (default: native_sim)"
	@echo "  make build-esp32       Build app for ESP32 DevkitC (WROOM)"
	@echo "  make flash             Flash firmware to ESP32"
	@echo "  make clean             Remove build artifacts"

setup-dev:
	@echo "👉 Running setup script (full)..."
	$(SETUP_SCRIPT)

setup-dev-minimal:
	@echo "👉 Running setup script (minimal)..."
	$(SETUP_SCRIPT) --minimal

init:
	@echo "👉 Initializing Zephyr workspace inside repo..."
	mkdir -p $(ZEPHYR_DIR)
	cd $(ZEPHYR_DIR) && $(WEST) init -l ..
	cd $(ZEPHYR_DIR) && $(WEST) update
	cd $(ZEPHYR_DIR) && $(WEST) zephyr-export
	@echo "✅ Zephyr workspace initialized in $(ZEPHYR_DIR)"

build:
	@echo "👉 Building app for $(BOARD)..."
	cd $(ZEPHYR_DIR) && $(WEST) build -b $(BOARD) app

build-esp32:
	$(MAKE) build BOARD=esp32_devkitc_wroom

flash:
	@echo "👉 Flashing firmware to ESP32..."
	cd $(ZEPHYR_DIR) && $(WEST) flash

clean:
	@echo "🧹 Cleaning build directory..."
	rm -rf $(ZEPHYR_DIR)/build

