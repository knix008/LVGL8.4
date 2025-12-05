#ifndef TYPES_H
#define TYPES_H

#include "lvgl/lvgl.h"

// Forward declaration
typedef void (*menu_callback_fn)(lv_event_t *e);

// ============================================================================
// MENU CONFIGURATION
// ============================================================================

#define MAX_STATUS_ICONS 5

typedef struct {
    const char *label;        // Display label (Korean/English)
    const char *icon_path;    // Path to icon image
    const char *config_key;   // Key for config file
    int screen_id;            // Associated screen ID
    menu_callback_fn callback; // Navigation callback function
} MenuItem;

// ============================================================================
// APPLICATION STATE STRUCTURES
// ============================================================================

typedef struct {
    lv_obj_t *screen;
    lv_obj_t *title_bar;
    lv_obj_t *title_label;
    lv_obj_t *current_title_label;
    lv_font_t *font_20;
    lv_font_t *font_button;   // 20px font for buttons
    lv_font_t *font_24_bold;  // Bold 24pt font for welcome message
    lv_obj_t *status_bar;  // Reference to the status bar
    bool menu_item_selected[MAX_STATUS_ICONS];  // Track which menu items are selected
    lv_obj_t *status_icons[MAX_STATUS_ICONS];  // Status bar icon objects
    uint32_t bg_color;  // Current background color
    uint32_t title_bar_color;  // Current title bar color
    uint32_t status_bar_color;  // Current status bar color
    uint32_t button_color;  // Current button background color
    uint32_t button_border_color;  // Current button border color
    char current_language[4];  // Current language code ("ko" or "en")
    int font_size_title_bar;   // Font size for title bar
    int font_size_label;       // Font size for labels
    int font_size_button_label; // Font size for button labels
    int font_size_bold;        // Font size for bold text
    char font_name_title[64];       // Font name for title
    char font_name_status_bar[64];  // Font name for status bar
    char font_name_button_label[64]; // Font name for button label
    lv_obj_t *menu_button_label;  // Reference to menu button label for language updates
    lv_obj_t *exit_button_label;  // Reference to exit button label for language updates
    lv_obj_t *welcome_message_label;  // Reference to welcome message label for updates
} AppState;

typedef struct {
    lv_obj_t *screen;
    int screen_id;
} ScreenState;

// ============================================================================
// GLOBAL MENU CONFIGURATION
// ============================================================================

extern const MenuItem MENU_ITEMS[MAX_STATUS_ICONS];

#endif
