#!/usr/bin/env bash
# Robust Zephyr setup helper for Ubuntu / Linux
# - Ensures we're inside a west workspace
# - Uses `west packages` when available (Zephyr ≥ 4.1 within a workspace)
# - Falls back to pip-installing Zephyr tool requirements if `packages` is unavailable
# - Optionally installs the Zephyr SDK when WEST_SDK_INSTALL=1
#
# Usage:
#   ./setup-zephyr-env.sh
#
# Optional env vars:
#   VENV_PATH=path/to/.venv      # If set, source this virtualenv before installing anything
#   WEST_SDK_INSTALL=1           # If set to 1, attempt 'west sdk install' (may be interactive)
#   EXTRA_PIP_ARGS="--user"      # Extra args for pip installs (if not using venv)
#
# Notes:
# - Run this from anywhere *inside* your west workspace (the tree containing the .west/ directory).
# - If you haven't initialized a workspace yet, do something like:
#     mkdir zephyrproject && cd zephyrproject
#     west init -m https://github.com/zephyrproject-rtos/zephyr
#     west update
# - `west packages` exists only when the workspace defines the Zephyr extension commands (Zephyr ≥ 4.1).
#   See: https://docs.zephyrproject.org/latest/develop/west/index.html
set -euo pipefail

log() { printf '\033[1;34m[setup]\033[0m %s\n' "$*"; }
warn() { printf '\033[1;33m[warn]\033[0m %s\n' "$*"; }
err() { printf '\033[1;31m[error]\033[0m %s\n' "$*"; }

# 1) Optionally activate venv
if [[ -n "${VENV_PATH:-}" ]]; then
  if [[ -f "$VENV_PATH/bin/activate" ]]; then
    log "Activating virtualenv: $VENV_PATH"
    # shellcheck disable=SC1090
    source "$VENV_PATH/bin/activate"
  else
    warn "VENV_PATH is set but activate script not found: $VENV_PATH/bin/activate"
  fi
fi

# 2) Sanity checks for west
if ! command -v west >/dev/null 2>&1; then
  err "west is not installed. Install with:  pip install --upgrade west"
  exit 2
fi

log "west version: $(west --version 2>/dev/null || echo unknown)"

# 3) Ensure we're in a west workspace (detect topdir)
WEST_TOPDIR=""
if west topdir >/dev/null 2>&1; then
  WEST_TOPDIR="$(west topdir)"
else
  # Fallback: search upwards for .west directory
  CUR="$(pwd)"
  while [[ "$CUR" != "/" ]]; do
    if [[ -d "$CUR/.west" ]]; then
      WEST_TOPDIR="$CUR"
      break
    fi
    CUR="$(dirname "$CUR")"
  done
fi

if [[ -z "$WEST_TOPDIR" ]]; then
  err "Not inside a west workspace (no .west/ found). Run this inside your Zephyr workspace root."
  err "Tip: create one with 'west init' then 'west update'."
  exit 3
fi

log "Workspace detected at: $WEST_TOPDIR"
cd "$WEST_TOPDIR"

# 4) Update projects
log "Running 'west update' to sync manifest projects…"
west update

# Fetch Espressif Wi‑Fi/Bluetooth blobs required for ESP32‑S3 builds.
# The Espressif HAL requires binary blobs for Wi‑Fi and Bluetooth; they
# must be downloaded after every `west update` to ensure version
# compatibility【212531024432480†L938-L950】.  Doing this here makes
# the board buildable out of the box.  If the blobs are already up to
# date, this command is a no‑op.
log "Fetching Espressif binary blobs (hal_espressif)…"
if ! west blobs fetch hal_espressif; then
  warn "Failed to fetch hal_espressif blobs. Zephyr builds for ESP32‑S3 may fail until the blobs are downloaded."
fi

# 5) Detect if 'west packages' exists (requires Zephyr extension commands in this workspace)
if west help packages >/dev/null 2>&1; then
  log "'west packages' is available. Installing Zephyr Python tools via packages…"
  west packages list || true
  if [[ -n "${EXTRA_PIP_ARGS:-}" ]]; then
    west packages pip --install --pip-extra-args "$EXTRA_PIP_ARGS"
  else
    west packages pip --install
  fi
else
  warn "'west packages' not available in this workspace. Falling back to requirements install."
  ZEPHYR_PATH="$(west list -f '{path}' zephyr 2>/dev/null || true)"
  if [[ -z "$ZEPHYR_PATH" || ! -d "$ZEPHYR_PATH" ]]; then
    warn "Could not locate Zephyr repo via 'west list'. Trying common path '$WEST_TOPDIR/zephyr'."
    ZEPHYR_PATH="$WEST_TOPDIR/zephyr"
  fi

  if [[ ! -d "$ZEPHYR_PATH" ]]; then
    err "Zephyr repo not found (looked for: $ZEPHYR_PATH). Cannot install requirements."
    exit 4
  fi

  REQ_PRIMARY="$ZEPHYR_PATH/scripts/requirements.txt"
  REQ_EXTRA="$ZEPHYR_PATH/scripts/requirements-extra.txt"
  PIP="pip"
  if command -v pip3 >/dev/null 2>&1; then PIP="pip3"; fi

  log "Installing Zephyr Python requirements from $REQ_PRIMARY"
  $PIP install -U -r "$REQ_PRIMARY" ${EXTRA_PIP_ARGS:-}
  if [[ -f "$REQ_EXTRA" ]]; then
    log "Installing extra Zephyr Python requirements from $REQ_EXTRA"
    $PIP install -U -r "$REQ_EXTRA" ${EXTRA_PIP_ARGS:-}
  fi
fi

# 6) Optional: install Zephyr SDK (may be interactive depending on version)
if [[ "${WEST_SDK_INSTALL:-0}" == "1" ]]; then
  log "Attempting Zephyr SDK install via 'west sdk install' (this may be interactive)…"
  if ! west sdk install; then
    warn "'west sdk install' did not complete successfully. You can install the SDK manually later."
  fi
else
  log "Skipping SDK install. Set WEST_SDK_INSTALL=1 to run 'west sdk install'."
fi

log "✅ Zephyr environment setup finished."

