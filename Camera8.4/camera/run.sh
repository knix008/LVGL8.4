#!/bin/bash

# GTK Webcam Application Run Script
# This script sets up the environment and runs the gtk_webcam application

set -e

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================"
echo "GTK Webcam Viewer - Face Recognition"
echo "========================================${NC}"
echo ""

# Check if gtk_webcam executable exists
if [ ! -f "$SCRIPT_DIR/gtk_webcam" ]; then
    echo -e "${RED}Error: gtk_webcam executable not found at $SCRIPT_DIR/gtk_webcam${NC}"
    echo "Please run 'make' first to build the application."
    exit 1
fi

echo -e "${GREEN}✓ Executable found: gtk_webcam${NC}"
echo ""

# Set up library paths
# Detect system architecture
ARCH=$(uname -m)
if [ "$ARCH" = "aarch64" ]; then
    ONNX_DIR="$SCRIPT_DIR/onnxruntime-linux-aarch64-1.16.3"
else
    ONNX_DIR="$SCRIPT_DIR/onnxruntime-linux-x64-1.16.3"
fi

export LD_LIBRARY_PATH="$ONNX_DIR/lib:$LD_LIBRARY_PATH"

# Check if ONNX Runtime library exists
if [ ! -f "$ONNX_DIR/lib/libonnxruntime.so.1.16.3" ]; then
    echo -e "${YELLOW}Warning: ONNX Runtime library not found${NC}"
    echo "Expected: $ONNX_DIR/lib/libonnxruntime.so.1.16.3"
    echo ""
else
    echo -e "${GREEN}✓ ONNX Runtime library found ($ARCH)${NC}"
fi

# Check if FAISS library is available (optional)
if [ -f "$SCRIPT_DIR/faiss/lib/libfaiss.so" ]; then
    export LD_LIBRARY_PATH="$SCRIPT_DIR/faiss/lib:$LD_LIBRARY_PATH"
    echo -e "${GREEN}✓ FAISS library found and added to LD_LIBRARY_PATH${NC}"
else
    echo -e "${YELLOW}⚠ FAISS library not found - using in-memory index implementation${NC}"
fi

# Check if ArcFace model exists
if [ ! -f "$SCRIPT_DIR/models/arcface_w600k_r50.onnx" ] || [ ! -s "$SCRIPT_DIR/models/arcface_w600k_r50.onnx" ]; then
    echo -e "${YELLOW}⚠ Warning: ArcFace model file is missing or empty${NC}"
    echo "  Location: $SCRIPT_DIR/models/arcface_w600k_r50.onnx"
    echo "  Face recognition will not work until a valid ArcFace ONNX model is provided."
    echo ""
fi

# Check if database directory exists
if [ ! -d "$SCRIPT_DIR/models" ]; then
    echo -e "${YELLOW}⚠ Creating models directory...${NC}"
    mkdir -p "$SCRIPT_DIR/models"
fi

# Check camera availability
if [ ! -e /dev/video0 ]; then
    echo -e "${YELLOW}⚠ Warning: Camera device /dev/video0 not found${NC}"
    echo "  The application will start but camera features will not work."
    echo "  Make sure:"
    echo "    1. Your system has a camera"
    echo "    2. The camera is not in use by another application"
    echo ""
fi

echo -e "${GREEN}Starting GTK Webcam Viewer...${NC}"
echo ""
echo "Environment:"
echo "  LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
echo "  Working directory: $SCRIPT_DIR"
echo ""
echo -e "${YELLOW}Press Ctrl+C to exit${NC}"
echo ""

# Run the application
cd "$SCRIPT_DIR"
exec ./gtk_webcam "$@"
