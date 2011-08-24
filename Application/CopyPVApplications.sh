#!/bin/bash

BUILD_DIR="$1"
BUNDLE_DIR="$2"

echo "Copying ParaView applications to $BUNDLE_DIR/Contents/bin ..."

mkdir -p "$BUNDLE_DIR/Contents/bin"
cp "$BUILD_DIR"/bin/pv* "$BUNDLE_DIR"/Contents/bin
