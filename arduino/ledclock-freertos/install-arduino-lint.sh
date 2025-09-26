#!/bin/bash
set -euo pipefail

echo "🔽 Arduino Lint installeren..."

TMPDIR="$(mktemp -d)"
TARBALL="$TMPDIR/arduino-lint.tar.gz"

cleanup() {
  rm -rf "$TMPDIR"
}
trap cleanup EXIT

# Download met progressbar
wget --show-progress https://downloads.arduino.cc/arduino-lint/arduino-lint_latest_Linux_64bit.tar.gz -O "$TARBALL"

# Uitpakken
tar -xzf "$TARBALL" -C "$TMPDIR"

# Zoek de binary (sommige releases bevatten direct de binary, andere in een map)
BIN_PATH=""
if [[ -f "$TMPDIR/arduino-lint" ]]; then
  BIN_PATH="$TMPDIR/arduino-lint"
else
  # zoek recursief
  BIN_PATH="$(find "$TMPDIR" -type f -name 'arduino-lint' -perm -u+x | head -n1 || true)"
fi

if [[ -z "$BIN_PATH" ]]; then
  echo "❌ Kon de 'arduino-lint' binary niet vinden na uitpakken."
  exit 1
fi

# Zorg dat hij uitvoerbaar is
chmod +x "$BIN_PATH"

# Verplaatsen naar /usr/local/bin
sudo mv "$BIN_PATH" /usr/local/bin/arduino-lint

echo "✅ Arduino Lint geïnstalleerd!"
echo "Versie:"
arduino-lint --version || true

echo
echo "Gebruik (in je projectmap):"
echo "  arduino-lint --recursive --project ."
