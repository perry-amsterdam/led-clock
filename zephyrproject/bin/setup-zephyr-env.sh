#!/usr/bin/env bash
set -e

# === CONFIG ===
ZEPHYR_SDK_VERSION=0.16.8
ZEPHYR_SDK_DIR=/opt/zephyr-sdk
ESP_DIR=$HOME/esp

# === DEFAULT FLAGS ===
MODE="full"
FORCE_SDK="no"

# === USAGE FUNCTION ===
usage() {
  echo "Usage: $0 [OPTIONS]"
  echo ""
  echo "Options:"
  echo "  --minimal     Install only Zephyr (Ubuntu + native build), skip ESP32 toolchain"
  echo "  --clean       Remove Zephyr SDK and ESP-IDF environments"
  echo "  --force-sdk   Force reinstall Zephyr SDK (delete existing first)"
  echo "  -h, --help    Show this help message"
  echo ""
  echo "Examples:"
  echo "  $0             Install full environment (Zephyr + ESP32)"
  echo "  $0 --minimal   Install minimal environment (Zephyr only)"
  echo "  $0 --clean     Remove installed environment"
  echo "  $0 --force-sdk Reinstall Zephyr SDK even if already present"
  exit 0
}

# === PARSE ARGS ===
for arg in "$@"; do
  case "$arg" in
    --minimal)
      MODE="minimal"
      ;;
    --clean)
      MODE="clean"
      ;;
    --force-sdk)
      FORCE_SDK="yes"
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
  echo "🧹 Cleaning Zephyr SDK + ESP32 environment..."
  sudo rm -rf ${ZEPHYR_SDK_DIR}
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
echo "[1/5] Installing system dependencies..."
sudo apt update && sudo apt upgrade -y
sudo apt install --no-install-recommends -y \
  git cmake ninja-build gperf ccache dfu-util device-tree-compiler wget \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel \
  xz-utils file make gcc flex bison gperf python3-serial \
  python3-click python3-cryptography python3-pyparsing python3-pyelftools

# === FIX FOR FUTURE LIB ===
echo "[2/5] Installing Python 'future' package..."
pip install --user --break-system-packages future

# === INSTALL WEST ===
echo "[3/5] Installing west (Zephyr build tool)..."
pip install --user --break-system-packages west

# Ensure ~/.local/bin is in PATH
if ! echo "$PATH" | grep -q "$HOME/.local/bin"; then
  export PATH=$HOME/.local/bin:$PATH
fi

# === INSTALL ZEPHYR SDK ===
echo "[4/5] Installing Zephyr SDK v${ZEPHYR_SDK_VERSION}..."

if [[ "$FORCE_SDK" == "yes" ]]; then
  echo "⚠️  Forcing SDK reinstall..."
  sudo rm -rf ${ZEPHYR_SDK_DIR}
fi

if [ ! -d "${ZEPHYR_SDK_DIR}" ]; then
  wget -q https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64.tar.xz
  tar -xf zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64.tar.xz
  rm zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64.tar.xz
  sudo mv zephyr-sdk-${ZEPHYR_SDK_VERSION} ${ZEPHYR_SDK_DIR}
  ${ZEPHYR_SDK_DIR}/setup.sh -t all -h
else
  echo "⏭️  Zephyr SDK already present at ${ZEPHYR_SDK_DIR}, skipping..."
fi

# === ESP-IDF TOOLCHAIN (ONLY IN FULL MODE) ===
if [[ "$MODE" == "full" ]]; then
  echo "[5/5] Installing ESP-IDF toolchain..."
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
echo "Adding helper functions to ~/.bashrc..."

# Remove old definitions first
sed -i '/zephyr-env()/,/}/d' $HOME/.bashrc || true
sed -i '/esp-env()/,/}/d' $HOME/.bashrc || true

cat << 'EOF' >> $HOME/.bashrc

# === Zephyr/ESP helper functions ===
zephyr-env() {
    local PROJECT_DIR=${1:-$PWD/zephyrproject}
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

echo ""
echo "✅ Setup complete!"
echo "➡️  Open a new terminal or run: source ~/.bashrc"
echo "   Then you can use:"
echo "     zephyr-env [PATH]   # Activate Zephyr environment"
echo "     esp-env             # Activate ESP-IDF toolchain"
echo ""
echo "👉 Note: Zephyr project is NOT created automatically."
echo "   Run 'make init' in your repo to initialize the Zephyr workspace."
