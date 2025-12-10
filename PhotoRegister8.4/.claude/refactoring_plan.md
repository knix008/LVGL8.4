# Application Refactoring Plan

## Overview
This document outlines a comprehensive refactoring plan for the LVGL-based embedded GUI application. The refactoring addresses critical safety issues, code organization problems, and maintainability concerns identified through static analysis.

## Project Structure (Current)
```
/home/shkwon/Projects/LVGL8.4/Video8.4/
├── src/               # 23 C source files
├── include/           # 25 header files
├── assets/
│   ├── fonts/
│   ├── images/
│   └── videos/
├── config/            # JSON configuration files
├── build/             # Build artifacts
└── Makefile           # Build configuration
```

## Refactoring Phases

### Phase 1: Critical Safety Fixes (Priority: CRITICAL)
**Estimated Effort: 2-3 days**
**Risk Level: Low (fixes only, no restructuring)**

#### 1.1 Buffer Overflow Protection

**Files to Fix:**
- `src/network.c:250-280` (number_btn_callback)
- `src/network.c:378` (IP config strcpy)
- `src/config.c` (multiple strcpy/sprintf instances)
- `src/korean.c:98` (wchar_to_utf8 usage)

**Changes:**
1. Replace all `strcpy()` with `strncpy()` + null termination
2. Add bounds checking before all array access
3. Validate `cursor_pos` in all input handlers
4. Add buffer size constants

**Example Change:**
```c
// Before (network.c:250-280)
if (len < 15) {
    for (int i = len; i > cursor_pos; i--) {
        temp_ipv4[i] = temp_ipv4[i - 1];
    }
    temp_ipv4[cursor_pos] = ch;

// After
#define IPV4_BUFFER_SIZE 16  // 15 chars + null terminator
#define IPV4_MAX_LENGTH 15

if (len >= IPV4_MAX_LENGTH || cursor_pos > len || cursor_pos < 0) {
    return;  // Prevent buffer overflow
}
for (int i = len; i > cursor_pos; i--) {
    temp_ipv4[i] = temp_ipv4[i - 1];
}
temp_ipv4[cursor_pos] = ch;
```

#### 1.2 Error Handling Enhancement

**Files to Fix:**
- `src/config.c:210-250` (fprintf return values)
- `src/network.c` (file operations)
- `src/config.c:121-207` (JSON parsing)

**Changes:**
1. Check all `fprintf()` return values
2. Add error handling for file operations
3. Validate JSON parsing results
4. Add error logging

**Example Change:**
```c
// Before
fprintf(file, "{\n");
fprintf(file, "  \"status_bar\": {\n");

// After
if (fprintf(file, "{\n") < 0) {
    log_error("Failed to write opening brace");
    fclose(file);
    return -1;
}
if (fprintf(file, "  \"status_bar\": {\n") < 0) {
    log_error("Failed to write status_bar section");
    fclose(file);
    return -1;
}
```

**Deliverables:**
- [ ] All unsafe string operations replaced
- [ ] All array accesses bounds-checked
- [ ] All file I/O operations error-checked
- [ ] Safety test suite created and passing

---

### Phase 2: Code Organization (Priority: HIGH)
**Estimated Effort: 4-5 days**
**Risk Level: Medium (file splitting requires Makefile updates)**

#### 2.1 Split Large Files

##### 2.1.1 Split network.c (983 lines → 3 files)

**New File Structure:**
```
src/network/
├── network_validation.c  # IP validation logic
├── network_ui.c          # Popup and screen UI
├── network_config.c      # Config persistence
└── network_private.h     # Shared private definitions
```

**network_validation.c** (lines 53-119, ~70 lines)
- `is_valid_ipv4()`
- `is_valid_ipv6()`
- IPv4/IPv6 validation helpers

**network_ui.c** (lines 464-767, ~300 lines + callbacks)
- `create_ip_popup_content()`
- Split into:
  - `create_ip_keypad_ipv4()`
  - `create_ip_keypad_ipv6()`
  - `create_ip_control_buttons()`
  - `create_ip_display_area()`
- All button callbacks

**network_config.c** (lines 121-191, ~70 lines)
- `save_ip_config()`
- `load_ip_config()`
- JSON handling

**include/network.h** (public API only)
```c
void create_network_screen(void);
int save_ip_config(void);
int load_ip_config(void);
```

##### 2.1.2 Split admin.c (837 lines → 3 files)

**New File Structure:**
```
src/admin/
├── admin_colors.c      # Color picker logic
├── admin_calendar.c    # Calendar date selection
├── admin_language.c    # Language selection
└── admin_private.h     # Shared definitions
```

**admin_colors.c** (~400 lines)
- `create_color_picker_section()`
- `update_all_screen_colors()`
- Color option arrays (move to constants)

**admin_calendar.c** (~150 lines)
- Calendar popup creation
- Calendar date selection callbacks

**admin_language.c** (~150 lines)
- Language selection UI
- Language switching logic

##### 2.1.3 Split config.c (751 lines → 2 files)

**New File Structure:**
```
src/config/
├── config_parser.c     # JSON parsing utilities
└── config_theme.c      # Theme management
```

**config_parser.c**
- `extract_json_section()` (new utility function)
- JSON read/write utilities
- File I/O operations

**config_theme.c**
- `save_theme_config()`
- `load_theme_config()`
- Theme color management

##### 2.1.4 Split input.c (779 lines → 3 files)

**New File Structure:**
```
src/input/
├── input_hangul.c      # Hangul composition algorithm
├── input_text.c        # Text input handling
└── input_private.h     # Shared state
```

**input_hangul.c**
- `hangul_make()` split into:
  - `process_vowel_input()`
  - `process_consonant_input()`
  - `handle_batchim_separation()`
- Hangul composition tables

**input_text.c**
- `write_hangul()`
- Text buffer management
- Cursor handling

**Makefile Updates:**
```makefile
# Add new source directories
NETWORK_SRCS = $(wildcard $(SRC_DIR)/network/*.c)
ADMIN_SRCS = $(wildcard $(SRC_DIR)/admin/*.c)
CONFIG_SRCS = $(wildcard $(SRC_DIR)/config/*.c)
INPUT_SRCS = $(wildcard $(SRC_DIR)/input/*.c)

SRCS = $(wildcard $(SRC_DIR)/*.c) $(NETWORK_SRCS) $(ADMIN_SRCS) \
       $(CONFIG_SRCS) $(INPUT_SRCS)
```

#### 2.2 Extract Duplicate Code

##### 2.2.1 Cursor Animation Utility

**Create:** `src/ui/cursor_utils.c` + `include/ui/cursor_utils.h`

```c
// cursor_utils.h
typedef void (*cursor_update_callback_t)(void);

typedef struct {
    lv_timer_t *timer;
    bool visible;
    cursor_update_callback_t update_fn;
} CursorState;

CursorState* cursor_create(cursor_update_callback_t update_fn, uint32_t blink_ms);
void cursor_start(CursorState *cursor);
void cursor_stop(CursorState *cursor);
void cursor_reset(CursorState *cursor);
void cursor_destroy(CursorState *cursor);
bool cursor_is_visible(CursorState *cursor);
```

**Usage:**
```c
// network.c
static CursorState *ip_cursor = NULL;

void show_ip_popup(void) {
    ip_cursor = cursor_create(update_popup_ip_display, 500);
    cursor_start(ip_cursor);
}

void hide_ip_popup(void) {
    cursor_stop(ip_cursor);
    cursor_destroy(ip_cursor);
    ip_cursor = NULL;
}
```

**Files to Update:**
- `src/network.c` (remove lines 194-214)
- `src/korean.c` (remove lines 41-61)

##### 2.2.2 JSON Section Extraction Utility

**Add to:** `src/config/config_parser.c`

```c
// config_parser.h
bool extract_json_section(
    const char* json,
    const char* key,
    char* output,
    size_t output_size
);
```

**Implementation:** See refactoring analysis example

**Files to Update:**
- `src/config.c` lines 121-207 (replace 4 duplicated blocks)

##### 2.2.3 Color Picker Factory

**Create:** `src/ui/color_picker.c` + `include/ui/color_picker.h`

```c
// color_picker.h
typedef struct {
    const char *label;
    uint32_t color;
} ColorOption;

typedef enum {
    COLOR_TARGET_BACKGROUND,
    COLOR_TARGET_TITLE_BAR,
    COLOR_TARGET_STATUS_BAR,
    COLOR_TARGET_BUTTON,
    COLOR_TARGET_BORDER
} ColorTarget;

lv_obj_t* create_color_picker(
    lv_obj_t *parent,
    const char *title,
    ColorTarget target,
    const ColorOption *options,
    int option_count,
    int x, int y
);
```

**Files to Update:**
- `src/admin.c` (remove duplicated color picker code)

#### 2.3 Break Up Long Functions

##### 2.3.1 Refactor hangul_make() (348 lines)

**Current:** `src/input.c` lines 181-529

**Split into:**
```c
// input_hangul.c
static void process_vowel_input(HangulState *state, int input);
static void process_consonant_input(HangulState *state, int input);
static void handle_batchim_separation(HangulState *state);
static void finalize_current_syllable(HangulState *state);

void hangul_make(int input) {
    if (is_vowel(input)) {
        process_vowel_input(&hangul_state, input);
    } else {
        process_consonant_input(&hangul_state, input);
    }
}
```

**Complexity Reduction:**
- Before: Cyclomatic complexity ~40
- After: Each function complexity <10

##### 2.3.2 Refactor create_ip_popup_content() (303 lines)

**Current:** `src/network.c` lines 464-767

**Split into:**
```c
// network_ui.c
static lv_obj_t* create_ip_display_area(lv_obj_t *parent);
static lv_obj_t* create_ip_keypad_ipv4(lv_obj_t *parent, int y_offset);
static lv_obj_t* create_ip_keypad_ipv6(lv_obj_t *parent, int y_offset);
static void create_ip_control_buttons(lv_obj_t *parent, int y_offset);

static lv_obj_t* create_ip_popup_content(lv_obj_t *popup) {
    lv_obj_t *container = create_container(popup);
    create_ip_display_area(container);
    create_ip_keypad_ipv4(container, 100);
    create_ip_keypad_ipv6(container, 250);
    create_ip_control_buttons(container, 500);
    return container;
}
```

##### 2.3.3 Refactor save_theme_config() (138 lines)

**Current:** `src/config.c` lines 341-479

**Simplify using extract_json_section():**
```c
int save_theme_config(void) {
    char existing_config[MAX_FILE_CONTENT_SIZE] = {0};
    read_existing_config(existing_config, sizeof(existing_config));

    char border_section[512], fonts_section[512], ip_section[512];

    extract_json_section(existing_config, "border", border_section, sizeof(border_section));
    extract_json_section(existing_config, "fonts", fonts_section, sizeof(fonts_section));
    extract_json_section(existing_config, "ip_config", ip_section, sizeof(ip_section));

    return write_theme_config(border_section, fonts_section, ip_section);
}
```

**Deliverables:**
- [ ] 4 large files split into focused modules
- [ ] 3 duplicate code patterns extracted to utilities
- [ ] 3 long functions refactored
- [ ] Makefile updated for new structure
- [ ] All files compile and link successfully

---

### Phase 3: State Management Refactoring (Priority: HIGH)
**Estimated Effort: 3-4 days**
**Risk Level: Medium (changes global state access patterns)**

#### 3.1 Create State Accessor API

**Create:** `src/state/app_state.c` + `include/state/app_state.h`

**New Header:**
```c
// app_state.h
#ifndef APP_STATE_H
#define APP_STATE_H

#include "lvgl/lvgl.h"
#include "types.h"

// Initialization
int app_state_init(void);
void app_state_cleanup(void);

// Screen management
lv_obj_t* app_state_get_screen(void);
void app_state_set_screen(lv_obj_t *screen);

// UI elements
lv_obj_t* app_state_get_title_bar(void);
lv_obj_t* app_state_get_status_bar(void);
lv_obj_t* app_state_get_title_label(void);
lv_obj_t* app_state_get_welcome_label(void);
lv_obj_t* app_state_get_menu_button_label(void);
lv_obj_t* app_state_get_exit_button_label(void);

// Font management
lv_font_t* app_state_get_font_20(void);
lv_font_t* app_state_get_font_button(void);
lv_font_t* app_state_get_font_24_bold(void);

// Color management
uint32_t app_state_get_bg_color(void);
void app_state_set_bg_color(uint32_t color);
uint32_t app_state_get_title_bar_color(void);
void app_state_set_title_bar_color(uint32_t color);
uint32_t app_state_get_status_bar_color(void);
void app_state_set_status_bar_color(uint32_t color);
uint32_t app_state_get_button_color(void);
void app_state_set_button_color(uint32_t color);
uint32_t app_state_get_button_border_color(void);
void app_state_set_button_border_color(uint32_t color);

// Language management
const char* app_state_get_language(void);
void app_state_set_language(const char *lang);

// Font configuration
int app_state_get_font_size_title_bar(void);
int app_state_get_font_size_label(void);
int app_state_get_font_size_button_label(void);
int app_state_get_font_size_bold(void);
const char* app_state_get_font_name_title(void);
const char* app_state_get_font_name_status_bar(void);
const char* app_state_get_font_name_button_label(void);

// Menu item selection
bool app_state_is_menu_item_selected(int index);
void app_state_set_menu_item_selected(int index, bool selected);
lv_obj_t* app_state_get_status_icon(int index);
void app_state_set_status_icon(int index, lv_obj_t *icon);

// Calendar date
calendar_date_t app_state_get_calendar_date(void);
void app_state_set_calendar_date(calendar_date_t date);

#endif
```

**Implementation:**
```c
// app_state.c
#include "state/app_state.h"

static AppState app_state = {0};

int app_state_init(void) {
    // Initialize default values
    app_state.bg_color = COLOR_BG_DARK;
    app_state.title_bar_color = COLOR_BG_TITLE;
    app_state.status_bar_color = COLOR_BG_TITLE;
    app_state.button_color = COLOR_BUTTON_BG;
    app_state.button_border_color = COLOR_BORDER;
    strncpy(app_state.current_language, "ko", sizeof(app_state.current_language) - 1);
    return 0;
}

lv_obj_t* app_state_get_screen(void) {
    return app_state.screen;
}

void app_state_set_screen(lv_obj_t *screen) {
    app_state.screen = screen;
}

uint32_t app_state_get_bg_color(void) {
    return app_state.bg_color;
}

void app_state_set_bg_color(uint32_t color) {
    app_state.bg_color = color;
}

// ... implement all other accessors
```

#### 3.2 Remove External AppState References

**Files to Update (22 occurrences across 10 files):**

1. **src/ui_helpers.c** (5 occurrences)
```c
// Before
extern AppState app_state;
lv_obj_set_style_bg_color(screen, lv_color_hex(app_state.bg_color), 0);

// After
#include "state/app_state.h"
lv_obj_set_style_bg_color(screen, lv_color_hex(app_state_get_bg_color()), 0);
```

2. **src/admin.c** (5 occurrences)
```c
// Before
extern AppState app_state;
app_state.bg_color = option->color;

// After
#include "state/app_state.h"
app_state_set_bg_color(option->color);
```

3. **src/network.c** (4 occurrences)
4. **src/korean.c** (1 occurrence)
5. **src/config.c** (1 occurrence)
6. **src/screen.c** (1 occurrence)
7. **src/home.c** (update if needed)
8. **src/menu.c** (update if needed)
9. **src/calendar.c** (update if needed)
10. **src/info.c** (update if needed)

#### 3.3 Screen Stack Encapsulation

**Create:** `src/state/screen_stack.c` + `include/state/screen_stack.h`

```c
// screen_stack.h
#ifndef SCREEN_STACK_H
#define SCREEN_STACK_H

#include "types.h"

int screen_stack_init(void);
int screen_stack_push(lv_obj_t *screen, int screen_id);
int screen_stack_pop(void);
ScreenState* screen_stack_get_current(void);
ScreenState* screen_stack_get_at(int index);
int screen_stack_get_depth(void);
void screen_stack_invalidate_all(void);
int screen_stack_find(int screen_id);

#endif
```

**Files to Update:**
- Remove `extern ScreenState screen_stack[]` from `src/admin.c`
- Remove `extern int screen_stack_top` from `src/admin.c`
- Update `src/screen.c` to use new API

**Deliverables:**
- [ ] State accessor API created
- [ ] All extern AppState references removed
- [ ] Screen stack encapsulated
- [ ] All modules compile with new API
- [ ] State management tests passing

---

### Phase 4: Code Quality Improvements (Priority: MEDIUM)
**Estimated Effort: 2-3 days**
**Risk Level: Low (cosmetic changes)**

#### 4.1 Replace Magic Numbers

##### 4.1.1 Color Constants

**Files to Update:**
- `src/network.c` (28 instances)
- `src/admin.c` (28 instances)
- `src/ui_helpers.c` (multiple instances)

**Create:** `include/ui/ui_colors.h`

```c
// ui_colors.h
#ifndef UI_COLORS_H
#define UI_COLORS_H

// UI Feedback Colors
#define UI_COLOR_SUCCESS       0x00FF00
#define UI_COLOR_ERROR         0xFF6666
#define UI_COLOR_WARNING       0xFFCC00
#define UI_COLOR_INFO          0xAAAAAA
#define UI_COLOR_SELECTED      0x00FF00
#define UI_COLOR_DISABLED      0x666666

// UI Element Colors
#define UI_COLOR_BLACK         0x000000
#define UI_COLOR_WHITE         0xFFFFFF
#define UI_COLOR_TRANSPARENT_BG 0x000000  // with alpha

// Border Colors
#define UI_COLOR_BORDER_ERROR  0xFF0000
#define UI_COLOR_BORDER_INFO   0xAAAAAA

#endif
```

**Updates:**
```c
// Before (network.c)
lv_obj_set_style_bg_color(mbox, lv_color_hex(0x000000), 0);
lv_obj_set_style_border_color(mbox, lv_color_hex(0xFF0000), 0);
lv_obj_set_style_text_color(text, lv_color_hex(0xFFFFFF), 0);

// After
lv_obj_set_style_bg_color(mbox, lv_color_hex(UI_COLOR_BLACK), 0);
lv_obj_set_style_border_color(mbox, lv_color_hex(UI_COLOR_BORDER_ERROR), 0);
lv_obj_set_style_text_color(text, lv_color_hex(UI_COLOR_WHITE), 0);
```

##### 4.1.2 Dimension Constants

**Add to:** `include/config.h` (or new `include/ui/ui_dimensions.h`)

```c
// IP Input Constants
#define IPV4_MAX_LENGTH        15  // "xxx.xxx.xxx.xxx"
#define IPV6_MAX_LENGTH        39  // Full IPv6 format
#define IPV4_BUFFER_SIZE       16  // Max length + null terminator
#define IPV6_BUFFER_SIZE       40  // Max length + null terminator

// Text Buffer Constants
#define UTF8_MAX_BYTES_PER_CHAR 4
#define CURSOR_CHAR_COUNT       1

// Button Dimensions
#define KEYPAD_BUTTON_SIZE     50
#define KEYPAD_BUTTON_SPACING  8
#define KOREAN_BUTTON_WIDTH    70
#define KOREAN_BUTTON_HEIGHT   50
#define KOREAN_BUTTON_SPACING  8
```

**Files to Update:**
- `src/network.c` lines 256, 269, 554-555
- `src/korean.c` lines 74, 321-324

#### 4.2 Standardize Naming Conventions

**Naming Standards:**
1. Functions: `snake_case`
2. Constants: `UPPER_SNAKE_CASE`
3. Types: `PascalCase` with `_t` suffix
4. No abbreviations (except common ones: `max`, `min`, `idx`)

**Files to Update:**

##### 4.2.1 Function Names
```c
// network.c
// Before
error_msgbox_event_cb()

// After
error_message_box_event_callback()
```

##### 4.2.2 Variable Names
```c
// config.c
// Before
lv_obj_t *sb_start;  // Unclear abbreviation
lv_obj_t *btn;
lv_obj_t *mbox;

// After
const char *status_bar_start;
lv_obj_t *button;
lv_obj_t *message_box;
```

##### 4.2.3 Consistent Event Callback Names
```c
// Pattern: <element>_<action>_callback
button_clicked_callback()
slider_value_changed_callback()
screen_pressed_callback()
timer_tick_callback()
```

#### 4.3 Add Function Documentation

**Documentation Standard:**
```c
/**
 * @brief Brief one-line description
 *
 * Detailed description of what the function does,
 * including any important algorithm details.
 *
 * @param param1 Description of first parameter
 * @param param2 Description of second parameter
 * @return Description of return value
 *
 * @note Any important notes or warnings
 * @example
 * // Example usage
 * result = function_name(arg1, arg2);
 */
```

**Priority Functions to Document:**

1. **Complex Algorithms:**
   - `hangul_make()` and sub-functions
   - `is_valid_ipv4()`, `is_valid_ipv6()`
   - JSON parsing functions

2. **Public APIs:**
   - All functions in public headers
   - State management API
   - Screen creation functions

3. **Callback Functions:**
   - Button callbacks with complex logic
   - Timer callbacks
   - Event handlers

**Example:**
```c
/**
 * @brief Handle number button press in IP input keypad
 *
 * Inserts the pressed digit at the current cursor position,
 * shifting existing characters to the right. Validates buffer
 * capacity before insertion to prevent overflow.
 *
 * Algorithm:
 * 1. Extract digit character from event user data
 * 2. Validate buffer capacity (15 for IPv4, 39 for IPv6)
 * 3. Validate cursor position is within bounds
 * 4. Shift characters right from cursor position
 * 5. Insert new character at cursor position
 * 6. Increment cursor position
 * 7. Update display
 *
 * @param e Event data containing the pressed digit as user_data
 *
 * @note This function assumes temp_ipv4/temp_ipv6 buffers are
 *       properly sized and null-terminated.
 */
static void number_button_clicked_callback(lv_event_t *e) {
    // Implementation
}
```

**Deliverables:**
- [ ] All magic numbers replaced with named constants
- [ ] All functions follow naming conventions
- [ ] All variables use clear, descriptive names
- [ ] Public APIs fully documented
- [ ] Complex functions documented with algorithm details

---

### Phase 5: Header Organization (Priority: LOW)
**Estimated Effort: 1-2 days**
**Risk Level: Low**

#### 5.1 Split config.h

**Current:** `include/config.h` (247 lines, too many responsibilities)

**New Structure:**
```
include/config/
├── screen_config.h       # Screen dimensions
├── colors.h              # Color constants
├── layout.h              # Spacing, padding, offsets
├── paths.h               # File paths
├── timers.h              # Timer intervals
└── config_api.h          # Configuration function declarations
```

**screen_config.h:**
```c
#ifndef SCREEN_CONFIG_H
#define SCREEN_CONFIG_H

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 640
#define BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10)

#define TITLE_BAR_HEIGHT 60
#define STATUS_BAR_HEIGHT 60
#define BUTTON_WIDTH 60
#define BUTTON_HEIGHT 40

#define MAX_SCREENS 10
#define MAX_BREADCRUMB_LENGTH 256
#define MAX_TITLE_LENGTH 256

#endif
```

**colors.h:**
```c
#ifndef UI_COLORS_H
#define UI_COLORS_H

// Background Colors
#define COLOR_BG_DARK 0x2A2A2A
#define COLOR_BG_LIGHT_GRAY 0x333333
#define COLOR_BG_BLACK 0x000000

// Title Bar Colors
#define COLOR_BG_TITLE 0x1A1A1A
#define COLOR_TITLE_DARK 0x1F1F1F
#define COLOR_TITLE_GRAY 0x404040

// ... (all other colors)

#endif
```

**layout.h:**
```c
#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

// Icon Sizing
#define ICON_SIZE_SMALL 40
#define ICON_IMAGE_OFFSET 10

// Padding
#define CONTENT_PADDING 10
#define CONTENT_WIDTH_PADDING 20
#define PADDING_HORIZONTAL 10
#define PADDING_VERTICAL 5

// Offsets
#define VERTICAL_OFFSET_SMALL 20
#define VERTICAL_OFFSET_MEDIUM 50
#define VERTICAL_OFFSET_LARGE 80

#endif
```

**paths.h:**
```c
#ifndef ASSET_PATHS_H
#define ASSET_PATHS_H

// Configuration Files
#define CONFIG_DIR "config"
#define STATUS_BAR_CONFIG_FILE "config/config.json"
#define IP_CONFIG_FILE "config/ip_config.json"

// Asset Directories
#define VIDEO_DIR "assets/videos"
#define IMAGE_DIR "assets/images"
#define FONT_DIR "assets/fonts"

// Image Assets
#define IMG_BACK_BUTTON "A:assets/images/backbutton.png"
#define IMG_CONFIG "A:assets/images/config.png"
// ... (all other image paths)

#endif
```

**timers.h:**
```c
#ifndef UI_TIMERS_H
#define UI_TIMERS_H

#define UPDATE_INTERVAL_TIMER 1000
#define WELCOME_MESSAGE_UPDATE_INTERVAL 60000
#define WELCOME_COLOR_UPDATE_INTERVAL 5000
#define INACTIVITY_TIMEOUT 10000
#define FRAME_DELAY_MS 1

#endif
```

**config_api.h:**
```c
#ifndef CONFIG_API_H
#define CONFIG_API_H

#include <stdint.h>

// Theme configuration
int save_theme_config(void);
int load_theme_config(void);
int load_status_bar_config(void);

// IP configuration
int save_ip_config(void);
int load_ip_config(void);

// Border configuration
int save_border_config(uint32_t color, int width);
int load_border_config(uint32_t *color, int *width);

#endif
```

#### 5.2 Create Layered Architecture

**Layer 1: Core** (No dependencies on other layers)
```
include/core/
├── types.h           # Base types
└── logger.h          # Logging
```

**Layer 2: Utilities** (Depends on Layer 1 only)
```
include/utils/
├── label.h           # Label management
└── font.h            # Font utilities
```

**Layer 3: Configuration** (Depends on Layer 1-2)
```
include/config/
├── screen_config.h
├── colors.h
├── layout.h
├── paths.h
├── timers.h
└── config_api.h
```

**Layer 4: State** (Depends on Layer 1-3)
```
include/state/
├── app_state.h
└── screen_stack.h
```

**Layer 5: UI Components** (Depends on Layer 1-4)
```
include/ui/
├── ui_helpers.h
├── ui_colors.h
├── cursor_utils.h
├── color_picker.h
├── border.h
└── style.h
```

**Layer 6: Screens** (Depends on all lower layers)
```
include/screens/
├── home.h
├── menu.h
├── admin.h
├── network.h
├── korean.h
├── info.h
├── face.h
└── calendar.h
```

**Layer 7: Navigation** (Top layer, orchestrates screens)
```
include/
└── navigation.h
```

**Dependency Rules:**
- Each layer can only depend on lower layers
- No circular dependencies
- No skipping layers (e.g., Layer 6 cannot directly use Layer 2, must go through intermediate layers)

**Deliverables:**
- [ ] config.h split into focused headers
- [ ] Headers organized into layers
- [ ] Dependency graph validated (no circular deps)
- [ ] Include guards consistent
- [ ] All files compile with new structure

---

## Implementation Strategy

### Recommended Order

1. **Phase 1 first** (Critical Safety)
   - Must be done before anything else
   - Low risk, high impact
   - Fixes production safety issues

2. **Phase 3 second** (State Management)
   - Do before Phase 2 to avoid refactoring files twice
   - Establishes clean API patterns
   - Makes Phase 2 easier

3. **Phase 2 third** (Code Organization)
   - Now that state API is clean, split files
   - Use new patterns established in Phase 3
   - Benefits from safety fixes in Phase 1

4. **Phase 4 fourth** (Code Quality)
   - Polish the refactored code
   - Apply consistent patterns
   - Document the improved structure

5. **Phase 5 last** (Header Organization)
   - Final structural cleanup
   - Nice-to-have, not critical
   - Can be deferred if time constrained

### Testing Strategy

**After Each Phase:**
1. Compile all files: `make clean && make`
2. Run application: `./system`
3. Test affected functionality:
   - Phase 1: Test input fields, config saving
   - Phase 2: Test all screens that were split
   - Phase 3: Test state changes (colors, language, etc.)
   - Phase 4: Regression test all features
   - Phase 5: Final regression test

**Manual Test Cases:**
- Home screen with video playback
- Menu navigation
- Network IP configuration
- Korean input
- Admin settings (colors, language, calendar)
- Info screen
- Face screen
- Configuration persistence (restart app, check settings saved)

### Rollback Plan

**Git Strategy:**
1. Create feature branch for each phase: `refactor/phase-1`, etc.
2. Commit after each sub-task
3. Tag working states: `v1.0-phase1-complete`
4. Can rollback to any phase if issues arise

**Backup Strategy:**
1. Before starting, create full backup: `tar -czf backup-pre-refactor.tar.gz .`
2. Keep backup of working executable
3. Keep copy of original source in separate directory

### Risk Mitigation

**High Risk Areas:**
- File splitting (Makefile changes, #include updates)
- State management changes (affects all modules)
- Buffer overflow fixes (must be tested thoroughly)

**Mitigation:**
- Review all Makefile changes carefully
- Test compilation after each file split
- Create comprehensive test cases for input validation
- Use static analysis tools: `cppcheck src/`
- Use memory safety tools: `valgrind ./system`

## Success Criteria

### Phase 1 (Safety)
- [ ] Zero buffer overflow warnings from static analysis
- [ ] All file I/O operations check return values
- [ ] Input validation prevents out-of-bounds access
- [ ] Safety test suite passes

### Phase 2 (Organization)
- [ ] No file exceeds 500 lines
- [ ] No function exceeds 100 lines
- [ ] No duplicate code blocks >10 lines
- [ ] Cyclomatic complexity <15 for all functions

### Phase 3 (State)
- [ ] Zero `extern AppState` declarations in source files
- [ ] All state access through accessor API
- [ ] State encapsulated in single module

### Phase 4 (Quality)
- [ ] Zero magic numbers in code
- [ ] Consistent naming conventions
- [ ] All public APIs documented
- [ ] Code style consistent

### Phase 5 (Headers)
- [ ] No header exceeds 150 lines
- [ ] Clear layer separation
- [ ] No circular dependencies
- [ ] Include paths consistent

## Estimated Timeline

| Phase | Days | Calendar Time |
|-------|------|---------------|
| Phase 1: Safety | 2-3 | Week 1 |
| Phase 3: State | 3-4 | Week 1-2 |
| Phase 2: Organization | 4-5 | Week 2-3 |
| Phase 4: Quality | 2-3 | Week 3 |
| Phase 5: Headers | 1-2 | Week 4 |
| **Total** | **12-17 days** | **3-4 weeks** |

*Note: Timeline assumes one person working full-time. Add buffer for testing and unexpected issues.*

## Next Steps

1. **Review this plan** - Discuss priorities and scope
2. **Choose starting phase** - Recommend Phase 1
3. **Set up version control** - Create feature branches
4. **Create backup** - Save current working state
5. **Begin implementation** - Start with first sub-task

## Questions for User

Before proceeding, please confirm:

1. **Scope**: Do you want to implement all phases, or focus on specific ones?
2. **Priority**: Is safety (Phase 1) the highest priority?
3. **Timeline**: Is 3-4 weeks acceptable, or do you need faster delivery?
4. **Testing**: Do you have existing test cases, or should we create them?
5. **Compatibility**: Are there any external dependencies or APIs we must maintain?
6. **Style**: Any specific coding style guidelines to follow?

---

*This plan will be updated as implementation progresses and new insights emerge.*
