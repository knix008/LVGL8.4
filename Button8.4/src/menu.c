#include "../include/menu.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/info.h"
#include "../include/admin.h"
#include "../include/network.h"

// ============================================================================
// EVENT CALLBACKS
// ============================================================================

static void back_btn_callback(lv_event_t *e) {
    (void)e;
    if (screen_stack_top > 0) {
        screen_stack_top--;
        show_screen(screen_stack[screen_stack_top].screen_id);
    }
}

static void info_btn_callback(lv_event_t *e) {
    (void)e;
    if (screen_stack[screen_stack_top].screen_id != SCREEN_INFO) {
        // Navigate using absolute path: clear stack to MENU then go to INFO
        screen_stack_top = 0;  // Reset to MAIN
        show_screen(SCREEN_MENU);  // Go through MENU
        show_screen(SCREEN_INFO);  // Then to INFO
    }
}

static void admin_btn_callback(lv_event_t *e) {
    (void)e;
    if (screen_stack[screen_stack_top].screen_id != SCREEN_ADMIN) {
        // Navigate using absolute path: clear stack to MENU then go to ADMIN
        screen_stack_top = 0;  // Reset to MAIN
        show_screen(SCREEN_MENU);  // Go through MENU
        show_screen(SCREEN_ADMIN);  // Then to ADMIN
    }
}

static void network_btn_callback(lv_event_t *e) {
    (void)e;
    if (screen_stack[screen_stack_top].screen_id != SCREEN_NETWORK) {
        // Navigate using absolute path: clear stack to MENU then go to NETWORK
        screen_stack_top = 0;  // Reset to MAIN
        show_screen(SCREEN_MENU);  // Go through MENU
        show_screen(SCREEN_NETWORK);  // Then to NETWORK
    }
}

// ============================================================================
// MENU SCREEN COMPONENTS
// ============================================================================

static lv_obj_t *create_menu_title_bar(lv_obj_t *parent) {
    lv_obj_t *title_bar = lv_obj_create(parent);
    lv_obj_set_size(title_bar, SCREEN_WIDTH, TITLE_BAR_HEIGHT);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 0);
    apply_bar_style(title_bar, COLOR_BG_TITLE);

    // Back button (circular)
    lv_obj_t *back_btn = lv_btn_create(title_bar);
    lv_obj_set_size(back_btn, TITLE_BAR_HEIGHT - 20, TITLE_BAR_HEIGHT - 20);  // Square button for circle
    lv_obj_align(back_btn, LV_ALIGN_LEFT_MID, PADDING_HORIZONTAL, 0);
    apply_circle_button_style(back_btn, COLOR_BUTTON_BACK);

    lv_obj_t *back_img = lv_img_create(back_btn);
    lv_img_set_src(back_img, IMG_BACK_BUTTON);
    lv_obj_align(back_img, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(back_btn, back_btn_callback, LV_EVENT_CLICKED, NULL);

    // Title label (positioned to the right of the back button)
    lv_obj_t *title_label = lv_label_create(title_bar);
    lv_label_set_text(title_label, "");  // Will be updated by update_title_bar_location
    apply_label_style(title_label);
    lv_obj_align(title_label, LV_ALIGN_LEFT_MID, (TITLE_BAR_HEIGHT - 20) + PADDING_HORIZONTAL * 2, 0);

    // Store the title label so it can be updated
    extern AppState app_state;
    app_state.current_title_label = title_label;

    // Update the title with breadcrumb path
    extern void update_title_bar_location(int screen_id);
    update_title_bar_location(SCREEN_MENU);

    return title_bar;
}

static lv_obj_t *create_menu_content(lv_obj_t *parent) {
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, SCREEN_WIDTH, SCREEN_HEIGHT - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, TITLE_BAR_HEIGHT);
    lv_obj_set_style_bg_color(content, lv_color_hex(COLOR_BG_DARK), 0);
    lv_obj_set_style_border_width(content, 0, 0);

    // Allow only vertical scrolling
    lv_obj_set_scroll_dir(content, LV_DIR_VER);

    // Create menu buttons with images and labels
    const char *menu_labels[] = {"관리자 설정", "네트워크 설정", "메뉴 3", "Info"};
    const char *menu_images[] = {IMG_CONFIG, IMG_NETWORK, IMG_SETUP, IMG_INFO};

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

        // Add event handlers
        if (i == 0) {
            lv_obj_add_event_cb(btn, admin_btn_callback, LV_EVENT_CLICKED, NULL);
        } else if (i == 1) {
            lv_obj_add_event_cb(btn, network_btn_callback, LV_EVENT_CLICKED, NULL);
        } else if (i == 3) {
            lv_obj_add_event_cb(btn, info_btn_callback, LV_EVENT_CLICKED, NULL);
        }
    }

    return content;
}

static lv_obj_t *create_menu_status_bar(lv_obj_t *parent) {
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

    // Setup button with image
    lv_obj_t *setup_btn = lv_btn_create(status_bar);
    lv_obj_set_size(setup_btn, img_btn_size, img_btn_size);
    lv_obj_set_pos(setup_btn, start_x + img_btn_size + spacing, (STATUS_BAR_HEIGHT - img_btn_size) / 2);
    apply_circle_button_style(setup_btn, COLOR_BUTTON_BACK);

    lv_obj_t *setup_img = lv_img_create(setup_btn);
    lv_img_set_src(setup_img, IMG_SETUP);
    lv_obj_center(setup_img);
    lv_obj_add_event_cb(setup_btn, admin_btn_callback, LV_EVENT_CLICKED, NULL);

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
// MENU SCREEN CREATION
// ============================================================================

void create_menu_screen(void) {
    lv_obj_t *menu_screen = lv_obj_create(NULL);
    lv_obj_set_size(menu_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(menu_screen, lv_color_hex(COLOR_BG_DARK), 0);

    // Disable scrolling on menu screen
    lv_obj_set_scrollbar_mode(menu_screen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(menu_screen, LV_OBJ_FLAG_SCROLLABLE);

    // Add to screen stack BEFORE creating title bar so breadcrumb can be built correctly
    if (screen_stack_top + 1 < MAX_SCREENS) {
        screen_stack_top++;
        screen_stack[screen_stack_top].screen = menu_screen;
        screen_stack[screen_stack_top].screen_id = SCREEN_MENU;
    }

    create_menu_title_bar(menu_screen);
    create_menu_content(menu_screen);
    create_menu_status_bar(menu_screen);

    lv_scr_load(menu_screen);
}
