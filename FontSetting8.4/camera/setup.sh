#!/bin/bash

# FAISS + Deep Learning Dependencies Setup Script
# Installs ONNX Runtime and FAISS for the face recognition application

set -e  # Exit on error

echo "=========================================="
echo "FAISS + Deep Learning Setup Script"
echo "=========================================="
echo ""
echo "This script will:"
echo "  1. Install system dependencies (may require sudo)"
echo "  2. Build FAISS v1.7.4 locally (no sudo needed)"
echo "  3. Download ArcFace ONNX model (InsightFace w600k_r50)"
echo ""

# Detect OS
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
else
    echo "Error: Could not detect OS"
    exit 1
fi

echo "Detected OS: $OS"
echo ""

# Install system dependencies
echo "Step 1: Installing system dependencies..."
echo ""

case "$OS" in
    ubuntu|debian)
        echo "Installing Ubuntu/Debian packages..."
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            cmake \
            pkg-config \
            python3 \
            python3-pip \
            libgomp1 \
            libopenblas-dev \
            liblapack-dev \
            gfortran \
            swig \
            git \
            libgtk-3-dev \
            libgdk-pixbuf2.0-dev \
            libopencv-dev \
            libsqlite3-dev
        ;;
    fedora|rhel|centos)
        echo "Installing Fedora/RHEL packages..."
        sudo dnf groupinstall -y "Development Tools" "Development Libraries"
        sudo dnf install -y \
            cmake \
            pkg-config \
            python3 \
            python3-devel \
            openblas-devel \
            lapack-devel \
            gcc-gfortran \
            swig \
            git \
            gtk3-devel \
            opencv-devel \
            sqlite-devel
        ;;
    arch)
        echo "Installing Arch Linux packages..."
        sudo pacman -S --noconfirm \
            base-devel \
            cmake \
            pkg-config \
            python \
            openblas \
            lapack \
            swig \
            git \
            gtk3 \
            opencv \
            sqlite
        ;;
    *)
        echo "Unsupported OS: $OS"
        echo "Please install build-essential, cmake, GTK-3-dev, OpenCV-dev, SQLite3-dev and BLAS/LAPACK development libraries manually"
        exit 1
        ;;
esac

echo "✓ System dependencies installed"
echo ""

# Install ONNX Runtime
echo "Step 2: Installing ONNX Runtime v1.16.3..."
echo ""

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ONNX_VERSION="1.16.3"

# Detect CPU architecture
ARCH=$(uname -m)
case "$ARCH" in
    x86_64)
        ONNX_ARCH="x64"
        ;;
    aarch64|arm64)
        ONNX_ARCH="aarch64"
        ;;
    *)
        echo "Error: Unsupported architecture: $ARCH"
        echo "Supported: x86_64, aarch64"
        exit 1
        ;;
esac

echo "Detected architecture: $ARCH ($ONNX_ARCH)"

# Check if already installed locally
if [ -d "$SCRIPT_DIR/onnxruntime-linux-$ONNX_ARCH-$ONNX_VERSION" ]; then
    echo "✓ ONNX Runtime already installed locally"
else
    echo "Downloading ONNX Runtime v$ONNX_VERSION for $ONNX_ARCH..."

    ONNX_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/onnxruntime-linux-${ONNX_ARCH}-${ONNX_VERSION}.tgz"
    ONNX_ARCHIVE="$SCRIPT_DIR/onnxruntime-linux-${ONNX_ARCH}-${ONNX_VERSION}.tgz"

    cd "$SCRIPT_DIR"

    if wget -q --show-progress "$ONNX_URL" -O "$ONNX_ARCHIVE"; then
        echo "Download completed"
        echo "Extracting ONNX Runtime..."
        if tar -xzf "$ONNX_ARCHIVE"; then
            echo "Extraction completed"
            rm -f "$ONNX_ARCHIVE"
            echo "✓ ONNX Runtime v$ONNX_VERSION installed locally"
        else
            echo "Error: Failed to extract ONNX Runtime"
            exit 1
        fi
    else
        echo "Error: Failed to download ONNX Runtime"
        exit 1
    fi
fi

echo ""

# Install FAISS
echo "Step 3: Building and installing FAISS v1.7.4 locally..."
echo ""

FAISS_LOCAL_DIR="$SCRIPT_DIR/faiss"

# Check if FAISS is already installed locally
if [ -d "$FAISS_LOCAL_DIR" ] && [ -f "$FAISS_LOCAL_DIR/lib/libfaiss.so" ]; then
    echo "✓ FAISS v1.7.4 already installed locally at $FAISS_LOCAL_DIR"
else
    echo "Building FAISS v1.7.4 from source (this may take 5-15 minutes)..."
    echo ""

    # Create temp directory for build
    TEMP_DIR=$(mktemp -d)
    echo "Using temporary directory: $TEMP_DIR"

    # Clone FAISS v1.7.4
    echo "Cloning FAISS v1.7.4 from GitHub..."
    git clone --depth 1 --branch v1.7.4 https://github.com/facebookresearch/faiss.git "$TEMP_DIR/faiss"
    cd "$TEMP_DIR/faiss"

    # Create build directory
    echo "Creating build directory..."
    mkdir -p build
    cd build

    # Configure with CMake - install to local faiss directory
    echo "Configuring CMake with OpenBLAS backend..."
    cmake .. \
        -DFAISS_ENABLE_GPU=OFF \
        -DFAISS_ENABLE_PYTHON=OFF \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
        -DCMAKE_INSTALL_PREFIX="$FAISS_LOCAL_DIR" \
        -DBLA_VENDOR=OpenBLAS

    if [ $? -ne 0 ]; then
        echo "Error: CMake configuration failed"
        rm -rf "$TEMP_DIR"
        exit 1
    fi

    # Build
    echo ""
    echo "Building FAISS (using $(nproc) CPU cores)..."
    echo "This may take 5-15 minutes, please wait..."
    make -j$(nproc)

    if [ $? -ne 0 ]; then
        echo "Error: FAISS build failed"
        rm -rf "$TEMP_DIR"
        exit 1
    fi

    # Install locally (no sudo needed!)
    echo ""
    echo "Installing FAISS to local directory: $FAISS_LOCAL_DIR"
    make install

    if [ $? -ne 0 ]; then
        echo "Error: FAISS installation failed"
        rm -rf "$TEMP_DIR"
        exit 1
    fi

    # Cleanup temp directory
    echo "Cleaning up temporary files..."
    rm -rf "$TEMP_DIR"

    echo "✓ FAISS v1.7.4 installed successfully to $FAISS_LOCAL_DIR"
fi

echo ""

# Download ArcFace model
echo "Step 4: Downloading ArcFace ONNX model (InsightFace w600k_r50)..."
echo ""

# Ensure we're in the script directory
cd "$SCRIPT_DIR"

MODEL_DIR="models"
MODEL_FILE="$MODEL_DIR/arcface_w600k_r50.onnx"
MODEL_URL="https://huggingface.co/public-data/insightface/resolve/main/models/buffalo_l/w600k_r50.onnx"

# Create models directory if it doesn't exist
if ! mkdir -p "$MODEL_DIR" 2>/dev/null; then
    echo "Warning: Could not create models directory"
    echo "Attempting to continue anyway..."
fi

if [ -f "$MODEL_FILE" ]; then
    FILE_SIZE=$(du -h "$MODEL_FILE" | cut -f1)
    echo "✓ ArcFace model already exists ($FILE_SIZE)"
else
    echo "Downloading ArcFace model (~166MB)..."
    echo "From: $MODEL_URL"
    echo ""

    if wget --show-progress "$MODEL_URL" -O "$MODEL_FILE" 2>&1; then
        if [ -f "$MODEL_FILE" ] && [ -s "$MODEL_FILE" ]; then
            FILE_SIZE=$(du -h "$MODEL_FILE" | cut -f1)
            echo ""
            echo "✓ ArcFace model downloaded successfully ($FILE_SIZE)"
        else
            echo "Warning: Model file is empty or incomplete"
            echo "You can download it manually:"
            echo "  mkdir -p models"
            echo "  wget -O models/arcface_w600k_r50.onnx '$MODEL_URL'"
            echo ""
        fi
    else
        echo "Warning: Failed to download ArcFace model"
        echo "You can download it manually:"
        echo "  mkdir -p models"
        echo "  wget -O models/arcface_w600k_r50.onnx '$MODEL_URL'"
        echo ""
    fi
fi

echo ""

# Update library cache (only if running system-wide installations)
echo "Step 5: Updating library cache..."
if command -v ldconfig &> /dev/null; then
    sudo ldconfig 2>/dev/null || echo "⚠ Could not update system library cache (may need sudo, but not critical for local FAISS)"
    echo "✓ Library cache updated"
else
    echo "⚠ ldconfig not available (not critical for local FAISS installation)"
fi
echo ""

# Verify installations
echo "Step 6: Verifying installations..."
echo ""

# Check ONNX Runtime
if pkg-config --exists onnxruntime 2>/dev/null; then
    echo "✓ ONNX Runtime: $(pkg-config --modversion onnxruntime)"
else
    echo "⚠ ONNX Runtime: Not found via pkg-config"
    if ldconfig -p | grep -q libonnxruntime.so; then
        echo "  (but library found in system)"
    fi
fi

# Check FAISS
if pkg-config --exists faiss 2>/dev/null; then
    echo "✓ FAISS: $(pkg-config --modversion faiss)"
else
    echo "⚠ FAISS: Not found via pkg-config"
    if ldconfig -p | grep -q libfaiss.so; then
        echo "  (but library found in system)"
    fi
fi

# Check GTK
if pkg-config --exists gtk+-3.0 2>/dev/null; then
    echo "✓ GTK-3: $(pkg-config --modversion gtk+-3.0)"
else
    echo "⚠ GTK-3: Not found"
fi

# Check OpenCV
if pkg-config --exists opencv4 2>/dev/null; then
    echo "✓ OpenCV: $(pkg-config --modversion opencv4)"
else
    echo "⚠ OpenCV: Not found"
fi

echo ""
echo "=========================================="
echo "✓ Setup Complete!"
echo "=========================================="
echo ""
echo "All dependencies installed:"
echo "  ✓ System packages (GTK-3, OpenCV, SQLite3, BLAS/LAPACK)"
echo "  ✓ ONNX Runtime v1.16.3"
echo "  ✓ FAISS v1.7.4"
if [ -f "models/arcface_w600k_r50.onnx" ]; then
    FACEMODEL_SIZE=$(du -h models/arcface_w600k_r50.onnx | cut -f1)
    echo "  ✓ ArcFace Model ($FACEMODEL_SIZE)"
else
    echo "  ⚠ ArcFace Model (download failed, but can be added manually)"
fi
echo ""
echo "Next steps:"
echo ""
echo "1. Build the application:"
echo "   make clean && make"
echo ""
echo "2. Run the application:"
echo "   ./gtk_webcam"
echo ""
echo "For more information, see INTEGRATION_COMPLETE.md or SETUP_FAISS_DEEPLEARNING.md"
echo ""
