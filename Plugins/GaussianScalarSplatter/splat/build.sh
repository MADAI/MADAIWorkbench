#!/bin/sh
TARGET_BUILD_DIR="${HOME}/build/splat"
BUILD_TYPE="Release" #BUILD_TYPE="Debug"
VTK_DIR=""

SRC_DIR="$(cd "$(dirname "$0")" ; pwd)"
mkdir -p "$TARGET_BUILD_DIR"
cd "$TARGET_BUILD_DIR"
cmake "$SRC_DIR" \
  -DCMAKE_BUILD_TYPE:STRING="${BUILD_TYPE}" \
  -DVTK_DIR:PATH="${VTK_DIR}" \
  && make && {
	echo "SUCCESS:"; echo "  ${TARGET_BUILD_DIR}/splat"; }
