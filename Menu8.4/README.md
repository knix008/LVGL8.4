# LVGL Menu Application

A modular LVGL 8.4 application featuring a window-switching menu system with screen navigation stack. The main screen displays real-time date and time information, and a menu window provides access to various application features.

## Features

- **Window Switching System**: Navigate between main screen and menu screen with a stack-based navigation system
- **Real-time Date/Time Display**: Shows current date and time with day of week on the main screen
  - Format: `Wednesday HH:MM:SS\nYYYY-MM-DD`
  - Updates every second automatically
- **Menu Screen**: Dedicated menu window with 4 menu items accessible via "메뉴" (Menu) button
- **Navigation Buttons**:
  - "메뉴" (Menu): Navigate to menu screen
  - "이전" (Previous): Navigate back to home screen
  - "종료" (Exit): Quit the application
- **Korean Text Support**: Full support for Korean language UI elements
- **Background Image Support**:
  - JPEG format (via SJPG/TJPGD decoder)
  - PNG format (via LODEPNG decoder)
  - GIF animations
- **Portrait Display Mode**: 320x640 pixel window suitable for mobile/tablet interfaces
- **SDL2 Integration**: Cross-platform rendering using SDL2 backend
- **Modular Architecture**: Separated into logical modules (home, menu, style, screen, init)

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
lv_img_set_src(bg_img, "A:assets/images/background.jpg");  // JPEG
lv_img_set_src(bg_img, "A:assets/images/background.png");  // PNG

// Create animated GIF
lv_obj_t *gif = lv_gif_create(screen);
lv_gif_set_src(gif, "A:assets/images/animation.gif");
```

**Note**: All image paths must use the `A:` file system driver prefix for POSIX file system access and should reference the `assets/images/` directory.

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

The window is configured for portrait orientation in [include/config.h](include/config.h):

```c
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 640
#define TITLE_BAR_HEIGHT 70
#define STATUS_BAR_HEIGHT 50
```

Modify these values to change the window size and layout. Then rebuild:

```bash
make clean && make
```

### Color Configuration

All colors are defined in [include/config.h](include/config.h):

```c
#define COLOR_BG_DARK 0x1A1A1A        // Dark background
#define COLOR_BG_TITLE 0x2C3E50       // Title bar background
#define COLOR_TEXT 0xFFFFFF           // Text color (white)
#define COLOR_BUTTON_BG 0x3498DB      // Button background
#define COLOR_BUTTON_BACK 0xE74C3C    // Back button background
#define COLOR_BORDER 0x34495E         // Border color
#define COLOR_TRANSPARENT 0x00        // Transparent background
```

### GUI Layout Configuration

All GUI layout parameters are defined as macros in [include/config.h](include/config.h):

```c
// Padding and margins
#define PADDING_HORIZONTAL 10           // Button padding (5px each side)
#define PADDING_VERTICAL 5              // Vertical padding
#define MARGIN_BUTTON 10                // Space between buttons
#define OFFSET_BUTTON_START_Y 20        // Top offset for first menu button

// Label and title widths
#define TITLE_LABEL_WIDTH (SCREEN_WIDTH - 20)      // Title label width (300px)
#define MENU_BUTTON_WIDTH (SCREEN_WIDTH - 20)      // Menu button width (300px)

// Menu configuration
#define MENU_ITEMS_COUNT 4              // Number of menu items
#define MENU_BUTTON_HEIGHT 60           // Menu button height
#define MENU_BUTTON_MARGIN 10           // Space between menu buttons

// Update intervals (milliseconds)
#define UPDATE_INTERVAL_TIMER 1000      // Date/time update frequency
```

This makes it easy to adjust layouts without changing code. For example:
- Change `MENU_ITEMS_COUNT` to add/remove menu items
- Change `MENU_BUTTON_HEIGHT` to make buttons taller/shorter
- Change `PADDING_HORIZONTAL` to add/remove button margins
- Change `UPDATE_INTERVAL_TIMER` to update date/time faster/slower

### Font Configuration

The application uses FreeType with NotoSansKR font, loaded in [src/init.c:32-48](src/init.c#L32-L48):

```c
int init_fonts(void) {
    if (!lv_freetype_init(0, 0, 0)) {
        fprintf(stderr, "Warning: FreeType initialization failed\n");
    }

    static lv_ft_info_t info;
    info.name = "assets/fonts/NotoSansKR-Regular.ttf";
    info.weight = FONT_SIZE;  // Defined in config.h
    info.style = FT_FONT_STYLE_NORMAL;

    if (lv_ft_font_init(&info)) {
        app_state.font_20 = info.font;
    }
    return 0;
}
```

To change the font or size, modify [include/config.h](include/config.h):

```c
#define FONT_SIZE 20                  // Font size in pixels
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
├── src/                 # Source files
│   ├── main.c          # Application entry point and event loop
│   ├── home.c          # Main/home screen implementation
│   ├── menu.c          # Menu screen implementation
│   ├── screen.c        # Screen management and navigation
│   ├── style.c         # UI styling helper functions
│   └── init.c          # SDL2 and LVGL initialization
├── include/            # Header files
│   ├── config.h        # Application configuration and constants
│   ├── types.h         # Data structure definitions
│   ├── screen.h        # Screen management declarations
│   ├── style.h         # Styling function declarations
│   ├── init.h          # Initialization function declarations
│   ├── home.h          # Home screen declarations
│   └── menu.h          # Menu screen declarations
├── assets/             # Font and image files
│   ├── fonts/          # TrueType font files (NotoSansKR-*.ttf)
│   └── images/         # Image assets (background-bikini-woman-big.jpg)
├── lvgl/               # LVGL library directory
├── Makefile            # Build configuration
├── lv_conf.h           # LVGL configuration file
├── setup.sh            # Setup script for building LVGL
├── README.md           # This file
└── .gitignore          # Git ignore patterns
```

## Architecture

### Modular Design

The application is organized into focused modules:

- **main.c**: Entry point, global state, and event loop
- **home.c**: Main/home screen with date/time display
- **menu.c**: Menu screen with navigation buttons and menu items
- **screen.c**: Screen navigation and stack management
- **style.c**: Reusable UI styling helper functions
- **init.c**: SDL2 and LVGL initialization

### Application State

The `AppState` structure manages global GUI state:

```c
typedef struct {
    lv_obj_t *screen;      // Main display screen
    lv_obj_t *title_bar;   // Title bar container
    lv_obj_t *title_label; // Title text label
    lv_font_t *font_20;    // Font for text rendering
} AppState;
```

### Screen Navigation

The `ScreenState` structure tracks each screen in the navigation stack:

```c
typedef struct {
    lv_obj_t *screen;      // Screen object
    int screen_id;         // Screen identifier (SCREEN_MAIN, SCREEN_MENU)
} ScreenState;
```

A stack-based navigation system supports up to 10 screens (configurable):
- `screen_stack[MAX_SCREENS]`: Array of screens
- `screen_stack_top`: Current position in the stack
- `show_screen()`: Navigate to a screen by ID
- `back_btn_callback()`: Return to previous screen

### Screen IDs

```c
#define SCREEN_MAIN 0      // Home screen
#define SCREEN_MENU 1      // Menu screen
```

### Real-time Date/Time Display

The home screen title bar updates automatically via an LVGL timer:

```c
lv_timer_create(
    (lv_timer_cb_t)update_title_bar,
    1000,  // 1 second interval
    NULL
);
```

Date/time format:
```
Wednesday
14:30:45
2025-11-28
```

Components:
- Day of week (Sunday-Saturday)
- Time in HH:MM:SS format (24-hour)
- Date in YYYY-MM-DD format

## Initialization Flow

1. **SDL2 Initialization** (`init_sdl()` in init.c): Creates window, renderer, and texture
2. **LVGL Initialization** (`init_lvgl()` in init.c): Sets up display driver and input device
3. **Font Initialization** (`init_fonts()` in init.c): Loads FreeType fonts (NotoSansKR)
4. **GUI Creation** (`create_gui()` in home.c): Creates home screen with title bar and status bar
5. **Timer Setup**: Creates 1-second update timer for date/time display
6. **Screen Stack Initialize**: Home screen added to screen stack at position 0
7. **Main Loop** (in main.c):
   - Handles SDL events (quit, ESC key)
   - Updates LVGL tick counter
   - Calls LVGL timer handler for screen updates
   - 5ms frame delay for smooth rendering

## Customization Examples

### Change Title Bar Color

Edit [src/home.c:27](src/home.c#L27) in `create_main_title_bar()`:

```c
apply_bar_style(app_state.title_bar, 0x2C3E50);  // New color value
```

### Change Menu Button Label

Edit [src/menu.c:31](src/menu.c#L31) in `create_menu_title_bar()`:

```c
lv_label_set_text(title_label, "메인 메뉴");  // New title
```

### Add More Menu Items

Edit [src/menu.c:59-64](src/menu.c#L59-L64) in `create_menu_content()`:

```c
int menu_items = 6;  // Change from 4 to 6
const char *menu_labels[] = {
    "메뉴 1", "메뉴 2", "메뉴 3", "메뉴 4", "메뉴 5", "메뉴 6"
};
int button_width = SCREEN_WIDTH - 20;   // Width with 10px padding on each side
int button_height = 60;                 // Button height
int button_margin = 10;                 // Space between buttons
```

The menu buttons have built-in padding. You can adjust:
- `button_width`: Change the `- 20` value to add/remove padding (currently 10px on each side)
- `button_height`: Taller/shorter buttons (currently 60px)
- `button_margin`: Space between buttons (currently 10px)
- `start_y`: Vertical offset of the first button (currently 20px)

### Change Screen Dimensions

Edit [include/config.h](include/config.h):

```c
#define SCREEN_WIDTH 480   // From 320
#define SCREEN_HEIGHT 800  // From 640
```

Then rebuild: `make clean && make`

### Change Update Frequency

Edit [src/home.c:65](src/home.c#L65):

```c
lv_timer_create((lv_timer_cb_t)update_title_bar, 500, NULL);  // Update every 500ms instead of 1000ms
```

### Customize Button Styles

Edit styling helper functions in [src/style.c](src/style.c):

```c
void apply_button_style(lv_obj_t *btn, uint32_t bg_color) {
    lv_obj_set_style_bg_color(btn, lv_color_hex(bg_color), 0);
    lv_obj_set_style_border_width(btn, 2, 0);  // Change border width
    lv_obj_set_style_border_color(btn, lv_color_hex(0xFF6B6B), 0);  // New border color
}
```

### Adjust Menu Button Layout

Edit [src/menu.c:61-64](src/menu.c#L61-L64) in `create_menu_content()`:

```c
int button_width = SCREEN_WIDTH - 20;   // Width (320 - 20 = 300, with 10px padding each side)
int button_height = 60;                 // Current height (adjust to 50, 70, etc.)
int button_margin = 10;                 // Space between buttons (adjust to 5, 15, etc.)
int start_y = 20;                       // Top offset of first button (adjust to 10, 30, etc.)
```

Menu button layout options:
- **Increase padding**: Change `SCREEN_WIDTH - 20` to `SCREEN_WIDTH - 30` (15px padding each side)
- **Decrease padding**: Change `SCREEN_WIDTH - 20` to `SCREEN_WIDTH - 10` (5px padding each side)
- **Adjust button height**: Change 60 to 50, 70, 80, etc.
- **Adjust spacing**: Change 10 to 5, 15, 20 for tighter/looser spacing
- **Adjust top offset**: Change 20 to move buttons down/up

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

**Compilation errors in src/ files**: Ensure all include paths are correct (should use `../include/` relative paths). Check the Makefile references `SRC_DIR = src` and `$(SRC_DIR)/*.o` in object file rules.

**Missing SDL2 headers**: Ensure SDL2 dev libraries are installed:

```bash
pkg-config --cflags --libs sdl2
```

**Missing FreeType headers**: Ensure FreeType dev libraries are installed:

```bash
pkg-config --cflags --libs freetype2
```

**Linker errors**: Verify LVGL library is built:

```bash
ls -la lvgl/build/liblvgl.a
```

If missing, rebuild LVGL:

```bash
./setup.sh
```

### Runtime Issues

**Window doesn't appear**: Check that SDL2 and your graphics drivers are properly installed.

**Menu button doesn't work**: Verify the event callback is registered in [src/home.c:88](src/home.c#L88):

```c
lv_obj_add_event_cb(menu_btn, menu_btn_callback, LV_EVENT_CLICKED, NULL);
```

**Back button doesn't work**: Verify screen stack navigation in [src/menu.c:46](src/menu.c#L46):

```c
lv_obj_add_event_cb(back_btn, back_btn_callback, LV_EVENT_CLICKED, NULL);
```

**Time not updating**: Ensure LVGL timer handler is being called in the main loop [src/main.c:70](src/main.c#L70):

```c
lv_timer_handler();
```

**Korean text not displaying**: Verify NotoSansKR font file exists:

```bash
ls -la assets/fonts/NotoSansKR-Regular.ttf
```

Check FreeType initialization in [src/init.c](src/init.c) is successful. Font load warnings are logged to stderr.

**Image not displaying**:
- Ensure image file exists: `ls -la assets/images/`
- Verify file path uses `A:` prefix: `"A:assets/images/image.jpg"`
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

- **Application**: 2.0 (Modular refactoring with menu system)
- **LVGL**: 8.4
- **SDL2**: Latest stable
- **FreeType**: Latest stable
- **Last Updated**: 2025-11-28

### Changelog

#### v2.0 (2025-11-28)
- Refactored monolithic main.c into modular components
- Implemented window-switching menu system with screen stack navigation
- Reorganized project structure: src/ and include/ directories
- Added Korean language support with NotoSansKR font
- Enhanced UI with separate home and menu screens
- Updated Makefile for new directory structure

#### v1.0
- Initial LVGL title bar application with real-time date/time display
