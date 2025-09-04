#!/usr/bin/env bash
set -euo pipefail

command -v curl >/dev/null 2>&1 || command -v wget >/dev/null 2>&1 || {
  echo "⚠️  curl of wget ontbreekt. Installeer er één, of doe 'pip install requests'.";
  exit 1
}

if ! command -v west >/dev/null 2>&1; then
  echo "west niet gevonden. Activeer je venv en installeer: pip install west"
  exit 1
fi

echo "→ Blobs ophalen voor hal_espressif..."
west blobs fetch hal_espressif
echo "✅ Klaar."
