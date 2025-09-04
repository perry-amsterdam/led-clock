
# ============================
# Makefile voor Zephyr project (robuste versie)
# ============================
# - Gebruikt Bash als shells
# - .ONESHELL zorgt dat elke recipe in één shell draait
# - Bootstrapt automatisch 'west' in .venv indien nodig
# - Kan workspace auto-initialiseren met AUTO_INIT=1

## Gebruik expliciet /bin/bash als shell. Op sommige systemen is `/usr/bin/env` niet aanwezig, maar
## `/bin/bash` bestaat wel. Dit voorkomt de fout 'No such file or directory' bij `make help`.
SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c
.ONESHELL:

# --------- Config ---------
VENV_PATH        ?= .venv
EXTRA_PIP_ARGS   ?=
WEST_SDK_INSTALL ?= 0
AUTO_INIT        ?= 0
MANIFEST_URL     ?= https://github.com/zephyrproject-rtos/zephyr
MANIFEST_REV     ?= v4.1.0
WORKSPACE_DIR    ?= $(CURDIR)

## Standaard target board. Voor de ESP32‑S3‑WROOM‑1‑N16R8 (16 MB flash, 8 MB PSRAM)
## gebruiken we het Zephyr‑board esp32s3_devkitc (WROOM‑gebaseerd)
BOARD       ?= esp32s3_devkitc
## Standaard applicatie. Pas aan naar je eigen app of sample.
APP         ?= samples/hello_world
BUILD_DIR   ?= build
ESP_PORT    ?= /dev/ttyUSB0

## Snippet configuraties voor flash/PSRAM groottes.  De N16R8 module heeft
## 16 MB SPI flash en 8 MB PSRAM.  Deze waarden worden doorgegeven aan
## west via `-S` vlaggen zodat de juiste variant wordt gebouwd【212531024432480†L1088-L1134】.
FLASH_SNIPPET ?= flash-16M
PSRAM_SNIPPET ?= psram-8M

SETUP_SCRIPT := ./setup-zephyr-env.sh

# --------- Bootstrap helper ---------
.PHONY: __bootstrap
__bootstrap: ## Installeer west in venv (indien nodig) en initialiseer workspace (optioneel)
	# Activeer venv als die bestaat, anders aanmaken als west ontbreekt
	if ! command -v west >/dev/null 2>&1; then
		echo "[INFO] west niet gevonden; installeren in $(VENV_PATH) …"
		python3 -m venv "$(VENV_PATH)"
		source "$(VENV_PATH)/bin/activate"
		python3 -m pip install --upgrade pip
		pip install --upgrade west
	else
		if [[ -f "$(VENV_PATH)/bin/activate" && -z "${VIRTUAL_ENV:-}" ]]; then
			echo "[INFO] Virtualenv activeren: $(VENV_PATH)"
			source "$(VENV_PATH)/bin/activate"
		fi
	fi

	# Als er geen workspace is en AUTO_INIT=1, maak er dan één
	if ! west topdir >/dev/null 2>&1; then
		if [[ "$(AUTO_INIT)" == "1" ]]; then
			echo "[INFO] Geen west workspace gedetecteerd; initialiseren in $(WORKSPACE_DIR)…"
			mkdir -p "$(WORKSPACE_DIR)"
			cd "$(WORKSPACE_DIR)"
			west init -m "$(MANIFEST_URL)" --mr "$(MANIFEST_REV)"
			west update
		else
			echo "[WARN] Geen west workspace gevonden (.west/ ontbreekt)."
			echo "      Tip:  west init -m $(MANIFEST_URL) --mr $(MANIFEST_REV)"
			echo "            west update"
		fi
	fi

# --------- Setup ---------
.PHONY: setup-dev
setup-dev: __bootstrap $(SETUP_SCRIPT) ## Installeer alle dependencies via setup-zephyr-env.sh
	# Activeer venv indien beschikbaar
	if [[ -f "$(VENV_PATH)/bin/activate" && -z "${VIRTUAL_ENV:-}" ]]; then
		source "$(VENV_PATH)/bin/activate"
	fi

	if ! command -v west >/dev/null 2>&1; then
		echo "[ERROR] west is niet beschikbaar na bootstrap."
		exit 2
	fi

	VENV_PATH="$${VIRTUAL_ENV:-$(VENV_PATH)}" EXTRA_PIP_ARGS="$(EXTRA_PIP_ARGS)" WEST_SDK_INSTALL="$(WEST_SDK_INSTALL)" \
		bash $(SETUP_SCRIPT)

# --------- Build ---------
.PHONY: build
build: __bootstrap ## Build de app (default: samples/hello_world voor native_sim)
	if [[ -f "$(VENV_PATH)/bin/activate" && -z "${VIRTUAL_ENV:-}" ]]; then
		source "$(VENV_PATH)/bin/activate"
	fi
	# Voeg flash/PSRAM snippets toe voor juiste geheugenconfiguratie
	west build -b $(BOARD) -S $(FLASH_SNIPPET) -S $(PSRAM_SNIPPET) \
		-d $(BUILD_DIR) $(APP)

# --------- Flash ---------
.PHONY: flash
flash: __bootstrap ## Flash naar het standaard target board
	if [[ -f "$(VENV_PATH)/bin/activate" && -z "${VIRTUAL_ENV:-}" ]]; then
		source "$(VENV_PATH)/bin/activate"
	fi
	west flash -d $(BUILD_DIR)

.PHONY: flash-esp32
flash-esp32: __bootstrap ## Flash specifiek naar een ESP32 (met ESP_PORT=/dev/ttyUSBx)
	if [[ -f "$(VENV_PATH)/bin/activate" && -z "${VIRTUAL_ENV:-}" ]]; then
		source "$(VENV_PATH)/bin/activate"
	fi
	west flash -d $(BUILD_DIR) --esp-device $(ESP_PORT)

# --------- Opschonen ---------
.PHONY: clean
clean: __bootstrap ## Wis alleen objecten (snelle schoonmaak)
	if [[ -f "$(VENV_PATH)/bin/activate" && -z "${VIRTUAL_ENV:-}" ]]; then
		source "$(VENV_PATH)/bin/activate"
	fi
	if [[ -d "$(BUILD_DIR)" ]]; then
		west build -t clean -d $(BUILD_DIR) || true
	else
		echo "[INFO] $(BUILD_DIR) bestaat niet; niets te doen."
	fi

.PHONY: pristine
pristine: __bootstrap ## Wis build-dir volledig en doe een schone rebuild
	if [[ -f "$(VENV_PATH)/bin/activate" && -z "${VIRTUAL_ENV:-}" ]]; then
		source "$(VENV_PATH)/bin/activate"
	fi
	if [[ -d "$(BUILD_DIR)" ]]; then rm -rf "$(BUILD_DIR)"; fi
	# Schone build met geheugen-snippets
	west build -b $(BOARD) -S $(FLASH_SNIPPET) -S $(PSRAM_SNIPPET) \
		-d $(BUILD_DIR) $(APP) --pristine

.PHONY: distclean
distclean: ## Wis de hele build-map
	if [[ -d "$(BUILD_DIR)" ]]; then rm -rf "$(BUILD_DIR)"; else echo "[INFO] $(BUILD_DIR) bestaat niet."; fi

# --------- Monitor ---------
.PHONY: monitor
monitor: __bootstrap ## Seriële monitor openen voor ESP32 (via west espressif monitor)
	if [[ -f "$(VENV_PATH)/bin/activate" && -z "${VIRTUAL_ENV:-}" ]]; then
		source "$(VENV_PATH)/bin/activate"
	fi
	west espressif monitor --esp-device $(ESP_PORT) -d $(BUILD_DIR)

# --------- Help ---------
.PHONY: help
help: ## Toon dit overzicht van beschikbare commando’s
	@echo "Beschikbare targets in deze Makefile:"
	@grep -E '^[a-zA-Z_-]+:.*?##' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2}'
