#!/bin/bash
set -e

# Get the absolute path of the directory containing this script
SCRIPT_DIR="$(realpath .)"

./scripts/build.sh Release

# Ensure ~/.local/bin exists
mkdir -p "$HOME/.local/bin"

# Add soft links
ln -sf "$SCRIPT_DIR/bin/Release/qinp" "$HOME/.local/bin/qinp"
ln -sf "$SCRIPT_DIR/bin/Release/stdlib" "$HOME/.local/bin/qinp-std"

echo "Installed QINP."
