#!/bin/bash

# Creates a submission zip with src/ and inc/ directories
# Usage: bash make_zip.sh

set -e

PROJ_DIR="$(cd "$(dirname "$0")" && pwd)"
TIMESTAMP="$(date '+%Y-%m-%d_%H-%M-%S')"
OUT_DIR="$PROJ_DIR/submissions/$TIMESTAMP"
TMP_DIR="$(mktemp -d)"

trap "rm -rf '$TMP_DIR'" EXIT

mkdir -p "$OUT_DIR"
mkdir -p "$TMP_DIR/src" "$TMP_DIR/inc"

# Copy .cpp and .S files from src/
cp "$PROJ_DIR"/src/*.cpp "$TMP_DIR/src/" 2>/dev/null || true
cp "$PROJ_DIR"/src/*.S "$TMP_DIR/src/" 2>/dev/null || true

# Copy .h and .hpp files from h/
cp "$PROJ_DIR"/h/*.h "$TMP_DIR/inc/" 2>/dev/null || true
cp "$PROJ_DIR"/h/*.hpp "$TMP_DIR/inc/" 2>/dev/null || true

# Create zip
cd "$TMP_DIR"
zip -r "$OUT_DIR/projekat.zip" src/ inc/

echo ""
echo "=== Zip created: $OUT_DIR/projekat.zip ==="
echo ""
echo "Contents:"
unzip -l "$OUT_DIR/projekat.zip"
