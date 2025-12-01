#include <stdio.h>
#include "../include/menu.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"

// ============================================================================
// MENU CONFIGURATION ARRAY
// ============================================================================

const MenuItem MENU_ITEMS[MAX_STATUS_ICONS] = {
    {"관리자 설정", IMG_CONFIG, "admin", SCREEN_ADMIN, admin_btn_callback},
    {"네트워크 설정", IMG_NETWORK, "network", SCREEN_NETWORK, network_btn_callback},
    {"한글 입력", IMG_KOREAN, "korean_input", SCREEN_KOREAN_INPUT, korean_input_btn_callback},
    {"Info", IMG_INFO, "info", SCREEN_INFO, info_btn_callback},
    {"Face", IMG_FACE, "face", SCREEN_FACE, settings_btn_callback}
};

// ============================================================================
// PLUS/MINUS BUTTON STATE MANAGEMENT
// ============================================================================

typedef struct {
    lv_obj_t *button;
    bool is_plus;  // true for plus, false for minus
    int item_index;
} plus_minus_btn_data_t;

static plus_minus_btn_data_t plus_minus_buttons[MENU_ITEMS_COUNT];

// ============================================================================
// MENU BUTTON VISUAL EFFECTS
// ============================================================================

static void menu_btn_visual_effect(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    // Get the menu icon image which is stored as user_data
    lv_obj_t *img = (lv_obj_t *)lv_event_get_user_data(e);
    if (!img) return;

    if (code == LV_EVENT_PRESSED) {
        // Scale down and reduce opacity when pressed
        lv_img_set_zoom(img, ZOOM_PRESSED);
        lv_obj_set_style_img_opa(img, LV_OPA_60, 0);
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        // Restore normal size and opacity when released
        lv_img_set_zoom(img, ZOOM_NORMAL);
        lv_obj_set_style_img_opa(img, LV_OPA_COVER, 0);
    }
}

// ============================================================================
// PLUS/MINUS BUTTON VISUAL EFFECTS
// ============================================================================

static void plus_minus_visual_effect(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    // Get the plus/minus image button
    lv_obj_t *img_btn = lv_event_get_current_target(e);
    if (!img_btn) return;

    if (code == LV_EVENT_PRESSED) {
        // Scale down and reduce opacity when pressed
        lv_img_set_zoom(img_btn, ZOOM_PRESSED);
        lv_obj_set_style_img_opa(img_btn, LV_OPA_60, 0);
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        // Restore normal size and opacity when released
        lv_img_set_zoom(img_btn, ZOOM_NORMAL);
        lv_obj_set_style_img_opa(img_btn, LV_OPA_COVER, 0);
    }
}

// ============================================================================

static void plus_minus_btn_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        // Get the button data
        plus_minus_btn_data_t *btn_data = (plus_minus_btn_data_t *)lv_event_get_user_data(e);
        if (!btn_data || !btn_data->button) return;

        // Toggle the button state
        if (btn_data->is_plus) {
            // Change from plus to minus - add icon to status bar
            lv_img_set_src(btn_data->button, IMG_MINUS);
            btn_data->is_plus = false;
            add_status_bar_icon(btn_data->item_index, MENU_ITEMS[btn_data->item_index].icon_path);
        } else {
            // Change from minus to plus - remove icon from status bar
            lv_img_set_src(btn_data->button, IMG_PLUS);
            btn_data->is_plus = true;
            remove_status_bar_icon(btn_data->item_index);
        }

        // Save configuration
        save_status_bar_config();
    }
}

// ============================================================================
// MENU SCREEN COMPONENTS
// ============================================================================

static lv_obj_t *create_menu_content(lv_obj_t *parent) {
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, SCREEN_WIDTH, SCREEN_HEIGHT - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, TITLE_BAR_HEIGHT);
    lv_obj_set_style_bg_color(content, lv_color_hex(COLOR_BG_DARK), 0);
    lv_obj_set_style_border_width(content, 0, 0);

    // Allow only vertical scrolling
    lv_obj_set_scroll_dir(content, LV_DIR_VER);

    // Create menu buttons using MENU_ITEMS configuration
    for (int i = 0; i < MENU_ITEMS_COUNT; i++) {

        lv_obj_t *btn = lv_btn_create(content);
        lv_obj_set_size(btn, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, OFFSET_BUTTON_START_Y + i * (MENU_BUTTON_HEIGHT + MENU_BUTTON_MARGIN));
        apply_button_style(btn, COLOR_BUTTON_BG);

        // Create image on the left side of the button
        lv_obj_t *img = lv_img_create(btn);
        lv_img_set_src(img, MENU_ITEMS[i].icon_path);
        lv_obj_align(img, LV_ALIGN_LEFT_MID, ICON_IMAGE_OFFSET, 0);

        // Create label on the right side of the image
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, MENU_ITEMS[i].label);
        apply_label_style(label);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, LABEL_OFFSET_X, 0);

        // Create plus image button on the right side of the button
        lv_obj_t *plus_btn = lv_img_create(btn);
        lv_img_set_src(plus_btn, IMG_PLUS);
        lv_obj_align(plus_btn, LV_ALIGN_RIGHT_MID, -ICON_IMAGE_OFFSET, 0);
        lv_obj_add_flag(plus_btn, LV_OBJ_FLAG_CLICKABLE);
        
        // Initialize plus/minus button data
        plus_minus_buttons[i].button = plus_btn;
        plus_minus_buttons[i].item_index = i;
        
        // Sync with loaded configuration
        extern AppState app_state;
        if (app_state.menu_item_selected[i]) {
            // Item is selected - show minus button
            lv_img_set_src(plus_btn, IMG_MINUS);
            plus_minus_buttons[i].is_plus = false;
        } else {
            // Item is not selected - show plus button
            lv_img_set_src(plus_btn, IMG_PLUS);
            plus_minus_buttons[i].is_plus = true;
        }
        
        lv_obj_add_event_cb(plus_btn, plus_minus_btn_callback, LV_EVENT_CLICKED, &plus_minus_buttons[i]);
        
        // Add visual effect event callbacks for plus/minus button
        lv_obj_add_event_cb(plus_btn, plus_minus_visual_effect, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(plus_btn, plus_minus_visual_effect, LV_EVENT_RELEASED, NULL);
        lv_obj_add_event_cb(plus_btn, plus_minus_visual_effect, LV_EVENT_PRESS_LOST, NULL);

        // Add visual effect event callbacks (pass the menu icon image as user_data)
        lv_obj_add_event_cb(btn, menu_btn_visual_effect, LV_EVENT_PRESSED, img);
        lv_obj_add_event_cb(btn, menu_btn_visual_effect, LV_EVENT_RELEASED, img);
        lv_obj_add_event_cb(btn, menu_btn_visual_effect, LV_EVENT_PRESS_LOST, img);

        // Add navigation event handler using callback from configuration
        if (MENU_ITEMS[i].callback) {
            lv_obj_add_event_cb(btn, MENU_ITEMS[i].callback, LV_EVENT_CLICKED, NULL);
        }
    }

    return content;
}

// ============================================================================
// MENU SCREEN CREATION
// ============================================================================

/**
 * Creates the main menu screen with navigation buttons.
 * Buttons are configured from the MENU_ITEMS array.
 */
void create_menu_screen(void) {
    lv_obj_t *menu_screen = create_screen_base(SCREEN_MENU);

    create_standard_title_bar(menu_screen, SCREEN_MENU);
    create_menu_content(menu_screen);
    create_standard_status_bar(menu_screen);

    finalize_screen(menu_screen, SCREEN_MENU);
}
