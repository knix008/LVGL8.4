#ifndef CONFIG_H
#define CONFIG_H

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

// ============================================================================
// APPLICATION CONFIGURATION
// ============================================================================

#define MAX_SCREENS 10
#define FONT_SIZE 16
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

// Save status bar configuration to YAML file
int save_status_bar_config(void);

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

// Frame timing (milliseconds)
#define FRAME_DELAY_MS 1

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
