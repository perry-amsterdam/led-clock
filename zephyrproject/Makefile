# Makefile for LED Clock with Zephyr
# Workspace lives in ./zephyrproject
# Manifest lives in ./zephyrproject/manifest
# App lives in ./zephyrproject/manifest/app

# Zorg dat 'west' en venv-python altijd gevonden worden
export PATH := $(HOME)/.local/bin:$(PATH)

# Basis paden (zonder trailing slash om spaties te voorkomen)
ZEPHYR_DIR := $(abspath $(CURDIR))
MANIFEST_DIR := $(ZEPHYR_DIR)/manifest
APP_DIR := $(MANIFEST_DIR)/app

BOARD ?= native_sim
WEST := west
SETUP_SCRIPT := $(CURDIR)/bin/setup-zephyr-env.sh

# Build directory per board
BUILD_DIR := $(ZEPHYR_DIR)/build/$(BOARD)

# Virtual environment directory
VENV_DIR := $(ZEPHYR_DIR)/.venv

# Force Zephyr to use Python from venv
export ZEPHYR_PYTHON := $(VENV_DIR)/bin/python

.PHONY: help setup-dev setup-dev-minimal init build build-esp32 flash clean clean-all run boards boards-all venv

help:
	@echo "Available targets:"
	@echo "  make build             Build app for \$$BOARD (default: native_sim)"
	@echo "  make build-esp32       Build app for ESP32 DevkitC (procpu)"
	@echo "  make flash             Flash firmware to ESP32"
	@echo "  make run               Run the app (only works for native_sim)"
	@echo "  make clean             Remove build artifacts for current BOARD"
	@echo "  make clean-all         Remove all build artifacts"
	@echo "  make boards            List available ESP32 boards"
	@echo "  make boards-all        List all available boards"
	@echo "  make venv              Show how to activate Python virtual environment"
	@echo "  make setup-dev         Install Zephyr SDK + ESP32 dependencies"
	@echo "  make setup-dev-minimal Install only Zephyr SDK (skip ESP32)"
	@echo "  make init              Initialize Zephyr workspace in ./zephyrproject"

setup-dev:
	@echo "👉 Running setup script (full)..."
	$(SETUP_SCRIPT)

setup-dev-minimal:
	@echo "👉 Running setup script (minimal)..."
	$(SETUP_SCRIPT) --minimal

init:
	@echo "👉 Initializing Zephyr workspace inside $(ZEPHYR_DIR)..."
	mkdir -p $(ZEPHYR_DIR)
	cd $(ZEPHYR_DIR) && $(WEST) init -l manifest
	cd $(ZEPHYR_DIR) && $(WEST) update
	cd $(ZEPHYR_DIR) && $(WEST) zephyr-export
	@echo "✅ Zephyr workspace initialized in $(ZEPHYR_DIR)"

build:
	@echo "👉 Building app for $(BOARD) in $(BUILD_DIR)..."
	cd $(ZEPHYR_DIR) && $(WEST) build -b $(BOARD) $(APP_DIR) -d $(BUILD_DIR)

build-esp32:
	$(MAKE) build BOARD=esp32_devkitc/esp32/procpu

flash:
	@echo "👉 Flashing firmware to $(BOARD)..."
	cd $(BUILD_DIR) && $(WEST) flash

run:
ifeq ($(BOARD),native_sim)
	@echo "🚀 Running native_sim executable..."
	$(BUILD_DIR)/zephyr/zephyr.elf
else
	@echo "❌ 'make run' only works with BOARD=native_sim"
	@exit 1
endif

boards:
	@echo "👉 Available ESP32 boards in Zephyr:"
	cd $(ZEPHYR_DIR) && $(WEST) boards | grep esp32 || true

boards-all:
	@echo "👉 All available boards in Zephyr:"
	cd $(ZEPHYR_DIR) && $(WEST) boards || true

venv:
	@echo "👉 Python virtual environment is at $(VENV_DIR)"
	@echo ""
	@echo "To activate manually, run:"
	@echo "    source $(VENV_DIR)/bin/activate"
	@echo ""

clean:
	@echo "🧹 Cleaning build directory for $(BOARD)..."
	rm -rf $(BUILD_DIR)

clean-all:
	@echo "🧹 Cleaning ALL build directories..."
	rm -rf $(ZEPHYR_DIR)/build
