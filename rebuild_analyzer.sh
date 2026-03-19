#!/bin/bash

# Exit on error
set -e

# Paths
BUILD_DIR="$HOME/MyFiles/Root/proj2-SDK/build"
INSTALL_DIR="$HOME/MyFiles/Root/proj2-SDK/install"
SRC_DIR="$HOME/MyFiles/Root/proj2-SDK"

echo "🔄 Cleaning build directory..."
cd "$BUILD_DIR"
rm -rf *

echo "🧹 Cleaning install directory (requires sudo)..."
rm -rf "${INSTALL_DIR:?}"/*

echo "⚙️ Running cmake..."
cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" -DCMAKE_INSTALL_RPATH="$INSTALL_DIR"/lib ..

echo "🔨 Building with make..."
make -j8

echo "📦 Installing to $INSTALL_DIR (requires sudo)..."
make install

export LD_LIBRARY_PATH="$INSTALL_DIR/lib:$LD_LIBRARY_PATH"
export PATH="$INSTALL_DIR/bin:$PATH"

echo "✅ Done! V1720 rebuilt and environment sourced."
echo "   Run 'DecoderGUI' to launch."
