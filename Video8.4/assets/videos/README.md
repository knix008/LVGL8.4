# Video Playback Feature

## Overview
This application now supports automatic video playback using FFmpeg on the home screen after 10 seconds of user inactivity.

## How It Works
1. When the home screen is active, the system monitors user activity (touch/click events)
2. If there is no user interaction for 10 seconds, the slideshow and welcome message will be hidden
3. The video player will automatically start playing the video in loop mode
4. When the user interacts with the screen again (touch/click), the video stops and the slideshow resumes

## Setting Up Videos

### Video Format
The application uses LVGL's FFmpeg integration for native video playback. Any video format supported by FFmpeg can be used.

### Supported Formats
- MP4 (H.264, H.265)
- AVI
- MKV
- MOV
- WebM
- And many more formats supported by FFmpeg

### Preparing Your Video

1. **Place your video file in this directory:**
   ```
   assets/videos/
   ```

2. **Recommended video specifications:**
   - Resolution: 320x520 pixels (to fit between title bar and status bar)
   - Codec: H.264 for best compatibility
   - Container: MP4
   - Frame rate: 24-30 FPS

3. **Convert video to recommended format using FFmpeg:**
   ```bash
   ffmpeg -i input_video.mp4 -vf "scale=320:520" -c:v libx264 -preset medium -crf 23 -c:a aac -b:a 128k assets/videos/video.mp4
   ```

### Example Commands

**Create a test video from images:**
```bash
# Create a 10-second test pattern
ffmpeg -f lavfi -i testsrc=duration=10:size=320x520:rate=30 -c:v libx264 -preset fast assets/videos/test.mp4
```

**Convert an existing video:**
```bash
# Convert and resize video
ffmpeg -i /path/to/your/video.mp4 -vf "scale=320:520:force_original_aspect_ratio=decrease,pad=320:520:(ow-iw)/2:(oh-ih)/2" -c:v libx264 -crf 23 assets/videos/video.mp4
```

**Extract a clip from a longer video:**
```bash
# Extract 30 seconds starting from 1 minute
ffmpeg -i input.mp4 -ss 00:01:00 -t 00:00:30 -vf "scale=320:520" -c:v libx264 -crf 23 assets/videos/clip.mp4
```

## Installation and Setup

### 1. Install FFmpeg libraries
Run the setup script which will automatically install FFmpeg dependencies:
```bash
./setup.sh
```

This will install:
- libavformat-dev
- libavcodec-dev
- libavutil-dev
- libswscale-dev

### 2. Build the application
After running setup.sh, rebuild the application:
```bash
make clean
make
```

### 3. Add your video
Place a video file in the `assets/videos/` directory. The application will automatically detect and play the first video file it finds.

## Configuration

### Inactivity Timeout
The default timeout is 10 seconds. To change it, edit [include/config.h](../../include/config.h):
```c
#define INACTIVITY_TIMEOUT 10000  // Time in milliseconds
```

### Video Directory
To change the video directory, edit [include/config.h](../../include/config.h):
```c
#define VIDEO_DIR "assets/videos"
```

## Troubleshooting

### FFmpeg not found during compilation
**Error:** `Package libavformat was not found`

**Solution:**
```bash
# Run setup script again
./setup.sh

# Or manually install FFmpeg libraries
sudo apt-get update
sudo apt-get install -y libavformat-dev libavcodec-dev libavutil-dev libswscale-dev
```

### No video file found
**Console message:** `Warning: No video files found in assets/videos directory`

**Solution:**
- Verify that a video file exists in `assets/videos/`
- Check that the file has a supported extension (.mp4, .avi, .mkv, .mov, .webm)
- Check file permissions: `ls -la assets/videos/`

### Video not starting after 10 seconds
**Possible causes:**
1. Video file not found or couldn't be loaded
2. FFmpeg not properly linked
3. Video format not supported

**Solution:**
- Check console output for error messages
- Verify FFmpeg libraries are installed: `pkg-config --libs libavformat`
- Try converting the video to MP4 with H.264 codec

### Video playback errors
**Error:** `Failed to set video source`

**Solution:**
- Ensure the video codec is supported by FFmpeg
- Try re-encoding the video with standard settings:
  ```bash
  ffmpeg -i problem_video.mp4 -c:v libx264 -c:a aac assets/videos/fixed_video.mp4
  ```

### Performance issues
If video playback is choppy:
1. Reduce video resolution (use 320x520 or smaller)
2. Lower the video bitrate
3. Use a simpler codec (H.264 instead of H.265)
4. Reduce frame rate to 24 FPS

**Optimize video for performance:**
```bash
ffmpeg -i input.mp4 -vf "scale=320:520" -c:v libx264 -preset ultrafast -crf 28 -r 24 assets/videos/optimized.mp4
```

## Technical Details

### Modified Files
- [lv_conf.h](../../lv_conf.h) - Enabled LV_USE_FFMPEG
- [include/config.h](../../include/config.h) - Added video configuration
- [include/video.h](../../include/video.h) - Video player API
- [src/video.c](../../src/video.c) - FFmpeg-based video playback
- [src/home.c](../../src/home.c) - Inactivity detection and video integration
- [Makefile](../../Makefile) - Added FFmpeg library linking
- [setup.sh](../../setup.sh) - Added FFmpeg dependency installation

### Event Detection
The system detects user activity through:
- `LV_EVENT_PRESSED` - Touch/mouse press
- `LV_EVENT_CLICKED` - Click events

These events reset the inactivity timer and stop video playback if running.

### Video Player API
The video player uses LVGL's FFmpeg integration:
- `lv_ffmpeg_player_create()` - Creates video player widget
- `lv_ffmpeg_player_set_src()` - Sets video source file
- `lv_ffmpeg_player_set_cmd()` - Controls playback (START, PAUSE, SEEK_START)
- `lv_ffmpeg_player_set_auto_restart()` - Enables video looping

### Directory Structure
```
assets/
└── videos/              # Place video files here
    ├── video.mp4        # Example video file
    └── README.md        # This file
```

## Example: Complete Setup

```bash
# 1. Install FFmpeg system tools (for video conversion)
sudo apt-get install ffmpeg

# 2. Convert a sample video
ffmpeg -i ~/Downloads/sample.mp4 -vf "scale=320:520" -c:v libx264 -crf 23 assets/videos/demo.mp4

# 3. Run setup to install development libraries
./setup.sh

# 4. Build the application
make clean
make

# 5. Run the application
./system

# 6. Wait 10 seconds without touching the screen to see the video
```

## Notes
- The video will loop continuously while playing
- Only the first video file found in the directory will be used
- Video playback stops immediately upon any user interaction
- The slideshow resumes when video playback stops
