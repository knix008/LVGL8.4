#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// ============================================================================
// SCREEN CONFIGURATION
// ============================================================================

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 640
#define BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10)

#define TITLE_BAR_HEIGHT 60
#define STATUS_BAR_HEIGHT 60
#define BUTTON_WIDTH 60
#define BUTTON_HEIGHT 40

// ============================================================================
// COLOR CONFIGURATION
// ============================================================================

#define COLOR_BG_DARK 0x2A2A2A
#define COLOR_BG_TITLE 0x1A1A1A
#define COLOR_BUTTON_BG 0x1A1A1A
#define COLOR_BUTTON_BACK 0x444444
#define COLOR_BORDER 0x888888
#define COLOR_TEXT 0xFFFFFF
#define COLOR_TRANSPARENT 128

// Predefined theme colors for color picker
#define COLOR_BG_LIGHT_GRAY 0x333333
#define COLOR_BG_BLACK 0x000000
#define COLOR_TITLE_DARK 0x1F1F1F
#define COLOR_TITLE_GRAY 0x404040
#define COLOR_STATUS_DARK 0x222222
#define COLOR_STATUS_GRAY 0x333333
#define COLOR_BUTTON_DARK 0x1A1A1A
#define COLOR_BUTTON_GRAY 0x2A2A2A
#define COLOR_BORDER_DIM 0x555555
#define COLOR_BORDER_BRIGHT 0xAAAAAA

// Welcome message colors animation
#define WELCOME_COLOR_WHITE 0xFFFFFF
#define WELCOME_COLOR_PINK 0xFF6B9D
#define WELCOME_COLOR_RED_PINK 0xC44569
#define WELCOME_COLOR_GOLD 0xF8B500
#define WELCOME_COLOR_CYAN 0x00D4FF
#define WELCOME_COLOR_GREEN 0x00FF88

// ============================================================================
// APPLICATION CONFIGURATION
// ============================================================================

#define MAX_SCREENS 10
#define FONT_SIZE 14
#define MAX_BREADCRUMB_LENGTH 256
#define MAX_TITLE_LENGTH 256

// ============================================================================
// UI ELEMENT SIZING
// ============================================================================

#define ICON_SIZE_SMALL 40
#define ICON_IMAGE_OFFSET 10
#define LABEL_OFFSET_X 60
#define BACK_BUTTON_PADDING 20
#define CONTENT_PADDING 10
#define CONTENT_WIDTH_PADDING 20
#define CONTENT_WIDTH_LARGE_PADDING 40
#define STATUS_ICON_SPACING 10
#define VERTICAL_OFFSET_SMALL 20
#define VERTICAL_OFFSET_MEDIUM 50
#define VERTICAL_OFFSET_LARGE 80

// ============================================================================
// VISUAL EFFECTS
// ============================================================================

#define ZOOM_NORMAL 256      // 100% zoom
#define ZOOM_PRESSED 230     // 90% zoom when pressed
#define OPACITY_PRESSED 60   // 60% opacity when pressed

// ============================================================================
// BORDER CONFIGURATION
// ============================================================================

// Default border colors
#define BORDER_COLOR_GREEN 0x00FF00
#define BORDER_COLOR_RED 0xFF0000
#define BORDER_COLOR_BLUE 0x0000FF
#define BORDER_COLOR_YELLOW 0xFFFF00
#define BORDER_COLOR_ORANGE 0xFF5733
#define BORDER_COLOR_PURPLE 0x8A2BE2
#define BORDER_COLOR_WHITE 0xFFFFFF

// Default border settings
#define BORDER_WIDTH_DEFAULT 8
#define BORDER_WIDTH_THIN 4
#define BORDER_WIDTH_THICK 12
#define BORDER_WIDTH_EXTRA_THICK 16

// ============================================================================
// CHUNJIIN KEYBOARD
// ============================================================================

#define CHUNJIIN_SPACE_KEY 10
#define CHUNJIIN_DELETE_KEY 11

// ============================================================================
// CONFIGURATION FILE PATHS
// ============================================================================

#define CONFIG_DIR "config"
#define STATUS_BAR_CONFIG_FILE "config/config.json"

// ============================================================================
// CONFIGURATION MANAGEMENT FUNCTIONS
// ============================================================================

// Save status bar configuration to JSON file
int save_status_bar_config(void);

// Save and load theme configuration
int save_theme_config(void);
int load_theme_config(void);
uint32_t get_background_color(void);
uint32_t get_title_bar_color(void);
uint32_t get_status_bar_color(void);
uint32_t get_button_color(void);
uint32_t get_button_border_color(void);

// Load status bar configuration from YAML file
int load_status_bar_config(void);

// ============================================================================
// IMAGE PATHS
// ============================================================================

#define IMG_BACK_BUTTON "A:assets/images/backbutton.png"
#define IMG_CONFIG "A:assets/images/config.png"
#define IMG_SETUP "A:assets/images/setup.png"
#define IMG_INFO "A:assets/images/Info.png"
#define IMG_NETWORK "A:assets/images/network.png"
#define IMG_KOREAN "A:assets/images/korean.png"
#define IMG_FACE "A:assets/images/face.png"
#define IMG_PLUS "A:assets/images/plus.png"
#define IMG_MINUS "A:assets/images/minus.png"
#define IMG_CANCEL "A:assets/images/cancel_button_40x40.png"

// ============================================================================
// GUI LAYOUT CONFIGURATION
// ============================================================================

// Padding and margins
#define PADDING_HORIZONTAL 10
#define PADDING_VERTICAL 5
#define PADDING_BUTTON 20
#define MARGIN_BUTTON 10
#define OFFSET_BUTTON_START_Y 20

// Label and title widths
#define TITLE_LABEL_WIDTH (SCREEN_WIDTH - 20)
#define MENU_BUTTON_WIDTH (SCREEN_WIDTH - 20)

// Menu configuration
#define MENU_ITEMS_COUNT 5
#define MENU_BUTTON_HEIGHT 60
#define MENU_BUTTON_MARGIN 10

// Update intervals (milliseconds)
#define UPDATE_INTERVAL_TIMER 1000
#define WELCOME_MESSAGE_UPDATE_INTERVAL 60000
#define WELCOME_COLOR_UPDATE_INTERVAL 5000

// Frame timing (milliseconds)
#define FRAME_DELAY_MS 1

// ============================================================================
// ADMIN SCREEN CONFIGURATION
// ============================================================================

// Color picker button dimensions
#define ADMIN_COLOR_BUTTON_WIDTH 60
#define ADMIN_COLOR_BUTTON_HEIGHT 40
#define ADMIN_COLOR_BUTTON_SPACING 5

// Admin screen layout
#define ADMIN_COLOR_SECTION_LABEL_Y_OFFSET 30
#define ADMIN_COLOR_BUTTON_Y_OFFSET 10
#define ADMIN_BORDER_SECTION_Y 415
#define ADMIN_LANGUAGE_SECTION_Y 470
#define ADMIN_LANGUAGE_BUTTONS_Y 505
#define ADMIN_LANGUAGE_INFO_Y 550

// ============================================================================
// WELCOME MESSAGE CONFIGURATION
// ============================================================================

// Time periods for welcome messages (24-hour format)
#define WELCOME_MORNING_START_HOUR 5
#define WELCOME_MORNING_END_HOUR 12
#define WELCOME_AFTERNOON_START_HOUR 12
#define WELCOME_AFTERNOON_END_HOUR 18
#define WELCOME_EVENING_START_HOUR 18
#define WELCOME_EVENING_END_HOUR 22

// Welcome message display
#define WELCOME_MESSAGE_CONTAINER_HEIGHT 120
#define WELCOME_MESSAGE_Y_POSITION 150
#define WELCOME_MESSAGE_FONT_SIZE 30

// ============================================================================
// FILE BUFFER SIZES (Static Memory Allocation)
// ============================================================================

#define MAX_WELCOME_JSON_SIZE 8192      // Maximum size for welcome.json
#define MAX_LABELS_JSON_SIZE 65536      // Maximum size for language label JSONs
#define MAX_CONFIG_JSON_SIZE 16384      // Maximum size for config.json
#define MAX_FILE_CONTENT_SIZE 16384     // General maximum file content size

// ============================================================================
// SCREEN IDS
// ============================================================================

enum {
    SCREEN_MAIN = 0,
    SCREEN_MENU = 1,
    SCREEN_INFO = 2,
    SCREEN_ADMIN = 3,
    SCREEN_NETWORK = 4,
    SCREEN_KOREAN_INPUT = 5,
    SCREEN_FACE = 6
};

#endif
