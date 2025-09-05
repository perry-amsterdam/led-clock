# Makefile — Zephyr (ESP32-S3) clean build flow
# -------------------------------------------------------------
SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c
.ONESHELL:

# ---- Configurable variables ---------------------------------
WORKSPACE_DIR ?= $(CURDIR)
APP          ?= manifest/app
BOARD        ?= esp32s3_devkitc/esp32s3/procpu
BUILD_DIR    ?= $(WORKSPACE_DIR)/build

WEST         ?= west
CMAKE        ?= cmake
NINJA        ?= ninja

CMAKE_ARGS   ?=
BUILD_ARGS   ?= -v
FLASH_ARGS   ?=
MONITOR_ARGS ?=

WEST_ENV     ?= :

ifdef DTC_OVERLAY_FILE
export DTC_OVERLAY_FILE := $(DTC_OVERLAY_FILE)
CMAKE_ARGS += -DDTC_OVERLAY_FILE=$(DTC_OVERLAY_FILE)
endif

.PHONY: help configure build rebuild run flash monitor erase env pristine clean dirs default

default: build

help:
	@cat <<'EOF'
	Targets:
	build        Configure (if needed) and build the app for $(BOARD)
	rebuild      Clean build directory and rebuild from scratch
	run          Build then run (if supported by board)
	flash        Flash the built firmware to the device
	monitor      Open serial monitor (requires espressif west plugin)
	erase        Full chip erase (DANGEROUS)
	env          Print environment (as seen by west)
	pristine     Remove the CMake build tree ("pristine" build)
	clean        Remove the entire build directory

Variables (override with VAR=value):
	APP=$(APP)
	BOARD=$(BOARD)
	BUILD_DIR=$(BUILD_DIR)
	CMAKE_ARGS=$(CMAKE_ARGS)
	BUILD_ARGS=$(BUILD_ARGS)
	FLASH_ARGS=$(FLASH_ARGS)
	MONITOR_ARGS=$(MONITOR_ARGS)
	EOF

dirs:
	@mkdir -p "$(BUILD_DIR)"

configure: dirs
	@echo "[cmake] Configuring $(APP) for $(BOARD) into $(BUILD_DIR)"
	@$(WEST_ENV); \
	$(CMAKE) -S "$(APP)" -B "$(BUILD_DIR)" -G Ninja -DBOARD="$(BOARD)" $(CMAKE_ARGS)

build: | configure
	@echo "[build] Building in $(BUILD_DIR)"
	@$(WEST_ENV); \
	$(NINJA) -C "$(BUILD_DIR)" $(BUILD_ARGS)

rebuild: clean build

run: build
	@echo "[run] west build + run (if supported by board/toolchain)"
	@$(WEST_ENV); \
	$(WEST) run -d "$(BUILD_DIR)" || { \
	  echo "[run] 'west run' not supported; trying 'west flash'"; \
	  $(WEST) flash -d "$(BUILD_DIR)" $(FLASH_ARGS); \
	}

flash: build
	@echo "[flash] Flashing"
	@$(WEST_ENV); \
	$(WEST) flash -d "$(BUILD_DIR)" $(FLASH_ARGS)

monitor:
	@echo "[monitor] Opening serial monitor (args: $(MONITOR_ARGS))"
	@$(WEST_ENV); \
	$(WEST) espressif monitor -d "$(BUILD_DIR)" $(MONITOR_ARGS)

erase:
	@echo "[erase] Full flash erase"
	@$(WEST_ENV); \
	$(WEST) espressif erase_flash -d "$(BUILD_DIR)"

env:
	@$(WEST_ENV); env | sort

pristine:
	@echo "[pristine] Removing CMake cache and files under $(BUILD_DIR)"
	@rm -rf "$(BUILD_DIR)/CMakeCache.txt" \
	        "$(BUILD_DIR)/CMakeFiles" \
	        "$(BUILD_DIR)/build.ninja" \
	        "$(BUILD_DIR)/cmake_install.cmake" \
	        "$(BUILD_DIR)/zephyr" \
	        "$(BUILD_DIR)/modules" \
	        "$(BUILD_DIR)/.ninja_*"

clean:
	@echo "[clean] Removing $(BUILD_DIR)"
	@rm -rf "$(BUILD_DIR)"
