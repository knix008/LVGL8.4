#include "../include/screen.h"
#include "../include/config.h"
#include "../include/style.h"
#include "../include/menu.h"
#include "../include/info.h"
#include "../include/admin.h"
#include "../include/network.h"
#include "../include/korean.h"
#include "../include/face.h"
#include "../include/navigation.h"
#include <string.h>
#include <stdio.h>

// External reference to the global app state
extern AppState app_state;

// ============================================================================
// SCREEN MANAGEMENT
// ============================================================================

/**
 * Updates the breadcrumb navigation title bar to show the current screen location.
 * 
 * @param screen_id The screen ID to update the title bar for
 */
void update_title_bar_location(int screen_id) {
    (void)screen_id;  // Current screen_id is already in the screen_stack
    static char breadcrumb[MAX_BREADCRUMB_LENGTH];

    // Build the breadcrumb path from the screen stack
    breadcrumb[0] = '\0';
    for (int i = 0; i <= screen_stack_top && i < MAX_SCREENS; i++) {
        const char *name = "홈";

        switch (screen_stack[i].screen_id) {
            case SCREEN_MAIN:
                name = "홈";
                break;
            case SCREEN_MENU:
                name = "메뉴";
                break;
            case SCREEN_INFO:
                name = "정보";
                break;
            case SCREEN_ADMIN:
                name = "관리자 설정";
                break;
            case SCREEN_NETWORK:
                name = "네트워크 설정";
                break;
            case SCREEN_KOREAN_INPUT:
                name = "한글 입력";
                break;
            case SCREEN_FACE:
                name = "Face";
                break;
            default:
                name = "홈";
                break;
        }

        if (i > 0) {
            strncat(breadcrumb, " > ", sizeof(breadcrumb) - strlen(breadcrumb) - 1);
        }
        strncat(breadcrumb, name, sizeof(breadcrumb) - strlen(breadcrumb) - 1);
    }

    if (app_state.current_title_label) {
        lv_label_set_text(app_state.current_title_label, breadcrumb);
    } else if (app_state.title_label) {
        lv_label_set_text(app_state.title_label, breadcrumb);
    }
}

/**
 * Shows a screen by screen ID, creating it if it doesn't exist or reusing it from the stack.
 * Manages the screen stack and handles screen transitions with proper cleanup.
 * 
 * @param screen_id The screen ID to display (SCREEN_MENU, SCREEN_INFO, etc.)
 */
void show_screen(int screen_id) {
    // First check if screen already exists anywhere in the stack
    for (int i = 0; i < MAX_SCREENS; i++) {
        if (screen_stack[i].screen && screen_stack[i].screen_id == screen_id) {
            screen_stack_top = i;
            lv_scr_load(screen_stack[i].screen);

            // Move the status bar to this existing screen
            move_status_bar_to_screen(screen_stack[i].screen, screen_id);

            update_title_bar_location(screen_id);
            return;
        }
    }

    // Screen doesn't exist, create it
    if (screen_id == SCREEN_MENU) {
        create_menu_screen();
        update_title_bar_location(screen_id);
    }

    if (screen_id == SCREEN_INFO) {
        create_info_screen();
        update_title_bar_location(screen_id);
    }

    if (screen_id == SCREEN_ADMIN) {
        create_admin_screen();
        update_title_bar_location(screen_id);
    }

    if (screen_id == SCREEN_NETWORK) {
        create_network_screen();
        update_title_bar_location(screen_id);
    }

    if (screen_id == SCREEN_KOREAN_INPUT) {
        create_korean_input_screen();
        update_title_bar_location(screen_id);
    }

    if (screen_id == SCREEN_FACE) {
        create_face_screen();
        update_title_bar_location(screen_id);
    }
}

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
    lv_obj_set_size(back_btn, TITLE_BAR_HEIGHT - BACK_BUTTON_PADDING, TITLE_BAR_HEIGHT - BACK_BUTTON_PADDING);
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
    lv_obj_align(title_label, LV_ALIGN_LEFT_MID, (TITLE_BAR_HEIGHT - BACK_BUTTON_PADDING) + PADDING_HORIZONTAL * 2, 0);

    // Store the title label
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

/**
 * Creates a base screen object with standard size and styling.
 * 
 * @param screen_id The screen ID (currently unused, reserved for future customization)
 * @return A configured LVGL screen object
 */
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

/**
 * Finalizes screen creation by adding it to the screen stack and loading it.
 * 
 * @param screen The LVGL screen object to finalize
 * @param screen_id The screen ID for stack management
 */
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

/**
 * Adds an icon to the status bar at the specified menu index position.
 * 
 * @param menu_index The position index (0-4) for the icon
 * @param icon_path The file path to the icon image
 */
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

/**
 * Removes an icon from the status bar at the specified menu index position.
 * 
 * @param menu_index The position index (0-4) of the icon to remove
 */
void remove_status_bar_icon(int menu_index) {
    if (menu_index < 0 || menu_index >= MAX_STATUS_ICONS) {
        return;
    }

    // Mark as not selected
    app_state.menu_item_selected[menu_index] = false;

    // Update the status bar icons
    update_status_bar_icons();
}

/**
 * Updates all status bar icons based on current configuration.
 * Loads enabled icons from configuration and positions them correctly.
 */
void update_status_bar_icons(void) {
    if (!app_state.status_bar) {
        return;
    }

    // First, clean all children from the status bar
    lv_obj_clean(app_state.status_bar);

    // Reset all icon references
    for (int i = 0; i < MAX_STATUS_ICONS; i++) {
        app_state.status_icons[i] = NULL;
    }

    // Icon configuration
    int img_btn_size = ICON_SIZE_SMALL;
    int spacing = STATUS_ICON_SPACING;
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
            lv_img_set_src(icon_img, MENU_ITEMS[i].icon_path);
            lv_obj_center(icon_img);

            // Add navigation callback from menu configuration
            if (MENU_ITEMS[i].callback) {
                lv_obj_add_event_cb(icon_btn, MENU_ITEMS[i].callback, LV_EVENT_CLICKED, NULL);
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
