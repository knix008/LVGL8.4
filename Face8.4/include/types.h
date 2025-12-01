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
    lv_obj_t *status_bar;  // Reference to the status bar
    bool menu_item_selected[MAX_STATUS_ICONS];  // Track which menu items are selected
    lv_obj_t *status_icons[MAX_STATUS_ICONS];  // Status bar icon objects
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
