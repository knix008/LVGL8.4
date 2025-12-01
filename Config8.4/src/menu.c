#include <stdio.h>
#include "../include/menu.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/screen_components.h"
#include "../include/navigation.h"

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
        lv_img_set_zoom(img, 230);  // Scale to 90% (256 = 100%)
        lv_obj_set_style_img_opa(img, LV_OPA_60, 0);  // Reduce opacity to 60%
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        // Restore normal size and opacity when released
        lv_img_set_zoom(img, 256);  // Restore to 100%
        lv_obj_set_style_img_opa(img, LV_OPA_COVER, 0);  // Restore full opacity
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
        lv_img_set_zoom(img_btn, 230);  // Scale to 90% (256 = 100%)
        lv_obj_set_style_img_opa(img_btn, LV_OPA_60, 0);  // Reduce opacity to 60%
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        // Restore normal size and opacity when released
        lv_img_set_zoom(img_btn, 256);  // Restore to 100%
        lv_obj_set_style_img_opa(img_btn, LV_OPA_COVER, 0);  // Restore full opacity
    }
}

// ============================================================================

static void plus_minus_btn_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        // Get the button data
        plus_minus_btn_data_t *btn_data = (plus_minus_btn_data_t *)lv_event_get_user_data(e);
        if (!btn_data || !btn_data->button) return;

        // Menu item to icon path mapping
        const char *menu_icons[] = {IMG_CONFIG, IMG_NETWORK, IMG_KOREAN, IMG_INFO};

        // Toggle the button state
        if (btn_data->is_plus) {
            // Change from plus to minus - add icon to status bar
            lv_img_set_src(btn_data->button, IMG_MINUS);
            btn_data->is_plus = false;
            add_status_bar_icon(btn_data->item_index, menu_icons[btn_data->item_index]);
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

    // Create menu buttons with images and labels
    const char *menu_labels[] = {"관리자 설정", "네트워크 설정", "한글 입력", "Info"};
    const char *menu_images[] = {IMG_CONFIG, IMG_NETWORK, IMG_KOREAN, IMG_INFO};

    for (int i = 0; i < MENU_ITEMS_COUNT; i++) {

        lv_obj_t *btn = lv_btn_create(content);
        lv_obj_set_size(btn, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, OFFSET_BUTTON_START_Y + i * (MENU_BUTTON_HEIGHT + MENU_BUTTON_MARGIN));
        apply_button_style(btn, COLOR_BUTTON_BG);

        // Create image on the left side of the button
        lv_obj_t *img = lv_img_create(btn);
        lv_img_set_src(img, menu_images[i]);
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 10, 0);

        // Create label on the right side of the image
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, menu_labels[i]);
        apply_label_style(label);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 60, 0);

        // Create plus image button on the right side of the button
        lv_obj_t *plus_btn = lv_img_create(btn);
        lv_img_set_src(plus_btn, IMG_PLUS);
        lv_obj_align(plus_btn, LV_ALIGN_RIGHT_MID, -10, 0);
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

        // Add navigation event handlers
        if (i == 0) {
            lv_obj_add_event_cb(btn, admin_btn_callback, LV_EVENT_CLICKED, NULL);
        } else if (i == 1) {
            lv_obj_add_event_cb(btn, network_btn_callback, LV_EVENT_CLICKED, NULL);
        } else if (i == 2) {
            lv_obj_add_event_cb(btn, korean_input_btn_callback, LV_EVENT_CLICKED, NULL);
        } else if (i == 3) {
            lv_obj_add_event_cb(btn, info_btn_callback, LV_EVENT_CLICKED, NULL);
        }
    }

    return content;
}

// ============================================================================
// MENU SCREEN CREATION
// ============================================================================

void create_menu_screen(void) {
    lv_obj_t *menu_screen = create_screen_base(SCREEN_MENU);

    create_standard_title_bar(menu_screen, SCREEN_MENU);
    create_menu_content(menu_screen);
    create_standard_status_bar(menu_screen);

    finalize_screen(menu_screen, SCREEN_MENU);
}
