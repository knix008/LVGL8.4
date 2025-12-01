#include "../include/screen_components.h"
#include "../include/navigation.h"
#include "../include/config.h"
#include "../include/style.h"
#include "../include/screen.h"

// ============================================================================
// STANDARD TITLE BAR
// ============================================================================

lv_obj_t *create_standard_title_bar(lv_obj_t *parent, int screen_id) {
    lv_obj_t *title_bar = lv_obj_create(parent);
    lv_obj_set_size(title_bar, SCREEN_WIDTH, TITLE_BAR_HEIGHT);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 0);
    apply_bar_style(title_bar, COLOR_BG_TITLE);

    // Back button (circular)
    lv_obj_t *back_btn = lv_btn_create(title_bar);
    lv_obj_set_size(back_btn, TITLE_BAR_HEIGHT - 20, TITLE_BAR_HEIGHT - 20);
    lv_obj_align(back_btn, LV_ALIGN_LEFT_MID, PADDING_HORIZONTAL, 0);
    apply_circle_button_style(back_btn, COLOR_BUTTON_BACK);

    lv_obj_t *back_img = lv_img_create(back_btn);
    lv_img_set_src(back_img, IMG_BACK_BUTTON);
    lv_obj_align(back_img, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(back_btn, back_btn_callback, LV_EVENT_CLICKED, NULL);

    // Title label
    lv_obj_t *title_label = lv_label_create(title_bar);
    lv_label_set_text(title_label, "");
    apply_label_style(title_label);
    lv_obj_align(title_label, LV_ALIGN_LEFT_MID, (TITLE_BAR_HEIGHT - 20) + PADDING_HORIZONTAL * 2, 0);

    // Store the title label
    extern AppState app_state;
    app_state.current_title_label = title_label;

    // Update the title with breadcrumb path
    update_title_bar_location(screen_id);

    return title_bar;
}

// ============================================================================
// STANDARD STATUS BAR
// ============================================================================

lv_obj_t *create_standard_status_bar(lv_obj_t *parent) {
    lv_obj_t *status_bar = lv_obj_create(parent);
    lv_obj_set_size(status_bar, SCREEN_WIDTH, STATUS_BAR_HEIGHT);
    lv_obj_align(status_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    apply_bar_style(status_bar, COLOR_BG_TITLE);

    // Image button configuration
    int img_btn_size = 40;
    int spacing = 10;
    int start_x = PADDING_HORIZONTAL;

    // Config button with image
    lv_obj_t *config_btn = lv_btn_create(status_bar);
    lv_obj_set_size(config_btn, img_btn_size, img_btn_size);
    lv_obj_set_pos(config_btn, start_x, (STATUS_BAR_HEIGHT - img_btn_size) / 2);
    apply_circle_button_style(config_btn, COLOR_BUTTON_BACK);

    lv_obj_t *config_img = lv_img_create(config_btn);
    lv_img_set_src(config_img, IMG_CONFIG);
    lv_obj_center(config_img);
    lv_obj_add_event_cb(config_btn, admin_btn_callback, LV_EVENT_CLICKED, NULL);

    // Korean input button with image
    lv_obj_t *korean_btn = lv_btn_create(status_bar);
    lv_obj_set_size(korean_btn, img_btn_size, img_btn_size);
    lv_obj_set_pos(korean_btn, start_x + img_btn_size + spacing, (STATUS_BAR_HEIGHT - img_btn_size) / 2);
    apply_circle_button_style(korean_btn, COLOR_BUTTON_BACK);

    lv_obj_t *korean_img = lv_img_create(korean_btn);
    lv_img_set_src(korean_img, IMG_KOREAN);
    lv_obj_center(korean_img);
    lv_obj_add_event_cb(korean_btn, korean_input_btn_callback, LV_EVENT_CLICKED, NULL);

    // Info button with image
    lv_obj_t *info_btn = lv_btn_create(status_bar);
    lv_obj_set_size(info_btn, img_btn_size, img_btn_size);
    lv_obj_set_pos(info_btn, start_x + (img_btn_size + spacing) * 2, (STATUS_BAR_HEIGHT - img_btn_size) / 2);
    apply_circle_button_style(info_btn, COLOR_BUTTON_BACK);

    lv_obj_t *info_img = lv_img_create(info_btn);
    lv_img_set_src(info_img, IMG_INFO);
    lv_obj_center(info_img);
    lv_obj_add_event_cb(info_btn, info_btn_callback, LV_EVENT_CLICKED, NULL);

    // Network button with image
    lv_obj_t *network_btn = lv_btn_create(status_bar);
    lv_obj_set_size(network_btn, img_btn_size, img_btn_size);
    lv_obj_set_pos(network_btn, start_x + (img_btn_size + spacing) * 3, (STATUS_BAR_HEIGHT - img_btn_size) / 2);
    apply_circle_button_style(network_btn, COLOR_BUTTON_BACK);

    lv_obj_t *network_img = lv_img_create(network_btn);
    lv_img_set_src(network_img, IMG_NETWORK);
    lv_obj_center(network_img);
    lv_obj_add_event_cb(network_btn, network_btn_callback, LV_EVENT_CLICKED, NULL);

    return status_bar;
}

// ============================================================================
// STANDARD CONTENT AREA
// ============================================================================

lv_obj_t *create_standard_content(lv_obj_t *parent) {
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, SCREEN_WIDTH, SCREEN_HEIGHT - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, TITLE_BAR_HEIGHT);
    lv_obj_set_style_bg_color(content, lv_color_hex(COLOR_BG_DARK), 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);

    return content;
}

// ============================================================================
// BASE SCREEN CREATION
// ============================================================================

lv_obj_t *create_screen_base(int screen_id) {
    (void)screen_id;  // screen_id may be used for customization

    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_size(screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_BG_DARK), 0);

    // Disable scrolling
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    return screen;
}

// ============================================================================
// SCREEN FINALIZATION
// ============================================================================

void finalize_screen(lv_obj_t *screen, int screen_id) {
    // Add to screen stack
    if (screen_stack_top + 1 < MAX_SCREENS) {
        screen_stack_top++;
        screen_stack[screen_stack_top].screen = screen;
        screen_stack[screen_stack_top].screen_id = screen_id;
    }

    // Load the screen
    lv_scr_load(screen);
}
