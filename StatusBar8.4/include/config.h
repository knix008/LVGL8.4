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

// ============================================================================
// IMAGE PATHS
// ============================================================================

#define IMG_BACK_BUTTON "A:assets/images/backbutton.png"
#define IMG_CONFIG "A:assets/images/config.png"
#define IMG_SETUP "A:assets/images/setup.png"
#define IMG_INFO "A:assets/images/Info.png"
#define IMG_NETWORK "A:assets/images/network.png"
#define IMG_KOREAN "A:assets/images/korean.png"
#define IMG_PLUS "A:assets/images/plus_big.png"
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
#define MENU_ITEMS_COUNT 4
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
    SCREEN_KOREAN_INPUT = 5
};

#endif
