# Makefile — Zephyr (ESP32-S3) clean build flow
# -------------------------------------------------------------
# Shell settings (bash with strict mode for recipes)
SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c
.ONESHELL:

# ---- Configurable variables ---------------------------------
# Workspace
WORKSPACE_DIR ?= $(CURDIR)

# App and board
APP          ?= manifest/app
BOARD        ?= esp32s3_devkitc/esp32s3/procpu

# Build output
BUILD_DIR    ?= build

# Optional: extra cmake args (e.g. -DDTC_OVERLAY_FILE=boards/esp32s3_devkitc.overlay)
EXTRA_CMAKE_ARGS ?=

# Tooling
WEST         ?= west

# Local helper scripts
SETUP_SCRIPT := ./setup-zephyr-env.sh
FETCH_BLOBS  := ./fetch-blobs.sh

# Serial/flash options (customize via environment/CLI: make flash MONITOR_ARGS="--device /dev/ttyUSB0")
FLASH_ARGS   ?=
MONITOR_ARGS ?=

# If user sets DTC_OVERLAY_FILE=..., propagate to CMake automatically
ifdef DTC_OVERLAY_FILE
EXTRA_CMAKE_ARGS += -DDTC_OVERLAY_FILE=$(DTC_OVERLAY_FILE)
endif

# Small helper to ensure we run in the prepared Zephyr env
define WEST_ENV
source "$(SETUP_SCRIPT)"
endef

# ---- Phonies -------------------------------------------------
.PHONY: help setup-dev fetch-blobs build pristine clean flash monitor erase env

help:
\t@echo "Targets:"
\t@echo "  make setup-dev        # Run local environment bootstrap (scripts in repo)"
\t@echo "  make fetch-blobs      # Fetch firmware blobs/dependencies (optional)"
\t@echo "  make build            # Configure & build $(APP) for $(BOARD) into $(BUILD_DIR)"
\t@echo "  make pristine         # Remove build dir and build from scratch"
\t@echo "  make clean            # Remove build dir only"
\t@echo "  make flash            # Flash using west (pass extra via FLASH_ARGS=...)"
\t@echo "  make monitor          # Open serial monitor (pass extra via MONITOR_ARGS=...)"
\t@echo "  make erase            # Full flash erase (espressif)"
\t@echo "Variables:"
\t@echo "  APP=$(APP)"
\t@echo "  BOARD=$(BOARD)"
\t@echo "  BUILD_DIR=$(BUILD_DIR)"
\t@echo "  EXTRA_CMAKE_ARGS=$(EXTRA_CMAKE_ARGS)"

# Ensure scripts are executable if present
setup-dev:
\t@[ -f "$(SETUP_SCRIPT)" ] || { echo "ERROR: $(SETUP_SCRIPT) not found"; exit 1; }
\t@chmod +x "$(SETUP_SCRIPT)"
\t@echo "[setup-dev] Running $(SETUP_SCRIPT) ..."
\t@$(SETUP_SCRIPT)
\t@$(MAKE) fetch-blobs || true

fetch-blobs:
\t@if [ -f "$(FETCH_BLOBS)" ]; then \\\n\t  chmod +x "$(FETCH_BLOBS)"; \\\n\t  echo "[fetch-blobs] Running $(FETCH_BLOBS) ..."; \\\n\t  $(FETCH_BLOBS); \\\n\telse \\\n\t  echo "[fetch-blobs] No $(FETCH_BLOBS) found — skipping"; \\\n\tfi

# Configure and build
build:
\t@echo "[build] APP=$(APP) BOARD=$(BOARD) BUILD_DIR=$(BUILD_DIR)"
\t@$(WEST_ENV); \\\n\t$(WEST) build -b "$(BOARD)" -d "$(BUILD_DIR)" "$(APP)" -- $(EXTRA_CMAKE_ARGS)

# Fresh build from scratch
pristine:
\t@echo "[pristine] Removing $(BUILD_DIR) and rebuilding"
\t@rm -rf "$(BUILD_DIR)"
\t@$(MAKE) build

# Remove build directory
clean:
\t@echo "[clean] Removing $(BUILD_DIR)"
\t@rm -rf "$(BUILD_DIR)"

# Flash using west (espressif runner inferred from board)
flash:
\t@echo "[flash] Using west flash (args: $(FLASH_ARGS))"
\t@$(WEST_ENV); \\\n\t$(WEST) flash -d "$(BUILD_DIR)" $(FLASH_ARGS)

# Serial monitor (west espressif plugin)
monitor:
\t@echo "[monitor] Opening serial monitor (args: $(MONITOR_ARGS))"
\t@$(WEST_ENV); \\\n\t$(WEST) espressif monitor -d "$(BUILD_DIR)" $(MONITOR_ARGS)

# Full erase (use with care)
erase:
\t@echo "[erase] Full flash erase"
\t@$(WEST_ENV); \\\n\t$(WEST) espressif erase_flash -d "$(BUILD_DIR)"

# Print the environment west sees (debug helper)
env:
\t@$(WEST_ENV); env | sort
