#include "../include/screen_components.h"
#include "../include/navigation.h"
#include "../include/config.h"
#include "../include/style.h"
#include "../include/screen.h"
#include <stdio.h>

// External reference to the global app state
extern AppState app_state;

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
    // If status bar doesn't exist, create it
    if (!app_state.status_bar) {
        app_state.status_bar = lv_obj_create(parent);
        lv_obj_set_size(app_state.status_bar, SCREEN_WIDTH, STATUS_BAR_HEIGHT);
        lv_obj_align(app_state.status_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
        apply_bar_style(app_state.status_bar, COLOR_BG_TITLE);

        // Ensure status bar is always visible and on top
        lv_obj_clear_flag(app_state.status_bar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(app_state.status_bar);

        // Initialize status icons array
        for (int i = 0; i < MAX_STATUS_ICONS; i++) {
            app_state.status_icons[i] = NULL;
        }
    } else {
        // Move the existing status bar to the new parent screen
        lv_obj_set_parent(app_state.status_bar, parent);
        lv_obj_align(app_state.status_bar, LV_ALIGN_BOTTOM_MID, 0, 0);

        // Ensure it's visible and on top
        lv_obj_clear_flag(app_state.status_bar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(app_state.status_bar);
    }

    // Update status bar icons based on current selections
    update_status_bar_icons();

    return app_state.status_bar;
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

// ============================================================================
// STATUS BAR ICON MANAGEMENT
// ============================================================================

void add_status_bar_icon(int menu_index, const char *icon_path) {
    (void)icon_path;  // Icon path is determined from menu_index in update_status_bar_icons

    if (menu_index < 0 || menu_index >= MAX_STATUS_ICONS) {
        return;
    }

    if (!app_state.status_bar) {
        return;
    }

    // Mark as selected
    app_state.menu_item_selected[menu_index] = true;

    // Update the status bar icons
    update_status_bar_icons();
}

void remove_status_bar_icon(int menu_index) {
    if (menu_index < 0 || menu_index >= MAX_STATUS_ICONS) {
        return;
    }

    // Mark as not selected
    app_state.menu_item_selected[menu_index] = false;

    // Update the status bar icons
    update_status_bar_icons();
}

void update_status_bar_icons(void) {
    if (!app_state.status_bar) {
        return;
    }

    // Menu item to icon path mapping
    const char *menu_icons[] = {IMG_CONFIG, IMG_NETWORK, IMG_KOREAN, IMG_INFO};

    // First, clean all children from the status bar
    lv_obj_clean(app_state.status_bar);

    // Reset all icon references
    for (int i = 0; i < MAX_STATUS_ICONS; i++) {
        app_state.status_icons[i] = NULL;
    }

    // Icon configuration
    int img_btn_size = 40;
    int spacing = 10;
    int start_x = PADDING_HORIZONTAL;

    // Count how many icons are selected and create them
    int icon_position = 0;
    for (int i = 0; i < MAX_STATUS_ICONS; i++) {
        if (app_state.menu_item_selected[i]) {
            // Create button for this icon
            lv_obj_t *icon_btn = lv_btn_create(app_state.status_bar);
            lv_obj_set_size(icon_btn, img_btn_size, img_btn_size);
            lv_obj_set_pos(icon_btn, start_x + icon_position * (img_btn_size + spacing),
                          (STATUS_BAR_HEIGHT - img_btn_size) / 2);
            apply_circle_button_style(icon_btn, COLOR_BUTTON_BACK);

            // Create image inside button
            lv_obj_t *icon_img = lv_img_create(icon_btn);
            lv_img_set_src(icon_img, menu_icons[i]);
            lv_obj_center(icon_img);

            // Add navigation callback based on menu item
            if (i == 0) {
                lv_obj_add_event_cb(icon_btn, admin_btn_callback, LV_EVENT_CLICKED, NULL);
            } else if (i == 1) {
                lv_obj_add_event_cb(icon_btn, network_btn_callback, LV_EVENT_CLICKED, NULL);
            } else if (i == 2) {
                lv_obj_add_event_cb(icon_btn, korean_input_btn_callback, LV_EVENT_CLICKED, NULL);
            } else if (i == 3) {
                lv_obj_add_event_cb(icon_btn, info_btn_callback, LV_EVENT_CLICKED, NULL);
            }

            // Store reference to the icon button
            app_state.status_icons[i] = icon_btn;
            icon_position++;
        }
    }

    // Force a refresh of the status bar
    lv_obj_invalidate(app_state.status_bar);
}

// ============================================================================
// STATUS BAR RELOCATION FOR EXISTING SCREENS
// ============================================================================

void move_status_bar_to_screen(lv_obj_t *screen, int screen_id) {
    // Don't move status bar to home screen (it has its own status bar)
    if (screen_id == SCREEN_MAIN) {
        return;
    }

    // If status bar exists, move it to this screen
    if (app_state.status_bar && screen) {
        lv_obj_set_parent(app_state.status_bar, screen);
        lv_obj_align(app_state.status_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_clear_flag(app_state.status_bar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(app_state.status_bar);
    }
}
