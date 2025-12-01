# LVGL Menu Application

A modern LVGL 8.4 application featuring a hierarchical menu system with breadcrumb navigation, icon-based status bar, and absolute path navigation support.

## Features

- **Hierarchical Navigation System**: Breadcrumb-style navigation showing current location path
  - Format: `홈 > 메뉴 > 관리자 설정`
  - Automatically updates based on navigation stack
- **Absolute Path Navigation**: Status bar buttons use absolute paths instead of stacking
  - Click any status bar button to navigate directly to that screen
  - Navigation path always shows: `홈 > 메뉴 > [Target Screen]`
- **Icon-Based Menu System**: Modern UI with icon + label buttons
  - Menu buttons display icons on the left with text labels on the right
  - Status bar features circular icon buttons for quick navigation
- **Real-time Date/Time Display**: Shows current date and time with day of week on the main screen
  - Format: `Wednesday 14:30:45 2025-11-29`
  - Updates every second automatically
- **Multiple Screens**: Navigate between different application sections
  - Main Screen (홈): Home screen with background image and date/time
  - Menu Screen (메뉴): Main menu with icon buttons
  - Info Screen (정보): Application information
  - Admin Screen (관리자 설정): Admin settings
  - Network Screen (네트워크 설정): Network configuration
- **Navigation Controls**:
  - Back Button: Navigate back through the hierarchy
  - Status Bar Icons: Quick access to all major screens
  - Menu Button: Navigate to menu from home screen
- **Korean Text Support**: Full support for Korean language UI elements
- **Background Image Support**:
  - JPEG format (via SJPG/TJPGD decoder)
  - PNG format (via LODEPNG decoder)
  - GIF animations
- **Portrait Display Mode**: 320x640 pixel window suitable for mobile/tablet interfaces
- **SDL2 Integration**: Cross-platform rendering using SDL2 backend
- **Modular Architecture**: Separated into logical modules (home, menu, info, admin, network, screen, style)

## Navigation System

### Breadcrumb Navigation

The title bar displays a breadcrumb path showing your current location:

```
홈 > 메뉴 > 관리자 설정
```

The breadcrumb updates automatically as you navigate through screens.

### Absolute Path Navigation

Status bar buttons use absolute path navigation:

- Clicking a status bar button resets the navigation stack
- Always creates path: `홈 > 메뉴 > [Target Screen]`
- Example: Clicking "Info" from "Admin" goes: `홈 > 메뉴 > 정보`

This prevents deep navigation stacking and provides consistent breadcrumb paths.

### Back Button Navigation

The back button navigates backward through the hierarchy:

- From `홈 > 메뉴 > 정보` → back → `홈 > 메뉴`
- From `홈 > 메뉴` → back → `홈`

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

// Create icon buttons
lv_obj_t *img = lv_img_create(btn);
lv_img_set_src(img, "A:assets/images/config.png");
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
#define TITLE_BAR_HEIGHT 60
#define STATUS_BAR_HEIGHT 60
```

Modify these values to change the window size and layout. Then rebuild:

```bash
make clean && make
```

### Color Configuration

All colors are defined in [include/config.h](include/config.h):

```c
#define COLOR_BG_DARK 0x2A2A2A        // Dark background
#define COLOR_BG_TITLE 0x1A1A1A       // Title/status bar background
#define COLOR_BUTTON_BG 0x1A1A1A      // Button background
#define COLOR_BUTTON_BACK 0x444444    // Back button background
#define COLOR_BORDER 0x888888         // Border color
#define COLOR_TEXT 0xFFFFFF           // Text color (white)
#define COLOR_TRANSPARENT 128         // Transparent background
```

### GUI Layout Configuration

All GUI layout parameters are defined as macros in [include/config.h](include/config.h):

```c
// Padding and margins
#define PADDING_HORIZONTAL 10           // Horizontal padding
#define PADDING_VERTICAL 5              // Vertical padding
#define PADDING_BUTTON 20               // Button padding
#define MARGIN_BUTTON 10                // Space between buttons
#define OFFSET_BUTTON_START_Y 20        // Top offset for first menu button

// Label and title widths
#define TITLE_LABEL_WIDTH (SCREEN_WIDTH - 20)      // Title label width
#define MENU_BUTTON_WIDTH (SCREEN_WIDTH - 20)      // Menu button width

// Menu configuration
#define MENU_ITEMS_COUNT 4              // Number of menu items
#define MENU_BUTTON_HEIGHT 60           // Menu button height
#define MENU_BUTTON_MARGIN 10           // Space between menu buttons

// Update intervals (milliseconds)
#define UPDATE_INTERVAL_TIMER 1000      // Date/time update frequency

// Frame timing (milliseconds)
#define FRAME_DELAY_MS 1                // Main loop delay
```

### Image Paths

Icon images are configured in [include/config.h](include/config.h):

```c
#define IMG_BACK_BUTTON "A:assets/images/backbutton.png"
#define IMG_CONFIG "A:assets/images/config.png"
#define IMG_SETUP "A:assets/images/setup.png"
#define IMG_INFO "A:assets/images/Info.png"
#define IMG_NETWORK "A:assets/images/network.png"
```

### Font Configuration

The application uses FreeType with NotoSansKR font. Font size is configured in [include/config.h](include/config.h):

```c
#define FONT_SIZE 16                  // Font size in pixels
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
│   ├── info.c          # Info screen implementation
│   ├── admin.c         # Admin settings screen implementation
│   ├── network.c       # Network settings screen implementation
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
│   ├── menu.h          # Menu screen declarations
│   ├── info.h          # Info screen declarations
│   ├── admin.h         # Admin screen declarations
│   └── network.h       # Network screen declarations
├── assets/             # Asset files
│   ├── fonts/          # TrueType font files (NotoSansKR-*.ttf)
│   └── images/         # Image assets (PNG icons, backgrounds)
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
- **menu.c**: Menu screen with icon buttons
- **info.c**: Info screen with application information
- **admin.c**: Admin settings screen
- **network.c**: Network settings screen
- **screen.c**: Screen navigation and breadcrumb management
- **style.c**: Reusable UI styling helper functions
- **init.c**: SDL2 and LVGL initialization

### Application State

The `AppState` structure manages global GUI state:

```c
typedef struct {
    lv_obj_t *screen;             // Main display screen
    lv_obj_t *title_bar;          // Title bar container
    lv_obj_t *title_label;        // Title text label (main screen)
    lv_obj_t *current_title_label; // Current screen title label
    lv_font_t *font_20;           // Font for text rendering
} AppState;
```

### Screen Navigation

The `ScreenState` structure tracks each screen in the navigation stack:

```c
typedef struct {
    lv_obj_t *screen;      // Screen object
    int screen_id;         // Screen identifier
} ScreenState;
```

Navigation stack:
- `screen_stack[MAX_SCREENS]`: Array of screens (up to 10)
- `screen_stack_top`: Current position in the stack
- `show_screen()`: Navigate to a screen by ID
- `update_title_bar_location()`: Update breadcrumb path

### Screen IDs

```c
enum {
    SCREEN_MAIN = 0,      // Home screen
    SCREEN_MENU = 1,      // Menu screen
    SCREEN_INFO = 2,      // Info screen
    SCREEN_ADMIN = 3,     // Admin settings screen
    SCREEN_NETWORK = 4    // Network settings screen
};
```

### Navigation Callbacks

**Absolute Path Navigation** (Status Bar Buttons):
```c
static void info_btn_callback(lv_event_t *e) {
    if (screen_stack[screen_stack_top].screen_id != SCREEN_INFO) {
        screen_stack_top = 0;          // Reset to MAIN
        show_screen(SCREEN_MENU);      // Go through MENU
        show_screen(SCREEN_INFO);      // Then to INFO
    }
}
```

**Hierarchical Navigation** (Back Button):
```c
static void back_btn_callback(lv_event_t *e) {
    if (screen_stack_top > 0) {
        screen_stack_top--;
        show_screen(screen_stack[screen_stack_top].screen_id);
    }
}
```

### Real-time Date/Time Display

The home screen title bar updates automatically via an LVGL timer:

```c
lv_timer_create(
    (lv_timer_cb_t)update_title_bar,
    UPDATE_INTERVAL_TIMER,  // 1000ms = 1 second
    NULL
);
```

Date/time format:
```
Wednesday 14:30:45
2025-11-29
```

## UI Components

### Status Bar Icons

Each screen has a status bar with 4 circular icon buttons:

1. **Config** (IMG_CONFIG): Navigate to admin settings
2. **Setup** (IMG_SETUP): Navigate to admin settings
3. **Info** (IMG_INFO): Navigate to info screen
4. **Network** (IMG_NETWORK): Navigate to network settings

Button configuration:
- Size: 40x40 pixels
- Spacing: 10 pixels
- Style: Circular with `apply_circle_button_style()`

### Menu Buttons

Menu screen buttons display icons with text labels:

```c
// Icon on left (10px from edge)
lv_obj_t *img = lv_img_create(btn);
lv_img_set_src(img, menu_images[i]);
lv_obj_align(img, LV_ALIGN_LEFT_MID, 10, 0);

// Label on right (60px from edge)
lv_obj_t *label = lv_label_create(btn);
lv_label_set_text(label, menu_labels[i]);
lv_obj_align(label, LV_ALIGN_LEFT_MID, 60, 0);
```

Menu items:
- 관리자 설정 (Admin Settings) - config.png
- 네트워크 설정 (Network Settings) - network.png
- 메뉴 3 (Menu 3) - setup.png
- Info - Info.png

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
   - 1ms frame delay for smooth rendering

## Customization Examples

### Add New Menu Item

Edit [src/menu.c](src/menu.c):

```c
// 1. Update menu item count in config.h
#define MENU_ITEMS_COUNT 5

// 2. Add new label and image
const char *menu_labels[] = {
    "관리자 설정", "네트워크 설정", "메뉴 3", "Info", "새 메뉴"
};
const char *menu_images[] = {
    IMG_CONFIG, IMG_NETWORK, IMG_SETUP, IMG_INFO, IMG_NEW_ICON
};

// 3. Add event handler
if (i == 4) {
    lv_obj_add_event_cb(btn, new_menu_callback, LV_EVENT_CLICKED, NULL);
}
```

### Add New Screen

1. Create new screen files: `src/newscreen.c` and `include/newscreen.h`
2. Add screen ID in [include/config.h](include/config.h):
```c
enum {
    SCREEN_MAIN = 0,
    SCREEN_MENU = 1,
    SCREEN_INFO = 2,
    SCREEN_ADMIN = 3,
    SCREEN_NETWORK = 4,
    SCREEN_NEW = 5       // New screen
};
```
3. Implement screen creation function following the pattern in existing screens
4. Add navigation callback and status bar button

### Change Status Bar Layout

Edit status bar creation functions to adjust icon positions:

```c
int img_btn_size = 40;     // Change icon size
int spacing = 10;          // Change spacing between icons
int start_x = PADDING_HORIZONTAL;  // Change left margin
```

### Customize Breadcrumb Display

Edit [src/screen.c](src/screen.c) in `update_title_bar_location()`:

```c
// Change separator
strncat(breadcrumb, " → ", sizeof(breadcrumb) - strlen(breadcrumb) - 1);

// Change screen names
case SCREEN_INFO:
    name = "App Info";  // English instead of Korean
    break;
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

Adjust memory pool size based on your needs:

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

**Compilation errors**: Ensure all include paths are correct. Check the Makefile references `SRC_DIR = src`.

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

**Navigation not working**: Verify event callbacks are registered correctly.

**Breadcrumb not updating**: Check `update_title_bar_location()` is called after screen creation.

**Icons not displaying**:
- Ensure image files exist: `ls -la assets/images/`
- Verify file paths use `A:` prefix
- Check image decoder is enabled in `lv_conf.h`

**Korean text not displaying**: Verify NotoSansKR font file exists:
```bash
ls -la assets/fonts/NotoSansKR-Regular.ttf
```

## References

- [LVGL Documentation](https://docs.lvgl.io/)
- [SDL2 Documentation](https://wiki.libsdl.org/)
- [FreeType Documentation](https://www.freetype.org/)
- [C Standard Library - time.h](https://cplusplus.com/reference/ctime/time/)

## License

Refer to the LVGL license (typically MIT) and dependent libraries' licenses.

## Version

- **Application**: 3.0 (Icon-based UI with absolute path navigation)
- **LVGL**: 8.4
- **SDL2**: Latest stable
- **FreeType**: Latest stable
- **Last Updated**: 2025-11-29

### Changelog

#### v3.0 (2025-11-29)
- Implemented icon-based menu system with icon + label buttons
- Added absolute path navigation for status bar buttons
- Implemented breadcrumb navigation showing hierarchical path
- Added circular icon buttons in status bar (40x40px)
- Created Info, Admin, and Network screens
- Enhanced menu buttons with left-aligned icons and labels
- Updated navigation system to prevent deep stacking

#### v2.0 (2025-11-28)
- Refactored monolithic main.c into modular components
- Implemented window-switching menu system with screen stack navigation
- Reorganized project structure: src/ and include/ directories
- Added Korean language support with NotoSansKR font
- Enhanced UI with separate home and menu screens
- Updated Makefile for new directory structure

#### v1.0
- Initial LVGL title bar application with real-time date/time display
