#!/bin/bash

# Setup script for MAC Address Hex Input with LVGL

set -e  # Exit on error

echo "==================================="
echo "Qwerty Input Setup Script"
echo "==================================="
echo ""

# Check for required tools
echo "Checking required tools..."
command -v gcc >/dev/null 2>&1 || { echo "Error: gcc not found. Install build-essential"; exit 1; }
command -v make >/dev/null 2>&1 || { echo "Error: make not found. Install build-essential"; exit 1; }
command -v pkg-config >/dev/null 2>&1 || { echo "Error: pkg-config not found"; exit 1; }

# Check for SDL2
echo "Checking SDL2..."
if ! pkg-config --exists sdl2; then
    echo "SDL2 not found. Installing..."
    sudo apt-get update
    sudo apt-get install -y libsdl2-dev
fi

# Check for FreeType
echo "Checking FreeType..."
if ! pkg-config --exists freetype2; then
    echo "FreeType not found. Installing..."
    sudo apt-get install -y libfreetype-dev
fi

# Clone LVGL if not present
if [ ! -d "lvgl" ]; then
    echo ""
    echo "Cloning LVGL v8.4..."
    git clone https://github.com/lvgl/lvgl.git
    cd lvgl
    git checkout release/v8.4
    cd ..
else
    echo ""
    echo "LVGL directory already exists"
fi

# Check for lv_conf.h
if [ ! -f "lv_conf.h" ]; then
    echo "Error: lv_conf.h not found!"
    exit 1
fi

echo ""
echo "==================================="
echo "Building LVGL Library..."
echo "==================================="
echo ""

# Build LVGL as a static library
cd lvgl

# Create build directory if it doesn't exist
mkdir -p build

# Get compiler flags
SDL_CFLAGS=$(pkg-config --cflags sdl2 2>/dev/null || echo "-I/usr/include/SDL2")
FREETYPE_CFLAGS=$(pkg-config --cflags freetype2 2>/dev/null || echo "-I/usr/include/freetype2")

# Compile all LVGL source files to object files
echo "Compiling LVGL sources..."
find src -name "*.c" | while read src_file; do
    obj_file="build/$(echo $src_file | sed 's/\.c$/.o/' | sed 's/src\///')"
    obj_dir=$(dirname "$obj_file")
    mkdir -p "$obj_dir"
    
    # Only compile if source is newer than object or object doesn't exist
    if [ ! -f "$obj_file" ] || [ "$src_file" -nt "$obj_file" ]; then
        echo "  Compiling $src_file..."
        gcc -Wall -Wextra -std=c11 -O2 -D_DEFAULT_SOURCE \
            -I.. -I. $SDL_CFLAGS $FREETYPE_CFLAGS \
            -c "$src_file" -o "$obj_file"
    fi
done

# Create static library
echo ""
echo "Creating static library liblvgl.a..."
find build -name "*.o" | xargs ar rcs build/liblvgl.a
ranlib build/liblvgl.a

cd ..

echo ""
echo "==================================="
echo "Setup Complete!"
echo "==================================="
echo ""
echo "LVGL has been compiled to: lvgl/build/liblvgl.a"
echo ""
echo "Next steps:"
echo "  - Run 'make' to build the application"
echo ""
