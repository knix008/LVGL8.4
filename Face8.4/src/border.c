#include "../include/border.h"
#include "../include/config.h"

// ============================================================================
// GLOBAL STATE
// ============================================================================

static lv_obj_t *green_border_rect = NULL;

// ============================================================================
// GREEN BORDER API IMPLEMENTATION
// ============================================================================

/**
 * Shows a green rectangle border on the active screen.
 * Creates a full-screen overlay with a transparent background and green border.
 */
void show_green_border(void) {
    lv_obj_t *active_screen = lv_scr_act();
    
    if (active_screen && green_border_rect == NULL) {
        // Create the green border rectangle
        green_border_rect = lv_obj_create(active_screen);
        lv_obj_set_size(green_border_rect, SCREEN_WIDTH, SCREEN_HEIGHT);
        lv_obj_align(green_border_rect, LV_ALIGN_TOP_LEFT, 0, 0);
        
        // Set transparent background with green border only
        lv_obj_set_style_bg_color(green_border_rect, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(green_border_rect, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_color(green_border_rect, lv_color_hex(0x00FF00), 0);
        lv_obj_set_style_border_width(green_border_rect, 8, 0);
        
        // Disable scroll and interaction
        lv_obj_set_scrollbar_mode(green_border_rect, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(green_border_rect, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(green_border_rect, LV_OBJ_FLAG_CLICKABLE);
        
        // Move to foreground to ensure visibility
        lv_obj_move_foreground(green_border_rect);
    }
}

/**
 * Removes the green rectangle border from the screen.
 */
void remove_green_border(void) {
    if (green_border_rect != NULL) {
        lv_obj_del(green_border_rect);
        green_border_rect = NULL;
    }
}
