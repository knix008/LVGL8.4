#include "../include/ui_helpers.h"
#include "../include/config.h"
#include "../include/style.h"
#include "../include/label.h"

// ============================================================================
// BUTTON CREATION HELPERS
// ============================================================================

lv_obj_t* create_button_with_label(lv_obj_t* parent, const char* text, 
                                   int width, int height, uint32_t bg_color,
                                   lv_event_cb_t callback, void* user_data) {
    extern AppState app_state;
    
    // Create button
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, width, height);
    
    // Apply styling
    if (bg_color == 0) {
        apply_button_style(btn, app_state.button_color);
    } else {
        apply_button_style(btn, bg_color);
    }
    
    // Create label
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    
    // Apply font if available
    if (app_state.font_20) {
        lv_obj_set_style_text_font(label, app_state.font_20, 0);
    }
    
    lv_obj_center(label);
    
    // Add event callback if provided
    if (callback) {
        lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, user_data);
    }
    
    return btn;
}

lv_obj_t* create_close_button(lv_obj_t* parent, lv_event_cb_t callback, void* user_data) {
    // Create button with Korean input style
    lv_obj_t* close_btn = lv_btn_create(parent);
    lv_obj_set_size(close_btn, 40, 40);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, -5, 5);
    apply_circle_button_style(close_btn, 0);

    // Create cancel image
    lv_obj_t* close_img = lv_img_create(close_btn);
    lv_img_set_src(close_img, IMG_CANCEL);
    lv_obj_align(close_img, LV_ALIGN_CENTER, 0, 0);
    
    // Add event callback if provided
    if (callback) {
        lv_obj_add_event_cb(close_btn, callback, LV_EVENT_CLICKED, user_data);
    }
    
    return close_btn;
}

lv_obj_t* create_nav_button(lv_obj_t* parent, const char* text, 
                            int width, int height, uint32_t bg_color,
                            lv_event_cb_t callback, void* user_data) {
    extern AppState app_state;
    
    // Create button
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, width, height);
    
    // Apply styling
    if (bg_color == 0) {
        apply_button_style(btn, app_state.button_color);
    } else {
        apply_button_style(btn, bg_color);
    }
    
    // Create label
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_center(label);
    
    // Add event callback if provided
    if (callback) {
        lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, user_data);
    }
    
    return btn;
}

// ============================================================================
// POPUP CREATION HELPERS
// ============================================================================

lv_obj_t* create_popup_overlay(lv_obj_t* parent) {
    // Create full screen dark overlay
    lv_obj_t* popup = lv_obj_create(parent);
    lv_obj_set_size(popup, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_pos(popup, 0, 0);
    lv_obj_set_style_bg_color(popup, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(popup, LV_OPA_50, 0);
    lv_obj_set_style_border_width(popup, 0, 0);
    lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(popup);
    
    return popup;
}

lv_obj_t* create_popup_container(lv_obj_t* overlay_parent, int width, int height) {
    extern AppState app_state;
    
    // Create centered container
    lv_obj_t* container = lv_obj_create(overlay_parent);
    lv_obj_set_size(container, width, height);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_70, 0);
    lv_obj_set_style_border_color(container, lv_color_hex(get_button_border_color()), 0);
    lv_obj_set_style_border_width(container, 2, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    
    return container;
}

// ============================================================================
// CALENDAR HELPERS
// ============================================================================

void create_calendar_nav_row(lv_obj_t* parent, const calendar_button_config_t config[5],
                             lv_obj_t* labels[5], lv_obj_t* buttons[5]) {
    extern AppState app_state;
    
    for (int i = 0; i < 5; i++) {
        const calendar_button_config_t* cfg = &config[i];
        
        // Create button
        lv_obj_t* btn = lv_btn_create(parent);
        lv_obj_set_size(btn, cfg->width, cfg->height);
        lv_obj_align(btn, LV_ALIGN_CENTER, cfg->x_offset, cfg->y_offset);
        apply_button_style(btn, cfg->bg_color ? cfg->bg_color : app_state.button_color);
        
        // Create label
        lv_obj_t* label = lv_label_create(btn);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        if (app_state.font_20) {
            lv_obj_set_style_text_font(label, app_state.font_20, 0);
        }
        lv_obj_center(label);
        
        // Add callback if provided
        if (cfg->callback) {
            lv_obj_add_event_cb(btn, cfg->callback, LV_EVENT_CLICKED, cfg->user_data);
        }
        
        // Store references if arrays provided
        if (labels) labels[i] = label;
        if (buttons) buttons[i] = btn;
    }
}

// ============================================================================
// LABEL CREATION HELPERS
// ============================================================================

lv_obj_t* create_styled_label(lv_obj_t* parent, const char* text, bool use_font) {
    extern AppState app_state;
    
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, text);
    apply_label_style(label);
    
    if (use_font && app_state.font_20) {
        lv_obj_set_style_text_font(label, app_state.font_20, 0);
    }
    
    return label;
}

lv_obj_t* create_title_label(lv_obj_t* parent, const char* text) {
    return create_styled_label(parent, text, true);
}