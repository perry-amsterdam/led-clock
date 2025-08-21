# Makefile for LED Clock Dev Environment
# Alles komt in ./zephyrproject binnen je repo

# Make sure west (pip user install) is in PATH
export PATH := $(HOME)/.local/bin:$(PATH)

ZEPHYR_DIR := $(CURDIR)/zephyrproject
APP_DIR := $(ZEPHYR_DIR)/app
WEST := west

.PHONY: help setup-dev init build flash clean

help:
	@echo "Available targets:"
	@echo "  make setup-dev   Install dependencies + ESP32 toolchain"
	@echo "  make init        Initialize Zephyr workspace in ./zephyrproject"
	@echo "  make build       Build firmware for ESP32"
	@echo "  make flash       Flash firmware to ESP32"
	@echo "  make clean       Remove build artifacts"

setup-dev:
	@echo "👉 Installing development environment..."
	@./bin/setup-zephyr-env.sh --minimal

init:
	@echo "👉 Initializing Zephyr workspace inside repo..."
	mkdir -p $(ZEPHYR_DIR)/app
	cd $(ZEPHYR_DIR) && $(WEST) init -l app
	cd $(ZEPHYR_DIR) && $(WEST) update
	cd $(ZEPHYR_DIR) && $(WEST) zephyr-export
	@echo "✅ Zephyr workspace initialized in $(ZEPHYR_DIR)"

build:
	@echo "👉 Building firmware for ESP32..."
	cd $(ZEPHYR_DIR) && $(WEST) build -b esp32_devkitc_wroom app

flash:
	@echo "👉 Flashing firmware to ESP32..."
	cd $(ZEPHYR_DIR) && $(WEST) flash

clean:
	@echo "🧹 Cleaning build directory..."
	rm -rf $(ZEPHYR_DIR)/build
