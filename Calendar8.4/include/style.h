#ifndef STYLE_H
#define STYLE_H

#include "lvgl/lvgl.h"

// ============================================================================
// STYLE HELPER FUNCTIONS
// ============================================================================

void apply_button_style(lv_obj_t *btn, uint32_t bg_color);
void apply_circle_button_style(lv_obj_t *btn, uint32_t bg_color);
void apply_label_style(lv_obj_t *label);
void apply_button_label_style(lv_obj_t *label);
void apply_bar_style(lv_obj_t *bar, uint32_t bg_color);

#endif
