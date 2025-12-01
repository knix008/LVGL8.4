#include "../include/style.h"
#include "../include/config.h"
#include "../include/types.h"

// ============================================================================
// STYLE HELPER FUNCTIONS
// ============================================================================

// Circular hit test event handler
static void circle_hit_test_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_HIT_TEST) {
        lv_obj_t *obj = lv_event_get_target(e);
        lv_hit_test_info_t *info = lv_event_get_param(e);

        if (!info) return;

        // Get button coordinates
        lv_area_t coords;
        lv_obj_get_coords(obj, &coords);

        // Calculate center and radius
        int32_t center_x = (coords.x1 + coords.x2) / 2;
        int32_t center_y = (coords.y1 + coords.y2) / 2;
        int32_t radius = (coords.x2 - coords.x1) / 2;

        // Calculate distance from center to the test point
        int32_t dx = info->point->x - center_x;
        int32_t dy = info->point->y - center_y;
        int32_t dist_sq = dx * dx + dy * dy;
        int32_t radius_sq = radius * radius;

        // Set hit test result: true if inside circle, false if outside
        info->res = (dist_sq <= radius_sq);
    }
}

void apply_button_style(lv_obj_t *btn, uint32_t bg_color) {
    lv_obj_set_style_bg_color(btn, lv_color_hex(bg_color), 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(COLOR_BORDER), 0);
}

void apply_circle_button_style(lv_obj_t *btn, uint32_t bg_color) {
    (void)bg_color;  // Unused - we want transparent background
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);  // Transparent background
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);  // No shadow

    // Add visual feedback for pressed state
    lv_obj_set_style_bg_opa(btn, LV_OPA_50, LV_STATE_PRESSED);  // Semi-transparent background when pressed
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x808080), LV_STATE_PRESSED);  // Gray overlay
    lv_obj_set_style_shadow_width(btn, 8, LV_STATE_PRESSED);  // Add shadow when pressed
    lv_obj_set_style_shadow_color(btn, lv_color_hex(0x000000), LV_STATE_PRESSED);  // Black shadow
    lv_obj_set_style_shadow_opa(btn, LV_OPA_40, LV_STATE_PRESSED);  // Shadow opacity

    lv_obj_add_flag(btn, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_add_event_cb(btn, circle_hit_test_event_cb, LV_EVENT_HIT_TEST, NULL);
}

void apply_label_style(lv_obj_t *label) {
    extern AppState app_state;
    lv_obj_set_style_text_color(label, lv_color_hex(COLOR_TEXT), 0);
    if (app_state.font_20) {
        lv_obj_set_style_text_font(label, app_state.font_20, 0);
    }
}

void apply_bar_style(lv_obj_t *bar, uint32_t bg_color) {
    lv_obj_set_style_bg_color(bar, lv_color_hex(bg_color), 0);
    lv_obj_set_style_bg_opa(bar, COLOR_TRANSPARENT, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 5, 0);
}
