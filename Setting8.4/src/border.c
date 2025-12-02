#include "../include/border.h"
#include "../include/config.h"

// ============================================================================
// GLOBAL STATE
// ============================================================================

static lv_obj_t *border_rect = NULL;

// ============================================================================
// BORDER API IMPLEMENTATION
// ============================================================================

/**
 * Shows a colored rectangle border on the active screen.
 * Creates a full-screen overlay with a transparent background and colored border.
 */
void show_border(uint32_t color, uint8_t width) {
    lv_obj_t *active_screen = lv_scr_act();
    
    if (active_screen && border_rect == NULL) {
        // Create the border rectangle
        border_rect = lv_obj_create(active_screen);
        lv_obj_set_size(border_rect, SCREEN_WIDTH, SCREEN_HEIGHT);
        lv_obj_align(border_rect, LV_ALIGN_TOP_LEFT, 0, 0);
        
        // Set transparent background with colored border only
        lv_obj_set_style_bg_color(border_rect, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(border_rect, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_color(border_rect, lv_color_hex(color), 0);
        lv_obj_set_style_border_width(border_rect, width, 0);
        
        // Disable scroll and interaction
        lv_obj_set_scrollbar_mode(border_rect, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(border_rect, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(border_rect, LV_OBJ_FLAG_CLICKABLE);
        
        // Move to foreground to ensure visibility
        lv_obj_move_foreground(border_rect);
    }
}

/**
 * Shows a green rectangle border (convenience function).
 */
void show_green_border(void) {
    show_border(BORDER_COLOR_GREEN, BORDER_WIDTH_DEFAULT);
}

/**
 * Shows a red rectangle border (convenience function).
 */
void show_red_border(void) {
    show_border(BORDER_COLOR_RED, BORDER_WIDTH_DEFAULT);
}

/**
 * Shows a blue rectangle border (convenience function).
 */
void show_blue_border(void) {
    show_border(BORDER_COLOR_BLUE, BORDER_WIDTH_DEFAULT);
}

/**
 * Shows a yellow rectangle border (convenience function).
 */
void show_yellow_border(void) {
    show_border(BORDER_COLOR_YELLOW, BORDER_WIDTH_DEFAULT);
}

/**
 * Shows an orange rectangle border (convenience function).
 */
void show_orange_border(void) {
    show_border(BORDER_COLOR_ORANGE, BORDER_WIDTH_DEFAULT);
}

/**
 * Shows a purple rectangle border (convenience function).
 */
void show_purple_border(void) {
    show_border(BORDER_COLOR_PURPLE, BORDER_WIDTH_DEFAULT);
}

/**
 * Shows a white rectangle border (convenience function).
 */
void show_white_border(void) {
    show_border(BORDER_COLOR_WHITE, BORDER_WIDTH_DEFAULT);
}

/**
 * Shows a border with thin width (convenience function).
 */
void show_thin_border(uint32_t color) {
    show_border(color, BORDER_WIDTH_THIN);
}

/**
 * Shows a border with thick width (convenience function).
 */
void show_thick_border(uint32_t color) {
    show_border(color, BORDER_WIDTH_THICK);
}

/**
 * Shows a border with extra thick width (convenience function).
 */
void show_extra_thick_border(uint32_t color) {
    show_border(color, BORDER_WIDTH_EXTRA_THICK);
}

/**
 * Removes the rectangle border from the screen.
 */
void remove_border(void) {
    if (border_rect != NULL) {
        lv_obj_del(border_rect);
        border_rect = NULL;
    }
}

/**
 * Removes the green rectangle border (alias for backward compatibility).
 */
void remove_green_border(void) {
    remove_border();
}
