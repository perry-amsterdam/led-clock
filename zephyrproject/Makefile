# ============================
# Makefile voor Zephyr project
# ============================

# Optionele variabelen (kun je meegeven bij make):
#   make setup-dev VENV_PATH=.venv
#   make setup-dev EXTRA_PIP_ARGS="--user"
#   make setup-dev WEST_SDK_INSTALL=1
#   make build BOARD=esp32s3_devkitm
#   make flash-esp32 ESP_PORT=/dev/ttyUSB0
VENV_PATH        ?=
EXTRA_PIP_ARGS   ?=
WEST_SDK_INSTALL ?=0

# Build-instellingen
BOARD ?= native_sim
APP   ?= samples/hello_world
BUILD_DIR ?= build

# ESP32 settings (aanpassen naar jouw device/poort)
ESP_PORT ?= /dev/ttyUSB0

# Pad naar het setup-script
SETUP_SCRIPT := ./setup-zephyr-env.sh

# ---------- Helpers ----------
$(SETUP_SCRIPT):
	@echo "[ERROR] $(SETUP_SCRIPT) ontbreekt! Zorg dat dit script in de project-root staat." && exit 1

check-workspace:
	@if ! command -v west >/dev/null 2>&1; then \
		echo "[ERROR] 'west' ontbreekt. Installeer met: pip install --upgrade west"; \
		exit 2; \
	fi
	@if ! west topdir >/dev/null 2>&1; then \
		echo "[ERROR] Je bent niet in een west workspace (geen .west/ gevonden)."; \
		echo "        Ga naar je workspace-root of run 'west init' en 'west update'."; \
		exit 3; \
	fi

# ---------- Setup ----------
.PHONY: setup-dev
setup-dev: check-workspace $(SETUP_SCRIPT) ## Installeer alle dependencies via setup-zephyr-env.sh
	@echo "[INFO] Running Zephyr setup script…"
	VENV_PATH="$(VENV_PATH)" EXTRA_PIP_ARGS="$(EXTRA_PIP_ARGS)" WEST_SDK_INSTALL="$(WEST_SDK_INSTALL)" \
		bash $(SETUP_SCRIPT)

# ---------- Build ----------
.PHONY: build
build: check-workspace ## Build de app (default: samples/hello_world voor native_sim)
	@echo "[INFO] Building $(APP) voor $(BOARD) in $(BUILD_DIR)…"
	west build -b $(BOARD) -d $(BUILD_DIR) $(APP)

# ---------- Flash ----------
.PHONY: flash
flash: check-workspace ## Flash naar het standaard target board
	@echo "[INFO] Flashing naar target board…"
	west flash -d $(BUILD_DIR)

.PHONY: flash-esp32
flash-esp32: check-workspace ## Flash specifiek naar een ESP32 (met ESP_PORT=/dev/ttyUSBx)
	@echo "[INFO] Flashing naar ESP32 via $(ESP_PORT)…"
	west flash -d $(BUILD_DIR) --esp-device $(ESP_PORT)

# ---------- Opschonen ----------
.PHONY: clean
clean: check-workspace ## Wis alleen objecten (snelle schoonmaak)
	@echo "[INFO] Cleaning build artifacts in $(BUILD_DIR)…"
	@if [ -d "$(BUILD_DIR)" ]; then \
		west build -d $(BUILD_DIR) -t clean || true; \
	else \
		echo "[INFO] $(BUILD_DIR) bestaat niet; niets te doen."; \
	fi

.PHONY: pristine
pristine: check-workspace ## Wis build-dir volledig en doe een schone rebuild
	@echo "[INFO] Pristine rebuild voorbereiden (verwijdert CMake cache)…"
	@if [ -d "$(BUILD_DIR)" ]; then rm -rf "$(BUILD_DIR)"; fi
	west build -b $(BOARD) -d $(BUILD_DIR) $(APP) --pristine

.PHONY: distclean
distclean: ## Wis de hele build-map
	@echo "[INFO] Verwijderen van $(BUILD_DIR)/…"
	@if [ -d "$(BUILD_DIR)" ]; then rm -rf "$(BUILD_DIR)"; else echo "[INFO] $(BUILD_DIR) bestaat niet."; fi

# ---------- Monitor ----------
.PHONY: monitor
monitor: check-workspace ## Seriële monitor openen voor ESP32 (via west espressif monitor)
	@echo "[INFO] Starting serial monitor op $(ESP_PORT)…"
	west espressif monitor --esp-device $(ESP_PORT) -d $(BUILD_DIR)

# ---------- Help ----------
.PHONY: help
help: ## Toon dit overzicht van beschikbare commando’s
	@echo "Beschikbare targets in deze Makefile:"
	@grep -E '^[a-zA-Z_-]+:.*?##' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2}'

