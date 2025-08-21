#!/usr/bin/env bash
set -e

# === CONFIG ===
ZEPHYR_SDK_VERSION=0.16.8
ZEPHYR_SDK_DIR=/opt/zephyr-sdk
ESP_DIR=$HOME/esp
PROJECT_DIR=$HOME/led-clock/zephyrproject
VENV_DIR=$PROJECT_DIR/.venv   # virtuele env in project

# === DEFAULT FLAGS ===
MODE="full"
FORCE_SDK="no"

# === USAGE FUNCTION ===
usage() {
  echo "Usage: $0 [OPTIONS]"
  echo ""
  echo "Options:"
  echo "  --minimal     Install only Zephyr SDK + native build support, skip ESP32 toolchain"
  echo "  --clean       Remove Zephyr SDK, ESP-IDF, and Python virtual environment"
  echo "  --force-sdk   Force reinstall Zephyr SDK (delete existing first)"
  echo "  -h, --help    Show this help message"
  echo ""
  echo "Examples:"
  echo "  $0             Install full environment (Zephyr + ESP32 support)"
  echo "  $0 --minimal   Install minimal environment (Zephyr only)"
  echo "  $0 --clean     Remove installed environment"
  echo "  $0 --force-sdk Reinstall Zephyr SDK even if already present"
  exit 0
}

# === PARSE ARGS ===
for arg in "$@"; do
  case "$arg" in
    --minimal) MODE="minimal" ;;
    --clean) MODE="clean" ;;
    --force-sdk) FORCE_SDK="yes" ;;
    -h|--help) usage ;;
    *) echo "Unknown option: $arg"; usage ;;
  esac
done

# === CLEAN MODE ===
if [ "$MODE" = "clean" ]; then
  echo "🧹 Removing Zephyr SDK, ESP-IDF and virtual environment..."
  sudo rm -rf "$ZEPHYR_SDK_DIR"
  rm -rf "$ESP_DIR"
  rm -rf "$VENV_DIR"
  echo "✅ Clean complete"
  exit 0
fi

# === INSTALL SYSTEM DEPENDENCIES ===
echo "👉 Installing system dependencies..."
sudo apt-get update
sudo apt-get install -y git cmake ninja-build gperf ccache dfu-util \
  device-tree-compiler wget python3-dev python3-pip python3-venv python3-setuptools \
  python3-tk python3-wheel xz-utils file make gcc gcc-multilib g++-multilib libsdl2-dev

# === INSTALL ZEPHYR SDK ===
if [ "$FORCE_SDK" = "yes" ]; then
  echo "👉 Forcing reinstall of Zephyr SDK..."
  sudo rm -rf "$ZEPHYR_SDK_DIR"
fi

if [ ! -d "$ZEPHYR_SDK_DIR" ]; then
  echo "👉 Installing Zephyr SDK $ZEPHYR_SDK_VERSION..."
  wget -q https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v$ZEPHYR_SDK_VERSION/zephyr-sdk-$ZEPHYR_SDK_VERSION-linux-x86_64-setup.run -O /tmp/zephyr-sdk.run
  chmod +x /tmp/zephyr-sdk.run
  sudo /tmp/zephyr-sdk.run -- -d "$ZEPHYR_SDK_DIR"
  rm /tmp/zephyr-sdk.run
else
  echo "✅ Zephyr SDK already installed at $ZEPHYR_SDK_DIR"
fi

# === PYTHON VIRTUAL ENVIRONMENT ===
if [ ! -d "$VENV_DIR" ]; then
  echo "👉 Creating Python virtual environment in $VENV_DIR"
  python3 -m venv "$VENV_DIR"
fi

echo "👉 Activating Python virtual environment..."
# shellcheck source=/dev/null
source "$VENV_DIR/bin/activate"

# Set Zephyr to use this venv's python
export ZEPHYR_PYTHON="$VENV_DIR/bin/python"
echo "✅ ZEPHYR_PYTHON set to $ZEPHYR_PYTHON"

# Upgrade pip inside venv
pip install --upgrade pip

# === INSTALL ESP32 TOOLCHAIN & DEPENDENCIES (ONLY IF NOT MINIMAL) ===
if [ "$MODE" = "full" ]; then
  echo "👉 Installing ESP32 Python dependencies in venv..."
  pip install "esptool>=5.0.2"
  $WEST packages pip --install
  echo "✅ esptool installed: $(esptool.py --version)"
fi

echo "🎉 Setup complete!"
echo "👉 To activate the environment later, run: source $VENV_DIR/bin/activate"

