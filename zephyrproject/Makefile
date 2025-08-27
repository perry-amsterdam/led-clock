# Makefile voor Zephyr project setup en build

# Optionele variabelen die je bij 'make' kunt meegeven:
#   make setup-dev VENV_PATH=.venv
#   make setup-dev EXTRA_PIP_ARGS="--user"
#   make setup-dev WEST_SDK_INSTALL=1
VENV_PATH ?=
EXTRA_PIP_ARGS ?=
WEST_SDK_INSTALL ?=0

# Pad naar het setup-script
SETUP_SCRIPT := ./setup-zephyr-env.sh

# Check dat het script er is
$(SETUP_SCRIPT):
	@echo "[ERROR] $(SETUP_SCRIPT) ontbreekt! Zorg dat dit script aanwezig is in de project-root." && exit 1

# Check dat we in een west workspace zitten
check-workspace:
	@if ! west topdir >/dev/null 2>&1; then \
		echo "[ERROR] Je bent niet in een west workspace (geen .west/ map gevonden)."; \
		echo "        Run 'west init' en 'west update' of ga naar je workspace-root."; \
		exit 2; \
	fi

.PHONY: setup-dev
setup-dev: check-workspace $(SETUP_SCRIPT)
	@echo "[INFO] Running Zephyr setup script…"
	VENV_PATH="$(VENV_PATH)" EXTRA_PIP_ARGS="$(EXTRA_PIP_ARGS)" WEST_SDK_INSTALL="$(WEST_SDK_INSTALL)" \
		bash $(SETUP_SCRIPT)

# Voorbeeld build target
.PHONY: build
build: check-workspace
	@echo "[INFO] Building sample app (hello_world voor native_sim)…"
	west build -b native_sim samples/hello_world

.PHONY: flash
flash: check-workspace
	@echo "[INFO] Flashing naar target board…"
	west flash
`:sa
