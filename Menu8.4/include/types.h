#ifndef TYPES_H
#define TYPES_H

#include "lvgl/lvgl.h"

// ============================================================================
// APPLICATION STATE STRUCTURES
// ============================================================================

typedef struct {
    lv_obj_t *screen;
    lv_obj_t *title_bar;
    lv_obj_t *title_label;
    lv_obj_t *current_title_label;
    lv_font_t *font_20;
} AppState;

typedef struct {
    lv_obj_t *screen;
    int screen_id;
} ScreenState;

#endif
