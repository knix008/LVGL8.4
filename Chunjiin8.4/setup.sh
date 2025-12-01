#!/bin/bash
# Chunjiin Korean Input Method - Automated Setup Script
# Sets up LVGL environment and builds the application
# Version: 2.0 - Enhanced with improved font loading and build system

set -e  # Exit on error

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=========================================${NC}"
echo -e "${BLUE} Chunjiin Korean Input Method - Setup ${NC}"
echo -e "${BLUE} 천지인 한글 입력기 ${NC}"
echo -e "${BLUE}=========================================${NC}"
echo ""

# Helper function to print colored messages
print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_info() {
    echo -e "${YELLOW}→${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_step() {
    echo -e "${BLUE}==>${NC} $1"
}

# Function to count total steps
print_step_progress() {
    echo -e "${BLUE}[$1/$2]${NC} $3"
}

# Validate Linux OS
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    print_error "This script is designed for Linux systems"
    print_error "Current OS: $OSTYPE"
    exit 1
fi

print_step "Step 1 of 6: Checking system requirements..."
echo ""

MISSING_PACKAGES=()

# Check for SDL2
if ! pkg-config --exists sdl2; then
    print_error "SDL2 development libraries not found"
    MISSING_PACKAGES+=("libsdl2-dev")
else
    print_success "SDL2 found"
fi

# Check for build tools
if ! command -v gcc &> /dev/null; then
    print_error "GCC not found"
    MISSING_PACKAGES+=("build-essential")
else
    print_success "GCC found"
fi

if ! command -v git &> /dev/null; then
    print_error "Git not found"
    MISSING_PACKAGES+=("git")
else
    print_success "Git found"
fi

# Check for FreeType
if ! pkg-config --exists freetype2; then
    print_error "FreeType development libraries not found"
    MISSING_PACKAGES+=("libfreetype6-dev")
else
    print_success "FreeType found"
fi

# Install missing packages if any
if [ ${#MISSING_PACKAGES[@]} -gt 0 ]; then
    echo ""
    echo "Missing packages: ${MISSING_PACKAGES[*]}"
    echo ""
    read -p "Install missing packages? (requires sudo) [Y/n]: " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
        print_info "Installing packages..."
        sudo apt-get update
        sudo apt-get install -y "${MISSING_PACKAGES[@]}"
        print_success "Packages installed"
    else
        print_error "Cannot continue without required packages"
        exit 1
    fi
fi

echo ""

# 2. Clone LVGL
print_step "Step 2 of 6: Setting up LVGL framework..."
echo ""

if [ -d "lvgl" ]; then
    print_info "LVGL directory already exists, skipping clone"
    LVGL_SIZE=$(du -sh lvgl | cut -f1)
    print_info "LVGL size: $LVGL_SIZE"
else
    print_info "Cloning LVGL v8.4 from GitHub..."
    print_info "This may take a minute..."
    git clone --depth 1 --branch release/v8.4 https://github.com/lvgl/lvgl.git
    print_success "LVGL v8.4 cloned successfully"
fi

echo ""

# 3. Verify font files
print_step "Step 3 of 6: Verifying font files..."
echo ""

# Font file paths - check multiple options
FONT_REGULAR="assets/NanumGothic-Regular.ttf"
FONT_CODING="assets/NanumGothicCoding.ttf"
FONT_BOLD="assets/NanumGothic-Bold.ttf"
FONT_CODING_BOLD="assets/NanumGothicCoding-Bold.ttf"

if [ ! -d "assets" ]; then
    print_error "assets directory not found"
    print_info "Creating assets directory..."
    mkdir -p assets
    print_warning "Please download Korean font files and place them in assets/"
    exit 1
fi

# Check for at least one regular font file
FONT_FOUND=false
if [ -f "$FONT_REGULAR" ]; then
    FONT_SIZE=$(du -h "$FONT_REGULAR" | cut -f1)
    print_success "NanumGothic-Regular.ttf found ($FONT_SIZE)"
    FONT_FOUND=true
elif [ -f "$FONT_CODING" ]; then
    FONT_SIZE=$(du -h "$FONT_CODING" | cut -f1)
    print_success "NanumGothicCoding.ttf found ($FONT_SIZE)"
    FONT_FOUND=true
else
    print_error "No regular font file found"
    FONT_FOUND=false
fi

# Check for bold font
if [ -f "$FONT_BOLD" ]; then
    BOLD_SIZE=$(du -h "$FONT_BOLD" | cut -f1)
    print_success "NanumGothic-Bold.ttf found ($BOLD_SIZE)"
elif [ -f "$FONT_CODING_BOLD" ]; then
    BOLD_SIZE=$(du -h "$FONT_CODING_BOLD" | cut -f1)
    print_success "NanumGothicCoding-Bold.ttf found ($BOLD_SIZE)"
else
    print_warning "No bold font file found (will use regular font as fallback)"
fi

if [ "$FONT_FOUND" = false ]; then
    echo ""
    print_error "No Korean font file found!"
    echo ""
    print_warning "Font Setup Instructions:"
    echo "  1. Download NanumGothic fonts from Google Fonts:"
    echo "     https://fonts.google.com/noto/specimen/Noto+Sans+KR"
    echo ""
    echo "  2. Or download NanumGothic Coding fonts from:"
    echo "     https://github.com/naver/nanumfont"
    echo ""
    echo "  3. Place the .ttf files in the assets/ directory"
    echo "  4. Run this script again"
    exit 1
fi

echo ""

# 4. Build LVGL library
print_step "Step 4 of 6: Building LVGL library..."
echo ""

if [ -f "lvgl/lib/liblvgl.a" ]; then
    LIBSIZE=$(du -h lvgl/lib/liblvgl.a | cut -f1)
    print_info "LVGL library already built ($LIBSIZE), skipping rebuild"
else
    print_info "Building LVGL static library..."

    # Create build and lib directories for LVGL
    mkdir -p lvgl/build
    mkdir -p lvgl/lib

    # Find all LVGL source files
    LVGL_SOURCES=$(find lvgl/src -name "*.c")
    TOTAL_FILES=$(echo "$LVGL_SOURCES" | wc -l)
    CURRENT=0

    print_info "Compiling $TOTAL_FILES LVGL source files..."
    echo ""

    for src in $LVGL_SOURCES; do
        obj="lvgl/build/$(basename ${src%.c}.o)"
        CURRENT=$((CURRENT + 1))

        if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ]; then
            printf "  [%3d/%3d] Compiling $(basename $src)...\n" "$CURRENT" "$TOTAL_FILES"
            gcc -Wall -Wextra -O2 -I. -Ilvgl $(pkg-config --cflags freetype2) -c "$src" -o "$obj" 2>&1 | grep -E "error|warning" || true
        fi
    done

    echo ""
    print_info "Creating static library archive..."
    ar rcs lvgl/lib/liblvgl.a lvgl/build/*.o
    LIBSIZE=$(du -h lvgl/lib/liblvgl.a | cut -f1)
    print_success "LVGL library built successfully ($LIBSIZE)"
fi

echo ""

# 5. Build the application
print_step "Step 5 of 6: Building Chunjiin application..."
echo ""

print_info "Font loading system:"
echo "  • Dynamic font loading via FreeType library"
echo "  • Optimized font initialization function"
echo "  • Supports multiple sizes: 12, 14, 16, 20 pixels"
echo "  • Enhanced error handling with file validation"
echo "  • No pre-compilation or font conversion needed"
echo ""

print_info "Cleaning previous builds..."
make clean 2>/dev/null || true

print_info "Compiling Chunjiin application..."
if make 2>&1 | tee build.log; then
    BUILD_SIZE=$(du -h chunjiin | cut -f1)
    print_success "Build successful! (Binary size: $BUILD_SIZE)"
    echo ""
    print_info "Build log saved to build.log"
else
    print_error "Build failed!"
    echo ""
    print_warning "Common issues and solutions:"
    echo "  1. lv_conf.h not found"
    echo "     → Make sure lv_conf.h exists in the project root"
    echo ""
    echo "  2. SDL2 development libraries missing"
    echo "     → Run: sudo apt-get install libsdl2-dev"
    echo ""
    echo "  3. LVGL library build failed"
    echo "     → Try: rm -rf lvgl/build lvgl/lib && run this script again"
    echo ""
    echo "  4. FreeType not found"
    echo "     → Run: sudo apt-get install libfreetype6-dev"
    echo ""
    print_info "See build.log for detailed error messages"
    exit 1
fi

echo ""

# 6. Summary and features
print_step "Step 6 of 6: Setup complete!"
echo ""
echo -e "${BLUE}=========================================${NC}"
echo -e "${GREEN}✓ Setup Complete!${NC}"
echo -e "${BLUE}=========================================${NC}"
echo ""
echo "The Chunjiin Korean Input Method is ready to use!"
echo ""

print_info "To run the application:"
echo "  • Direct execution:  ./chunjiin"
echo "  • Using make:        make run"
echo ""

print_info "Features:"
echo "  ✓ Korean input using Chunjiin (천지인) method"
echo "  ✓ Real-time character composition"
echo "  ✓ Incomplete character display support"
echo "  ✓ Multiple input modes: 한글, 영문, 숫자, 특수문자"
echo "  ✓ Beautiful Korean fonts via FreeType + NanumGothic"
echo "  ✓ LVGL-based GUI with SDL2 graphics"
echo ""

print_info "Project information:"
echo "  • Source files: main.c, chunjiin.c, chunjiin_hangul.c"
echo "  • Configuration: lv_conf.h"
echo "  • Documentation: README.md"
echo ""

# Ask if user wants to run the application
echo -n "Run the application now? [Y/n]: "
read -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
    print_step "Starting Chunjiin Korean Input Method..."
    echo ""
    ./chunjiin
else
    print_info "Setup completed. You can run the application anytime with: ./chunjiin"
fi
