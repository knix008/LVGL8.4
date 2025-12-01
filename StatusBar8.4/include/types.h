#ifndef TYPES_H
#define TYPES_H

#include "lvgl/lvgl.h"

// ============================================================================
// APPLICATION STATE STRUCTURES
// ============================================================================

#define MAX_STATUS_ICONS 4

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

#endif
