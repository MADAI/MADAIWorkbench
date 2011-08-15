#!/bin/bash

BUILD_DIR="$1"
BUNDLE_DIR="$2"

echo "Copying custom plugins to $BUNDLE_DIR/Contents/Plugins ..."

mkdir -p "$BUNDLE_DIR/Contents/Plugins"
cp "$BUILD_DIR"/bin/*.dylib "$BUNDLE_DIR"/Contents/Plugins