#!/usr/bin/env bash
set -e

# === CONFIG ===
ZEPHYR_SDK_VERSION=0.16.8
ZEPHYR_DIR=$HOME/zephyrproject
ZEPHYR_SDK_DIR=/opt/zephyr-sdk
ESP_DIR=$HOME/esp

# === USAGE FUNCTION ===
usage() {
  echo "Usage: $0 [OPTIONS]"
  echo ""
  echo "Options:"
  echo "  --minimal     Install only Zephyr (Ubuntu + native build), skip ESP32 toolchain"
  echo "  --clean       Remove Zephyr and ESP-IDF environments"
  echo "  -h, --help    Show this help message"
  echo ""
  echo "Installed helper functions (added to ~/.bashrc):"
  echo "  zephyr-env [PATH]   Activate Zephyr environment."
  echo "                      - Without argument: goes to ~/zephyrproject"
  echo "                      - With argument:   goes to the given project path"
  echo ""
  echo "  esp-env             Activate ESP-IDF environment (ESP32 toolchain)."
  echo ""
  echo "Examples:"
  echo "  $0            Install full environment (Zephyr + ESP32)"
  echo "  $0 --minimal  Install minimal environment (Zephyr only)"
  echo "  $0 --clean    Remove installed environment"
  echo ""
  echo "After installation:"
  echo "  zephyr-env                  # Jump into ~/zephyrproject with env ready"
  echo "  zephyr-env ~/my-zephyr-app  # Jump into custom project"
  echo "  esp-env                     # Activate ESP-IDF toolchain"
  exit 0
}

# === PARSE ARGS ===
MODE="full"
for arg in "$@"; do
  case "$arg" in
    --minimal)
      MODE="minimal"
      ;;
    --clean)
      MODE="clean"
      ;;
    -h|--help)
      usage
      ;;
    *)
      echo "❌ Unknown option: $arg"
      usage
      ;;
  esac
done

# === CLEAN MODE ===
if [[ "$MODE" == "clean" ]]; then
  echo "🧹 Cleaning Zephyr + ESP32 environment..."
  sudo rm -rf ${ZEPHYR_SDK_DIR}
  rm -rf ${ZEPHYR_DIR}
  rm -rf ${ESP_DIR}
  sed -i '/esp-idf\/export.sh/d' $HOME/.bashrc || true
  sed -i '/zephyr-env()/,/}/d' $HOME/.bashrc || true
  sed -i '/esp-env()/,/}/d' $HOME/.bashrc || true
  echo "✅ Environment removed."
  exit 0
fi

if [[ "$MODE" == "minimal" ]]; then
  echo "⚡ Running in MINIMAL mode (Ubuntu + Zephyr only, no ESP32)"
fi

# === SYSTEM DEPENDENCIES ===
echo "[1/7] Installing system dependencies..."
sudo apt update && sudo apt upgrade -y
sudo apt install --no-install-recommends -y \
  git cmake ninja-build gperf ccache dfu-util device-tree-compiler wget \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel \
  xz-utils file make gcc flex bison gperf python3-serial \
  python3-click python3-cryptography python3-pyparsing python3-pyelftools

# === FIX FOR FUTURE LIB ===
echo "[2/7] Installing Python 'future' package..."
pip install --user --break-system-packages future

# === INSTALL WEST ===
echo "[3/7] Installing west (Zephyr build tool)..."
pip install --user --break-system-packages west

# === INSTALL ZEPHYR SDK ===
echo "[4/7] Installing Zephyr SDK v${ZEPHYR_SDK_VERSION}..."
wget -q https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64.tar.xz
tar -xf zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64.tar.xz
rm zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64.tar.xz
sudo mv zephyr-sdk-${ZEPHYR_SDK_VERSION} ${ZEPHYR_SDK_DIR}
${ZEPHYR_SDK_DIR}/setup.sh -t all -h

# === INIT ZEPHYR PROJECT ===
echo "[5/7] Initializing Zephyr project..."
mkdir -p ${ZEPHYR_DIR}
cd ${ZEPHYR_DIR}
west init -m https://github.com/zephyrproject-rtos/zephyr.git
west update
west zephyr-export
pip install --break-system-packages -r zephyr/scripts/requirements.txt

# === ESP-IDF TOOLCHAIN (ONLY IN FULL MODE) ===
if [[ "$MODE" == "full" ]]; then
  echo "[6/7] Installing ESP-IDF toolchain..."
  mkdir -p ${ESP_DIR}
  cd ${ESP_DIR}
  if [ ! -d "esp-idf" ]; then
    git clone --recursive https://github.com/espressif/esp-idf.git
  fi
  cd esp-idf
  ./install.sh

  # Add ESP-IDF export to bashrc if not present
  if ! grep -q "esp-idf/export.sh" $HOME/.bashrc; then
    echo ". ${ESP_DIR}/esp-idf/export.sh" >> $HOME/.bashrc
  fi
else
  echo "⏭️  Skipping ESP-IDF installation (minimal mode)"
fi

# === ADD FUNCTIONS TO BASHRC ===
echo "[7/7] Adding helper functions to ~/.bashrc..."

# Remove old definitions first
sed -i '/zephyr-env()/,/}/d' $HOME/.bashrc || true
sed -i '/esp-env()/,/}/d' $HOME/.bashrc || true

cat << 'EOF' >> $HOME/.bashrc

# === Zephyr/ESP helper functions ===
zephyr-env() {
    local PROJECT_DIR=${1:-$HOME/zephyrproject}
    if [ -d "$PROJECT_DIR" ]; then
        cd "$PROJECT_DIR" || return
        west zephyr-export > /dev/null 2>&1
        echo "✅ Zephyr environment ready in: $PROJECT_DIR"
    else
        echo "❌ Directory not found: $PROJECT_DIR"
    fi
}

esp-env() {
    if [ -f "$HOME/esp/esp-idf/export.sh" ]; then
        . "$HOME/esp/esp-idf/export.sh"
        echo "✅ ESP-IDF environment ready"
    else
        echo "❌ ESP-IDF not installed in $HOME/esp/esp-idf"
    fi
}
EOF

echo "✅ Functions 'zephyr-env' and 'esp-env' added to ~/.bashrc"

# === TEST BUILD ===
echo "🔧 Testing native build..."
cd ${ZEPHYR_DIR}/zephyr/samples/hello_world
west build -b native_sim

echo ""
echo "✅ Zephyr environment setup complete!"
if [[ "$MODE" == "minimal" ]]; then
  echo "👉 Minimal mode: Only Zephyr on Ubuntu installed"
else
  echo "👉 Full mode: Zephyr + ESP32 toolchain installed"
fi

echo ""
echo "➡️  Open a new terminal or run: source ~/.bashrc"
echo "   Then you can use:"
echo "     zephyr-env                  # Default project"
echo "     zephyr-env ~/my-zephyr-app  # Custom project"
echo "     esp-env                     # Activate ESP-IDF toolchain"

