# LVGL Title Bar Application

A minimal LVGL 8.4 application featuring a transparent title bar at the top of the window that displays real-time date and time information with day of week.

## Features

- **Transparent Title Bar**: Semi-transparent title bar positioned at the top of the window with a dark background
- **Real-time Date/Time Display**: Shows current date and time with day of week
  - Format: `Monday 14:30:45 / 2025-11-27`
  - Updates every second automatically
- **Background Image Support**:
  - JPEG format (via SJPG/TJPGD decoder)
  - PNG format (via LODEPNG decoder)
  - GIF animations
- **Portrait Display Mode**: 320x640 pixel window suitable for mobile/tablet interfaces
- **SDL2 Integration**: Cross-platform rendering using SDL2 backend
- **Multi-format Graphics**: Support for multiple image formats simultaneously

## Dependencies

- **LVGL 8.4**: Light and Versatile Graphics Library
- **SDL2**: Simple DirectMedia Layer for display and input
- **FreeType**: Font engine (used by LVGL for rendering)
- **libjpeg**: JPEG image support
- **libpng**: PNG image support
- **Standard C Library**: For date/time operations

## Image Format Support

The application supports multiple image formats for background images and graphics:

| Format | Decoder | Extension | Status |
|--------|---------|-----------|--------|
| JPEG | SJPG/TJPGD | `.jpg` | ✅ Enabled |
| PNG | LODEPNG | `.png` | ✅ Enabled |
| GIF | GIF Widget | `.gif` | ✅ Enabled |

### Using Images in Your Application

```c
// Load background image
lv_obj_t *bg_img = lv_img_create(screen);
lv_img_set_src(bg_img, "A:assets/background.jpg");  // JPEG
lv_img_set_src(bg_img, "A:assets/background.png");  // PNG

// Create animated GIF
lv_obj_t *gif = lv_gif_create(screen);
lv_gif_set_src(gif, "A:assets/animation.gif");
```

**Note**: All image paths must use the `A:` file system driver prefix for POSIX file system access.

## Installation

### Prerequisites

Install required system packages:

```bash
# Debian/Ubuntu
sudo apt-get install libsdl2-dev libfreetype6-dev libjpeg-dev libpng-dev pkg-config build-essential

# macOS (with Homebrew)
brew install sdl2 freetype libjpeg libpng
```

For **image format support**:
- **JPEG**: Requires `libjpeg` (for SJPG/TJPGD decoder)
- **PNG**: Requires `libpng` (for LODEPNG decoder)
- **GIF**: Built-in to LVGL (no external dependency)

### Build Setup

1. Clone or navigate to the project directory
2. Run the setup script to build LVGL:

```bash
./setup.sh
```

3. Build the application:

```bash
make
```

## Usage

### Running the Application

```bash
make run
```

Or directly:

```bash
./title
```

### Keyboard Controls

- **Esc**: Exit application
- **Close Window**: Exit application

The application runs continuously, updating the date/time display every second.

## Display Configuration

### Window Size

The window is configured for portrait orientation:

```c
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 640
```

Modify these values in [main.c:10-11](main.c#L10-L11) to change the window size.

### Title Bar Appearance

Customize the title bar in the `create_gui()` function:

```c
// Background color and transparency
lv_obj_set_style_bg_color(app_state.title_bar, lv_color_hex(0xFFFFFF), 0);
lv_obj_set_style_bg_opa(app_state.title_bar, 180, 0);  // 70% opacity

// Border styling (blue border)
lv_obj_set_style_border_width(app_state.title_bar, 2, 0);
lv_obj_set_style_border_color(app_state.title_bar, lv_color_hex(0x4A90E2), 0);

// Height
lv_obj_set_size(app_state.title_bar, SCREEN_WIDTH, 60);
```

### Font Configuration

By default, the application uses LVGL's built-in font. If you want to use custom TrueType fonts, uncomment the FreeType initialization in the `init_fonts()` function:

```c
// Uncomment this section in init_fonts() to use custom fonts
if (!lv_freetype_init(0, 0, 0)) {
    fprintf(stderr, "Warning: FreeType initialization failed\n");
}

static lv_ft_info_t info;
info.name = "assets/NotoSansKR-Regular.ttf";
info.weight = 20;
info.style = FT_FONT_STYLE_NORMAL;
if (lv_ft_font_init(&info)) {
    app_state.font_20 = info.font;
}
```

## Makefile Targets

```
make              # Build the application
make run          # Build and run
make clean        # Remove build artifacts
make clean-lvgl   # Clean LVGL build (requires ./setup.sh to rebuild)
make help         # Show help information
```

## Project Structure

```
.
├── main.c           # Main application source code
├── Makefile         # Build configuration
├── lv_conf.h        # LVGL configuration file
├── setup.sh         # Setup script for building LVGL
├── lvgl/            # LVGL library directory
├── assets/          # Font files (optional)
├── README.md        # This file
└── .gitignore       # Git ignore patterns
```

## Architecture

### Application State

The `AppState` structure manages:

```c
typedef struct {
    lv_obj_t *screen;      // Main display screen
    lv_obj_t *title_bar;   // Title bar container
    lv_obj_t *title_label; // Title text label
    lv_font_t *font_20;    // Font for title (if custom fonts used)
} AppState;
```

### Title Bar Update

The title bar updates automatically via an LVGL timer:

```c
lv_timer_create(
    (lv_timer_cb_t)update_title_bar,
    1000,  // 1 second interval
    NULL
);
```

### Date/Time Formatting

The `update_title_bar()` function formats the date/time as:

```
Monday, 2025-11-27  14:30:45
```

Components:
- Day of week (Sunday-Saturday)
- Date in YYYY-MM-DD format
- Time in HH:MM:SS format (24-hour)

## Initialization Flow

1. **SDL2 Initialization** (`init_sdl()`): Creates window, renderer, and texture
2. **LVGL Initialization** (`init_lvgl()`): Sets up display driver and input device
3. **Font Initialization** (`init_fonts()`): Loads fonts (uses LVGL default by default)
4. **GUI Creation** (`create_gui()`): Creates title bar and labels
5. **Timer Setup**: Creates a 1-second update timer for the date/time display
6. **Main Loop**: Handles events and updates LVGL

## Customization Examples

### Change Title Bar Color

```c
// In create_gui(), modify:
lv_obj_set_style_bg_color(app_state.title_bar, lv_color_hex(0xYYYYYY), 0);
```

### Change Opacity

```c
// In create_gui(), modify (0-255, where 255 is fully opaque):
lv_obj_set_style_bg_opa(app_state.title_bar, 200, 0);  // 78% opacity
```

### Change Title Bar Height

```c
// In create_gui(), modify:
lv_obj_set_size(app_state.title_bar, SCREEN_WIDTH, 80);  // 80 pixels high
```

### Change Text Color

```c
// In create_gui(), modify:
lv_obj_set_style_text_color(app_state.title_label, lv_color_hex(0x000000), 0);  // Black
```

## Configuration (lv_conf.h)

### Image Format Support

Configure which image formats to enable:

```c
// PNG support (LODEPNG decoder)
#define LV_USE_PNG 1
#define LV_USE_LODEPNG 1

// JPEG support (SJPG/TJPGD decoder)
#define LV_USE_SJPG 1
#define LV_USE_TJPGD 1

// GIF support
#define LV_USE_GIF 1
```

### Memory Configuration

Adjust memory pool size based on your needs (for image decoding):

```c
// Default: 4MB for JPEG/PNG/GIF decoding
#define LV_MEM_SIZE (4 * 1024 * 1024U)

// Cache size for decoded images
#define LV_CACHE_DEF_SIZE 2048
```

### File System Driver

The application uses POSIX file system with driver letter 'A':

```c
#define LV_USE_FS_POSIX 1
#define LV_FS_POSIX_LETTER 'A'
#define LV_FS_POSIX_PATH ""  // Current working directory
```

## Troubleshooting

### Build Errors

**Missing SDL2 headers**: Ensure SDL2 dev libraries are installed:

```bash
pkg-config --cflags --libs sdl2
```

**Missing FreeType headers**: Ensure FreeType dev libraries are installed:

```bash
pkg-config --cflags --libs freetype2
```

### Runtime Issues

**Window doesn't appear**: Check that SDL2 and your graphics drivers are properly installed.

**Time not updating**: Ensure LVGL timer handler is being called in the main loop (it is by default).

**Image not displaying**:
- Ensure image file exists: `ls -la assets/`
- Verify file path uses `A:` prefix: `"A:assets/image.jpg"`
- Check that required image decoder is enabled in `lv_conf.h`:
  - For JPEG: `#define LV_USE_SJPG 1`
  - For PNG: `#define LV_USE_PNG 1 and LV_USE_LODEPNG 1`
  - For GIF: `#define LV_USE_GIF 1`
- If adding new image formats, rebuild LVGL: `make clean-lvgl && ./setup.sh`

**Out of memory errors when loading images**:
- Increase memory pool in `lv_conf.h`: `#define LV_MEM_SIZE (4 * 1024 * 1024U)`
- Reduce image resolution or switch to smaller file sizes
- Disable unused image format decoders to free memory

## References

- [LVGL Documentation](https://docs.lvgl.io/)
- [SDL2 Documentation](https://wiki.libsdl.org/)
- [FreeType Documentation](https://www.freetype.org/)
- [C Standard Library - time.h](https://cplusplus.com/reference/ctime/time/)

## License

Refer to the LVGL license (typically MIT) and dependent libraries' licenses.

## Version

- **Application**: 1.0
- **LVGL**: 8.4
- **SDL2**: Latest stable
- **Build Date**: 2025-11-27
