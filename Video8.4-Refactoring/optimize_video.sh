#!/bin/bash

# Video Optimization Script for LVGL FFmpeg Player
# This script optimizes videos for smooth playback on the 320x520 display

if [ $# -eq 0 ]; then
    echo "Usage: $0 <input_video> [output_name]"
    echo ""
    echo "Example:"
    echo "  $0 my_video.mp4"
    echo "  $0 my_video.mp4 optimized.mp4"
    echo ""
    echo "This will create an optimized video in assets/videos/"
    echo "Optimizations:"
    echo "  - Resolution: 320x640 (full screen with overlay bars)"
    echo "  - Frame rate: 24 fps (smooth playback)"
    echo "  - Codec: H.264 with ultrafast preset"
    echo "  - Quality: CRF 28 (good quality, small size)"
    echo "  - Audio: Removed (not needed)"
    exit 1
fi

INPUT_FILE="$1"
OUTPUT_NAME="${2:-optimized.mp4}"

# Check if input file exists
if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: Input file '$INPUT_FILE' not found"
    exit 1
fi

# Create output directory if it doesn't exist
mkdir -p assets/videos

# Set output path
OUTPUT_FILE="assets/videos/$OUTPUT_NAME"

echo "========================================="
echo "Video Optimization for LVGL FFmpeg Player"
echo "========================================="
echo ""
echo "Input:  $INPUT_FILE"
echo "Output: $OUTPUT_FILE"
echo ""
echo "Settings:"
echo "  - Resolution: 320x640"
echo "  - Frame rate: 24 fps"
echo "  - Preset: ultrafast (for performance)"
echo "  - Quality: CRF 28 (good quality)"
echo ""
echo "Starting conversion..."
echo ""

# Optimize video
ffmpeg -i "$INPUT_FILE" \
    -vf "scale=320:640:force_original_aspect_ratio=decrease,pad=320:640:(ow-iw)/2:(oh-ih)/2" \
    -c:v libx264 \
    -preset ultrafast \
    -crf 28 \
    -r 24 \
    -an \
    "$OUTPUT_FILE" \
    -y

if [ $? -eq 0 ]; then
    echo ""
    echo "========================================="
    echo "Optimization Complete!"
    echo "========================================="
    echo ""

    # Show file sizes
    INPUT_SIZE=$(du -h "$INPUT_FILE" | cut -f1)
    OUTPUT_SIZE=$(du -h "$OUTPUT_FILE" | cut -f1)

    echo "Original size: $INPUT_SIZE"
    echo "Optimized size: $OUTPUT_SIZE"
    echo ""
    echo "Video saved to: $OUTPUT_FILE"
    echo ""
    echo "To use this video, make sure it's the only video file in assets/videos/"
    echo "or rename it to Video.mp4"
else
    echo ""
    echo "Error: Video optimization failed"
    exit 1
fi
