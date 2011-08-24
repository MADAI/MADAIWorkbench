#!/bin/bash

PARAVIEW_BUILD_DIR="$1"
BUNDLE_DIR="$2"

echo "Copying Python files to $BUNDLE_DIR/Contents/Plugins ..."

LibrariesDir="$BUNDLE_DIR/Contents/MacOS"
PythonDir="$BUNDLE_DIR/Contents/Python"
mkdir -p "$PythonDir"
cp -Rp "$PARAVIEW_BUILD_DIR/Utilities/VTKPythonWrapping/site-packages/" "$PythonDir/"


cp -p "$PARAVIEW_BUILD_DIR"/bin/*Python.so "$LibrariesDir/"
