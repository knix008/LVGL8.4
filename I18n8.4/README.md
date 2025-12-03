# LVGL Menu Application with Korean Input

A modern LVGL 8.4 application featuring a hierarchical menu system with breadcrumb navigation, icon-based status bar, absolute path navigation support, Chunjiin Korean input method, and internationalization support with dynamic welcome messages.

> **Note**: For details about recent code quality improvements and refactoring, see [REFACTORING_NOTES.md](REFACTORING_NOTES.md) and [REFACTORING_PHASE1_COMPLETE.md](REFACTORING_PHASE1_COMPLETE.md)

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
- **Persistent Status Bar Configuration**: Status bar icons are saved and restored automatically
  - Configuration stored in `config/config.yaml`
  - Toggle icons on/off using +/− buttons in the menu
  - Settings persist across application restarts
- **Real-time Date/Time Display**: Shows current date and time with day of week on the main screen
  - Format: `Wednesday 14:30:45 2025-11-29`
  - Updates every second automatically
- **Dynamic Welcome Messages**: Time-based and language-aware greeting messages on the home screen
  - Displays different messages based on time of day (morning, afternoon, evening, night)
  - Supports multiple languages (Korean and English)
  - **Color Animation**: Welcome message cycles through vibrant colors every 5 seconds
    - White → Pink → Red-Pink → Gold → Cyan → Green (repeating)
  - Large 30px bold font for visibility
  - Positioned in the upper 1/3 of the screen
  - Auto-updates every 60 seconds based on current time period
  - Configured via `config/welcome.json`
- **Multiple Screens**: Navigate between different application sections
  - Main Screen (홈): Home screen with background image and date/time
  - Menu Screen (메뉴): Main menu with icon buttons
  - Info Screen (정보): Application information
  - Admin Screen (관리자 설정): Admin settings
  - Network Screen (네트워크 설정): Network configuration
  - Korean Input Screen (한글 입력): Chunjiin Korean text input method
- **Korean Input Method**: Full Chunjiin (천지인) implementation
  - Mode switching: Hangul (한글), English (upper/lower), Numbers, Special characters
  - 12-button keyboard layout matching standard Chunjiin input
  - Real-time text composition and display
  - Support for consonants, vowels, and syllable combination
  - Enter button to display input result in popup and clear text area
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
- **Modular Architecture**: Component-based architecture with reusable UI components and centralized navigation

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

## Status Bar Configuration

The status bar icon configuration is automatically saved and restored using YAML files.

### Configuration File

Location: `config/config.yaml`

Example configuration:
```yaml
admin: true          # Admin settings icon
network: false       # Network settings icon
korean_input: true   # Korean input icon
info: true           # Info icon
face: false          # Face icon
```

### How to Use

1. **Open the Menu**: Click the menu button from the home screen
2. **Toggle Icons**: Use the +/− buttons next to each menu item
   - **Plus (+)**: Icon not in status bar (click to add)
   - **Minus (−)**: Icon in status bar (click to remove)
3. **Automatic Save**: Changes are saved immediately to the YAML file
4. **Persistent**: Configuration is restored when you restart the application

### Manual Editing

You can also manually edit `config/config.yaml`:

```yaml
admin: false
network: true
korean_input: false
info: true
```

Changes take effect on next application start.

### Technical Details

- Configuration is loaded during initialization in `src/main.c`
- Configuration is saved automatically when icons are toggled in `src/menu.c`
- Implementation in `src/config.c` using libyaml parser
- Config directory is created automatically if it doesn't exist
- If no config file exists, all icons start as disabled

## Welcome Message Configuration

The welcome message system displays time-based and language-specific greeting messages on the home screen.

### Configuration File

Location: `config/welcome.json`

Example configuration:
```json
{
  "ko": {
    "morning": "좋은 아침입니다! 좋은 하루 되세요.",
    "afternoon": "좋은 오후입니다!",
    "evening": "좋은 저녁입니다!",
    "night": "안녕하세요!"
  },
  "en": {
    "morning": "Good morning! Have a great day.",
    "afternoon": "Good afternoon!",
    "evening": "Good evening!",
    "night": "Hello!"
  }
}
```

### Time Periods

Welcome messages are displayed based on the current hour:

| Period | Hours | Message Type |
|--------|-------|--------------|
| Morning | 5:00 - 11:59 | morning |
| Afternoon | 12:00 - 17:59 | afternoon |
| Evening | 18:00 - 21:59 | evening |
| Night | 22:00 - 4:59 | night |

### Color Animation

The welcome message cycles through 6 vibrant colors every 5 seconds:
1. **White** (0xFFFFFF) - Default
2. **Pink** (0xFF6B9D) - Soft pink
3. **Red-Pink** (0xC44569) - Deep red
4. **Gold** (0xF8B500) - Golden yellow
5. **Cyan** (0x00D4FF) - Bright cyan
6. **Green** (0x00FF88) - Fresh green

### Font and Positioning

- **Font**: NotoSansKR-Bold 30pt
- **Alignment**: Horizontally centered
- **Position**: Vertically in upper 1/3 of screen (y = 150px)
- **Container Height**: 120px with text wrap support
- **Background**: Transparent

### How to Customize

1. **Edit Welcome Messages**: Modify `config/welcome.json` with your own messages
2. **Change Time Periods**: Edit `WELCOME_MORNING_START_HOUR`, etc. in `include/config.h`
3. **Adjust Colors**: Modify the `welcome_colors[]` array in `src/home.c` or update constants in `include/config.h`
4. **Change Update Interval**: Set `WELCOME_MESSAGE_UPDATE_INTERVAL` (milliseconds) in `include/config.h`
5. **Adjust Color Speed**: Set `WELCOME_COLOR_UPDATE_INTERVAL` (milliseconds) in `include/config.h`

### Implementation Details

- Configuration is loaded via `welcome_load()` in `src/welcome.c`
- Time-based message selection via `welcome_get_message()` in `src/welcome.c`
- Color animation callback `welcome_color_timer_callback()` in `src/home.c`
- Timers created during home screen initialization in `src/home.c`
- All configuration values are constants defined in `include/config.h`

## Dependencies

- **LVGL 8.4**: Light and Versatile Graphics Library
- **SDL2**: Simple DirectMedia Layer for display and input
- **FreeType**: Font engine (used by LVGL for rendering)
- **libjpeg**: JPEG image support
- **libpng**: PNG image support
- **libyaml**: YAML parser for configuration files
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
sudo apt-get install libsdl2-dev libfreetype6-dev libjpeg-dev libpng-dev libyaml-dev pkg-config build-essential

# macOS (with Homebrew)
brew install sdl2 freetype libjpeg libpng libyaml
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
./input
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
#define IMG_KOREAN "A:assets/images/korean.png"
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
├── src/                     # Source files
│   ├── main.c              # Application entry point and event loop
│   ├── home.c              # Main/home screen implementation
│   ├── menu.c              # Menu screen implementation with centralized configuration
│   ├── info.c              # Info screen implementation
│   ├── admin.c             # Admin settings screen implementation
│   ├── network.c           # Network settings screen implementation
│   ├── korean.c            # Korean input screen implementation
│   ├── face.c              # Face screen implementation
│   ├── chunjiin.c          # Chunjiin input method core logic
│   ├── keypad.c            # Hangul syllable composition logic
│   ├── screen.c            # Screen management, navigation stack, and UI components
│   ├── navigation.c        # Centralized navigation callbacks
│   ├── config.c            # Configuration file management (YAML)
│   ├── style.c             # UI styling helper functions
│   └── init.c              # SDL2 and LVGL initialization
├── include/                # Header files
│   ├── config.h            # Application configuration and constants
│   ├── types.h             # Data structure definitions and menu configuration
│   ├── screen.h            # Screen management and UI component declarations
│   ├── navigation.h        # Navigation callback declarations
│   ├── style.h             # Styling function declarations
│   ├── init.h              # Initialization function declarations
│   ├── home.h              # Home screen declarations
│   ├── menu.h              # Menu screen declarations
│   ├── info.h              # Info screen declarations
│   ├── admin.h             # Admin screen declarations
│   ├── network.h           # Network screen declarations
│   ├── korean.h            # Korean input screen declarations
│   ├── face.h              # Face screen declarations
│   └── chunjiin.h          # Chunjiin input method declarations
├── assets/              # Asset files
│   ├── fonts/           # TrueType font files (NotoSansKR-*.ttf)
│   └── images/          # Image assets (PNG icons, backgrounds)
├── lvgl/                # LVGL library directory
├── Makefile             # Build configuration
├── lv_conf.h            # LVGL configuration file
├── setup.sh             # Setup script for building LVGL
├── README.md            # This file
└── .gitignore           # Git ignore patterns
```

## Architecture

### Component-Based Design

The application follows a component-based architecture with clear separation of concerns:

**Core Framework:**
- **main.c**: Application entry point, global state, and event loop
- **init.c**: SDL2 and LVGL initialization
- **screen.c**: Screen navigation stack, breadcrumb management, and reusable UI components
- **navigation.c**: Centralized navigation callbacks for consistent behavior
- **config.c**: Configuration file management (YAML load/save)
- **style.c**: Reusable UI styling helper functions

**Screen Implementations:**
- **home.c**: Main/home screen with date/time display
- **menu.c**: Menu screen with centralized menu item configuration (MENU_ITEMS array)
- **info.c**: Info screen with application information
- **admin.c**: Admin settings screen
- **network.c**: Network settings screen
- **korean.c**: Korean input screen with Chunjiin keyboard
- **face.c**: Face screen

**Korean Input System:**
- **chunjiin.c**: Chunjiin input method core logic (button handling, mode switching)
- **keypad.c**: Hangul syllable composition and character mapping

### Design Benefits

The refactored architecture provides:
- **Code Reusability**: Common components (title bars, status bars) defined once and reused across all screens
- **Centralized Navigation**: All navigation callbacks in one place for consistent behavior
- **Reduced Duplication**: ~500+ lines of duplicate code eliminated (20% → 0% duplication)
- **Maintainability**: Changes to common UI elements made in one location
- **Clean Screen Implementations**: Screen files focus only on unique content, not boilerplate

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
    SCREEN_MAIN = 0,         // Home screen
    SCREEN_MENU = 1,         // Menu screen
    SCREEN_INFO = 2,         // Info screen
    SCREEN_ADMIN = 3,        // Admin settings screen
    SCREEN_NETWORK = 4,      // Network settings screen
    SCREEN_KOREAN_INPUT = 5, // Korean input screen
    SCREEN_FACE = 6          // Face screen
};
```

### Navigation Callbacks

All navigation callbacks are centralized in [src/navigation.c](src/navigation.c):

**Absolute Path Navigation** (Status Bar Buttons):
```c
void info_btn_callback(lv_event_t *e) {
    (void)e;
    if (screen_stack[screen_stack_top].screen_id != SCREEN_INFO) {
        screen_stack_top = 0;          // Reset to MAIN
        show_screen(SCREEN_MENU);      // Go through MENU
        show_screen(SCREEN_INFO);      // Then to INFO
    }
}
```

**Hierarchical Navigation** (Back Button):
```c
void back_btn_callback(lv_event_t *e) {
    (void)e;
    if (screen_stack_top > 0) {
        screen_stack_top--;
        show_screen(screen_stack[screen_stack_top].screen_id);
    }
}
```

All screens use these shared navigation callbacks, eliminating duplication.

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

### Reusable Screen Components

The application provides standardized screen components in [src/screen.c](src/screen.c):

**Standard Title Bar** (`create_standard_title_bar`):
- Displays breadcrumb navigation path
- Includes back button with icon
- Auto-updates based on navigation stack
- Consistent across all screens

**Standard Status Bar** (`create_standard_status_bar`):
- Circular icon buttons for quick navigation (configurable)
- Icons dynamically loaded based on user configuration
- Absolute path navigation to screens
- Uniform styling and layout

**Standard Content Area** (`create_standard_content`):
- Pre-configured content container
- Proper sizing accounting for title/status bars
- Dark background and consistent styling
- Scrollable by default

**Screen Base Creation** (`create_screen_base`):
- Creates base screen object
- Registers in screens array
- Sets default styling
- Returns screen object for customization

**Screen Finalization** (`finalize_screen`):
- Updates navigation stack
- Sets as active screen
- Updates breadcrumb path
- Completes screen creation

### Centralized Menu Configuration

Menu items are configured in a single location using the `MENU_ITEMS` array in [src/menu.c](src/menu.c):

```c
const MenuItem MENU_ITEMS[MAX_STATUS_ICONS] = {
    {"관리자 설정", IMG_CONFIG, "admin", SCREEN_ADMIN, admin_btn_callback},
    {"네트워크 설정", IMG_NETWORK, "network", SCREEN_NETWORK, network_btn_callback},
    {"한글 입력", IMG_KOREAN, "korean_input", SCREEN_KOREAN_INPUT, korean_input_btn_callback},
    {"Info", IMG_INFO, "info", SCREEN_INFO, info_btn_callback},
    {"Face", IMG_FACE, "face", SCREEN_FACE, settings_btn_callback}
};
```

Each menu item contains:
- **label**: Display text shown in menu
- **icon_path**: Path to icon image
- **config_key**: Key used in YAML configuration file
- **screen_id**: Target screen identifier
- **callback**: Navigation callback function

This configuration is automatically used by:
- Menu screen for button creation
- Status bar for icon management
- Configuration save/load system
- Navigation callbacks

**Example Screen Implementation:**
```c
void create_admin_screen(void) {
    lv_obj_t *admin_screen = create_screen_base(SCREEN_ADMIN);

    create_standard_title_bar(admin_screen, SCREEN_ADMIN);
    create_admin_content(admin_screen);  // Screen-specific content only
    create_standard_status_bar(admin_screen);

    finalize_screen(admin_screen, SCREEN_ADMIN);
}
```

This component-based approach reduced screen implementations by 70-76%, with most screens now under 60 lines.

### Status Bar Icons

Each screen (except home) has a status bar with 4 circular icon buttons:

1. **Config** (IMG_CONFIG): Navigate to admin settings
2. **Korean** (IMG_KOREAN): Navigate to Korean input screen
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
- 한글 입력 (Korean Input) - korean.png
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

## Korean Input (Chunjiin Method)

The application includes a complete implementation of the Chunjiin (천지인) Korean input method, which is a popular text input system used on mobile devices.

### Chunjiin Keyboard Layout

The keyboard follows the standard Chunjiin layout with 12 buttons arranged in a 3x4 grid:

```
Row 0:  천(1)   지(2)   인(3)
Row 1:  ㄱ(4)   ㄴ(5)   ㄷ(6)
Row 2:  ㅂ(7)   ㅅ(8)   ㅈ(9)
Row 3: 공백(10) ㅇㅁ(0) 삭제(11)
```

### Input Modes

The Korean input screen supports multiple input modes accessible via the "모드" (Mode) button:

1. **한글 (Hangul)**: Korean text input using Chunjiin method
   - Consonants: ㄱ, ㄴ, ㄷ, ㅂ, ㅅ, ㅈ, ㅇㅁ
   - Vowels: 천(ㆍ), 지(ㅡ), 인(ㅣ) combined to form all Korean vowels
   - Automatic syllable composition

2. **영문(대) (Upper English)**: Uppercase English letters (A-Z)

3. **영문(소) (Lower English)**: Lowercase English letters (a-z)

4. **숫자 (Numbers)**: Numeric input (0-9)

5. **특수문자 (Special Characters)**: Punctuation and special symbols

### Features

- **Real-time text composition**: Characters are composed as you type
- **Mode indicator**: Shows current input mode at the top of screen
- **Text display area**: 100px height display with text wrapping
- **Centered keyboard**: Horizontally centered button grid
- **Control buttons**: Mode switch, Clear ("지우기"), and Enter
- **Enter button**: Displays input result in popup with styled message box
  - Black transparent background (50% opacity)
  - White Korean text with font support
  - Green centered OK button
  - Auto-clears text after confirmation
- **Wide character support**: Full UTF-8 encoding for Korean text

### Implementation Details

The Chunjiin implementation consists of three main components:

1. **korean.c**: UI implementation with keyboard buttons and event handlers
2. **chunjiin.c**: Core input logic, button processing, and mode management
3. **keypad.c**: Hangul syllable composition algorithm

The system automatically composes Korean syllables from consonant and vowel inputs following standard Hangul composition rules.

## Version

- **Application**: 4.4 (With Static Memory Allocation & Security Hardening)
- **LVGL**: 8.4
- **SDL2**: Latest stable
- **FreeType**: Latest stable
- **Last Updated**: 2025-12-03

### Changelog

#### v4.4 (2025-12-03) - Static Memory Allocation & Security Hardening
- **Memory Safety**: Eliminated all dynamic memory allocation (malloc/free)
  - Replaced with fixed-size static buffers for all file I/O operations
  - Prevents buffer overflow and memory leak vulnerabilities
  - No more memory management overhead or fragmentation
- **Buffer Size Configuration**: Added centralized buffer size constants
  - `MAX_WELCOME_JSON_SIZE` (8 KB) - Welcome message configuration
  - `MAX_LABELS_JSON_SIZE` (64 KB) - Language labels and translations
  - `MAX_CONFIG_JSON_SIZE` (16 KB) - Application configuration
  - `MAX_FILE_CONTENT_SIZE` (16 KB) - General-purpose file reading
- **Boundary Checking**: Comprehensive validation before file operations
  - All file reads validate size against maximum buffer before processing
  - Graceful error handling for files exceeding maximum size
  - Error messages logged to stderr for debugging
- **Files Refactored**: 4 files modified, 7 malloc/free calls removed
  - `src/welcome.c`: Welcome message JSON loading (1 malloc → static buffer)
  - `src/label.c`: Language JSON loading (2 malloc → static buffers)
  - `src/config.c`: Configuration file reading (4 malloc/free calls → static buffer)
  - `include/config.h`: Added 4 buffer size constants
- **Security Benefits**:
  - Eliminates buffer overflow attacks via oversized files
  - Prevents heap fragmentation attacks
  - Removes use-after-free and double-free vulnerabilities
  - Predictable memory layout for embedded systems
- **Build Status**: 0 errors, 0 warnings

#### v4.3 (2025-12-03) - Welcome Messages & Code Refactoring Phase 1
- **Welcome Message Feature**: Time-based and language-aware greeting messages
  - Displays different messages for morning, afternoon, evening, and night
  - Supports Korean and English languages
  - Color animation cycling through 6 vibrant colors every 5 seconds
  - Large 30px bold NotoSansKR font for visibility
  - Auto-updates based on current time period
  - Configured via `config/welcome.json`
  - Positioned in upper 1/3 of home screen
- **Code Refactoring Phase 1**: Foundation work for improving code quality
  - Extracted 30+ hardcoded magic numbers to central `include/config.h`
  - Refactored `src/home.c` to use named constants for colors, sizing, and timers
  - Refactored `src/welcome.c` to use time period constants
  - Reduced technical debt by 22% (135+ → 105+ items)
  - 70% reduction in hardcoded values (50+ → 15+)
  - Created comprehensive 8-week refactoring roadmap (REFACTORING_NOTES.md)
  - Documented completion report (REFACTORING_PHASE1_COMPLETE.md)
  - Build status: 0 errors, 0 warnings
  - Improvements:
    - Central configuration management
    - Improved code readability
    - Enhanced maintainability
    - Foundation for Phase 2 module creation

#### v4.2 (2025-12-01) - Centralized Menu Configuration & File Cleanup
- **Menu system refactoring**: Introduced centralized menu configuration
  - Created `MenuItem` structure in types.h
  - Defined `MENU_ITEMS` array with all menu configurations in one place
  - Eliminated hardcoded menu labels, icons, and callbacks throughout codebase
  - Removed repetitive if-else chains for menu item handling
- **File renaming for clarity**:
  - `korean_input.c/h` → `korean.c/h`
  - `chunjiin_hangul.c` → `keypad.c`
  - Merged `screen_components.c/h` into `screen.c/h`
- **Configuration system improvements**:
  - Updated to use menu configuration keys from MENU_ITEMS
  - Added support for 5th menu item (Face screen)
  - Fixed segmentation faults in config save/load
- **New Face screen**: Added customizable face screen accessible from menu
- **Benefits**:
  - Single source of truth for menu configuration
  - Easy to add/modify menu items (just update MENU_ITEMS array)
  - Improved type safety using structures
  - Better code organization and maintainability

#### v4.1 (2025-11-30) - Refactored Architecture
- **Major refactoring**: Comprehensive code restructuring for maintainability
- Created reusable component system
  - Standard title bar component with breadcrumb navigation
  - Standard status bar component with icon buttons
  - Standard content area component
  - Screen base creation and finalization functions
- Centralized navigation callbacks (navigation.c/h)
  - Single implementation of all navigation functions
  - Eliminated 200+ lines of duplicate callback code
- Refactored all screen implementations to use components:
  - admin.c: 216 → 53 lines (76% reduction)
  - info.c: 212 → 50 lines (76% reduction)
  - network.c: 214 → 52 lines (76% reduction)
  - menu.c: 231 → 72 lines (69% reduction)
  - korean_input.c: 457 → 306 lines (33% reduction)
- **Total impact**: Eliminated ~500+ lines of duplicate code (20% → 0% duplication)
- Updated Makefile to include new component files
- All screens now follow consistent component-based pattern

#### v4.0 (2025-11-30) - Korean Input with Enter Button
- Implemented Chunjiin Korean input method
- Added Korean input screen with 12-button keyboard layout
- Implemented multi-mode support (Hangul, English, Numbers, Special characters)
- Added real-time Hangul syllable composition
- Created centered keyboard layout matching standard Chunjiin design
- **Added Enter button** to Korean input screen
  - Displays input result in popup with styled message box
  - Black transparent background (50% opacity)
  - White Korean text with proper font support
  - Green centered OK button
  - Auto-clears text area after confirmation
- Updated all screen status bars with Korean input button
- Replaced setup icon with Korean input icon (korean.png, 40x40)
- Updated menu item from "메뉴 3" to "한글 입력"
- Added wide character (wchar_t) to UTF-8 conversion
- Integrated Chunjiin source files (chunjiin.c, chunjiin_hangul.c)

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
