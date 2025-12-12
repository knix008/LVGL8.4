#include "../include/ui_helpers.h"
#include "../include/config.h"
#include "../include/style.h"
#include "../include/label.h"
#include "../include/state.h"
#include "../include/border.h"
#include <stdlib.h>

// ============================================================================
// BUTTON CREATION HELPERS
// ============================================================================

lv_obj_t* create_button_with_label(lv_obj_t* parent, const char* text,
                                   int width, int height, uint32_t bg_color,
                                   lv_event_cb_t callback, void* user_data) {
    // Create button
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, width, height);

    // Apply styling
    if (bg_color == 0) {
        apply_button_style(btn, app_state_get_button_color());
    } else {
        apply_button_style(btn, bg_color);
    }

    // Create label
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);

    // Apply button font for button labels
    lv_font_t *font = app_state_get_font_button();
    if (font) {
        lv_obj_set_style_text_font(label, font, 0);
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
    // Create button
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, width, height);

    // Apply styling
    if (bg_color == 0) {
        apply_button_style(btn, app_state_get_button_color());
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
    
    
    for (int i = 0; i < 5; i++) {
        const calendar_button_config_t* cfg = &config[i];
        
        // Create button
        lv_obj_t* btn = lv_btn_create(parent);
        lv_obj_set_size(btn, cfg->width, cfg->height);
        lv_obj_align(btn, LV_ALIGN_CENTER, cfg->x_offset, cfg->y_offset);
        apply_button_style(btn, cfg->bg_color ? cfg->bg_color : app_state_get_button_color());
        
        // Create label
        lv_obj_t* label = lv_label_create(btn);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        // Use button font for button labels
        if (app_state_get_font_button()) {
            lv_obj_set_style_text_font(label, app_state_get_font_button(), 0);
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
    
    
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, text);
    apply_label_style(label);
    
    if (use_font && app_state_get_font_20()) {
        lv_obj_set_style_text_font(label, app_state_get_font_20(), 0);
    }
    
    return label;
}

lv_obj_t* create_title_label(lv_obj_t* parent, const char* text) {
    return create_styled_label(parent, text, true);
}

// ============================================================================
// WARNING MESSAGE BOX - YELLOW BORDER AROUND SCREEN EDGES
// ============================================================================

#define WARNING_BORDER_WIDTH 8  // Width of the yellow border in pixels

/**
 * Timer callback to auto-close warning border
 */
static void warning_border_close_timer_cb(lv_timer_t* timer) {
    warning_border_t* border = (warning_border_t*)timer->user_data;
    if (border) {
        if (border->top) lv_obj_del(border->top);
        if (border->bottom) lv_obj_del(border->bottom);
        if (border->left) lv_obj_del(border->left);
        if (border->right) lv_obj_del(border->right);
        free(border);
    }
    lv_timer_del(timer);
}

/**
 * Event callback to close both message box and border when message box is closed
 */
static void warning_msgbox_close_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* mbox = lv_event_get_current_target(e);
    warning_msgbox_t* wmb = (warning_msgbox_t*)lv_event_get_user_data(e);
    
    if (code == LV_EVENT_VALUE_CHANGED) {
        // Button was clicked - remove border immediately and close the message box
        remove_border();
        if (mbox) {
            lv_msgbox_close(mbox);
        }
        // Free the structure after closing
        if (wmb) {
            free(wmb);
        }
    } else if (code == LV_EVENT_DELETE) {
        // Message box is being deleted - ensure border is removed and free structure
        remove_border();
        if (wmb) {
            free(wmb);
        }
    }
}

/**
 * Create a yellow warning border around the application window edges
 * @param parent Parent container (usually lv_scr_act())
 * @param message_key Language key for the warning message (optional, can be NULL)
 *                    If provided, a message label will be displayed at the top center.
 *                    The function will use get_label() to retrieve the localized message.
 * @param border_width Width of the yellow border in pixels (0 for default 8px)
 * @param auto_close_ms Auto-close timer in milliseconds (0 for no auto-close)
 * @return Pointer to warning_border_t structure containing the border objects, or NULL on failure
 */
warning_border_t* create_warning_box(lv_obj_t* parent, const char* message_key, 
                                     int border_width, uint32_t auto_close_ms) {
    // Default border width if not specified
    if (border_width <= 0) border_width = WARNING_BORDER_WIDTH;
    
    // Allocate memory for border structure
    warning_border_t* border = (warning_border_t*)malloc(sizeof(warning_border_t));
    if (!border) {
        return NULL;
    }
    border->top = NULL;
    border->bottom = NULL;
    border->left = NULL;
    border->right = NULL;
    
    // Create top border
    border->top = lv_obj_create(parent);
    lv_obj_set_size(border->top, SCREEN_WIDTH, border_width);
    lv_obj_set_pos(border->top, 0, 0);
    lv_obj_set_style_bg_color(border->top, lv_color_hex(0xFFFF00), 0);
    lv_obj_set_style_bg_opa(border->top, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(border->top, 0, 0);
    lv_obj_set_style_radius(border->top, 0, 0);
    lv_obj_clear_flag(border->top, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(border->top);
    
    // Create bottom border
    border->bottom = lv_obj_create(parent);
    lv_obj_set_size(border->bottom, SCREEN_WIDTH, border_width);
    lv_obj_set_pos(border->bottom, 0, SCREEN_HEIGHT - border_width);
    lv_obj_set_style_bg_color(border->bottom, lv_color_hex(0xFFFF00), 0);
    lv_obj_set_style_bg_opa(border->bottom, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(border->bottom, 0, 0);
    lv_obj_set_style_radius(border->bottom, 0, 0);
    lv_obj_clear_flag(border->bottom, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(border->bottom);
    
    // Create left border
    border->left = lv_obj_create(parent);
    lv_obj_set_size(border->left, border_width, SCREEN_HEIGHT);
    lv_obj_set_pos(border->left, 0, 0);
    lv_obj_set_style_bg_color(border->left, lv_color_hex(0xFFFF00), 0);
    lv_obj_set_style_bg_opa(border->left, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(border->left, 0, 0);
    lv_obj_set_style_radius(border->left, 0, 0);
    lv_obj_clear_flag(border->left, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(border->left);
    
    // Create right border
    border->right = lv_obj_create(parent);
    lv_obj_set_size(border->right, border_width, SCREEN_HEIGHT);
    lv_obj_set_pos(border->right, SCREEN_WIDTH - border_width, 0);
    lv_obj_set_style_bg_color(border->right, lv_color_hex(0xFFFF00), 0);
    lv_obj_set_style_bg_opa(border->right, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(border->right, 0, 0);
    lv_obj_set_style_radius(border->right, 0, 0);
    lv_obj_clear_flag(border->right, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(border->right);
    
    // Create optional message label at top center if message_key is provided
    if (message_key) {
        const char* message = get_label(message_key);
        if (!message) {
            message = message_key;
        }
        
        lv_obj_t* msg_label = lv_label_create(parent);
        lv_label_set_text(msg_label, message);
        lv_label_set_long_mode(msg_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(msg_label, SCREEN_WIDTH - (border_width * 2) - 20);
        lv_obj_set_pos(msg_label, border_width + 10, border_width + 10);
        
        // Style the message text - dark color for visibility on yellow background
        lv_obj_set_style_text_color(msg_label, lv_color_hex(0x000000), 0);
        lv_obj_set_style_text_align(msg_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_bg_color(msg_label, lv_color_hex(0xFFFF00), 0);
        lv_obj_set_style_bg_opa(msg_label, LV_OPA_COVER, 0);
        lv_obj_set_style_pad_all(msg_label, 5, 0);
        lv_obj_set_style_radius(msg_label, 4, 0);
        
        // Apply label font
        lv_font_t *font = app_state_get_font_label();
        if (font) {
            lv_obj_set_style_text_font(msg_label, font, 0);
        } else if (app_state_get_font_20()) {
            lv_obj_set_style_text_font(msg_label, app_state_get_font_20(), 0);
        }
        
        lv_obj_move_foreground(msg_label);
    }
    
    // Auto-close timer if specified
    if (auto_close_ms > 0) {
        lv_timer_t* timer = lv_timer_create(warning_border_close_timer_cb, auto_close_ms, border);
        lv_timer_set_repeat_count(timer, 1);
    }
    
    return border;
}

/**
 * Create a warning message box with orange border around screen edges
 * This function creates both a standard LVGL message box and an orange border around the screen.
 * Uses the existing border system (show_orange_border/remove_border).
 * @param parent Parent container (usually lv_scr_act() or NULL for screen)
 * @param title_key Language key for the message box title (e.g., "common.warning")
 * @param message_key Language key for the message box content (e.g., "menu_screen.status_bar_full")
 * @param button_texts Array of button texts (e.g., {"OK", ""} or {get_label("common.ok"), ""})
 * @param add_close_btn Whether to add a close button
 * @param border_width Width of the orange border in pixels (0 for default, ignored - uses existing border system)
 * @return Pointer to warning_msgbox_t structure, or NULL on failure
 */
warning_msgbox_t* create_warning_msgbox_with_border(lv_obj_t* parent, 
                                                     const char* title_key,
                                                     const char* message_key,
                                                     const char* button_texts[],
                                                     bool add_close_btn,
                                                     int border_width) {
    (void)border_width; // Unused - uses existing border system with default width
    
    // Show the orange border using the existing border system
    show_orange_border();
    
    // Get localized strings
    const char* title = get_label(title_key);
    if (!title) title = title_key;
    
    const char* message = get_label(message_key);
    if (!message) message = message_key;
    
    // Create the message box
    lv_obj_t* mbox = lv_msgbox_create(parent, title, message, button_texts, add_close_btn);
    if (!mbox) {
        // Clean up border if message box creation fails
        remove_border();
        return NULL;
    }
    
    // Center and style the message box
    lv_obj_center(mbox);
    lv_obj_move_foreground(mbox);
    lv_obj_set_width(mbox, 280);
    lv_obj_set_style_bg_color(mbox, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(mbox, LV_OPA_70, 0);
    lv_obj_set_style_border_color(mbox, lv_color_hex(0xFF6B00), 0);
    lv_obj_set_style_border_width(mbox, 2, 0);
    
    // Style the title
    lv_obj_t* title_label = lv_msgbox_get_title(mbox);
    if (title_label) {
        if (app_state_get_font_24_bold()) {
            lv_obj_set_style_text_font(title_label, app_state_get_font_24_bold(), 0);
        }
        lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFAA00), 0);
    }
    
    // Style the text
    lv_obj_t* text_label = lv_msgbox_get_text(mbox);
    if (text_label) {
        if (app_state_get_font_20()) {
            lv_obj_set_style_text_font(text_label, app_state_get_font_20(), 0);
        }
        lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), 0);
    }
    
    // Style the buttons
    lv_obj_t* btns_obj = lv_msgbox_get_btns(mbox);
    if (btns_obj) {
        lv_obj_set_height(btns_obj, 50);
        if (app_state_get_font_20()) {
            lv_obj_set_style_text_font(btns_obj, app_state_get_font_20(), 0);
        }
    }
    
    // Allocate structure to hold message box reference
    warning_msgbox_t* wmb = (warning_msgbox_t*)malloc(sizeof(warning_msgbox_t));
    if (!wmb) {
        // Clean up on failure
        lv_obj_del(mbox);
        remove_border();
        return NULL;
    }
    
    // Set border to NULL since we're using the global border system
    wmb->border = NULL;
    wmb->msgbox = mbox;
    
    // Add event callback to clean up border when message box is closed
    lv_obj_add_event_cb(mbox, warning_msgbox_close_cb, LV_EVENT_VALUE_CHANGED, wmb);
    if (add_close_btn) {
        lv_obj_add_event_cb(mbox, warning_msgbox_close_cb, LV_EVENT_DELETE, wmb);
    }
    
    return wmb;
}