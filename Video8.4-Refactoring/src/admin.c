#include "../include/admin.h"
#include "../include/admin_colors.h"
#include "../include/admin_calendar.h"
#include "../include/admin_language.h"
#include "../include/admin_fonts.h"
#include "../include/config.h"
#include "../include/state.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"
#include "../include/label.h"
#include <stdio.h>

// ============================================================================
// ADMIN SCREEN CONTENT CREATION
// ============================================================================

/**
 * Creates the admin screen content area with all settings sections
 */
static lv_obj_t *create_admin_content(lv_obj_t *parent) {
    if (!parent) return NULL;

    lv_obj_t *content = create_standard_content(parent);

    // Enable vertical scrolling with wider scrollbar
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);
    lv_obj_set_style_pad_right(content, 15, LV_PART_SCROLLBAR);
    lv_obj_set_style_width(content, 8, LV_PART_SCROLLBAR);

    // Main title - left aligned at 5px
    lv_obj_t *title_label = lv_label_create(content);
    lv_label_set_text(title_label, get_label("admin_screen.title"));
    apply_label_style(title_label);
    lv_obj_set_pos(title_label, 5, CONTENT_PADDING);

    // Calendar Settings Section (First Row)
    create_calendar_section(content, 40);

    // Create five color sections (moved down to make room for calendar)
    create_color_section(content, get_label("admin_screen.background_color"), 140, COLOR_TARGET_BACKGROUND);
    create_color_section(content, get_label("admin_screen.title_bar_color"), 220, COLOR_TARGET_TITLE_BAR);
    create_color_section(content, get_label("admin_screen.status_bar_color"), 300, COLOR_TARGET_STATUS_BAR);
    create_color_section(content, get_label("admin_screen.button_color"), 380, COLOR_TARGET_BUTTON);
    create_color_section(content, get_label("admin_screen.button_border_color"), 460, COLOR_TARGET_BUTTON_BORDER);

    // Language Settings Section
    create_language_section(content, 560);

    // Info text at bottom
    lv_obj_t *info_label = lv_label_create(content);
    lv_label_set_long_mode(info_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(info_label, SCREEN_WIDTH - CONTENT_WIDTH_PADDING);
    lv_label_set_text(info_label, get_label("admin_screen.info_text"));
    lv_obj_set_style_text_color(info_label, lv_color_hex(0xAAAAAA), 0);
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(info_label, app_state_get_font_20(), 0);
    }
    lv_obj_set_pos(info_label, CONTENT_PADDING, 640);

    return content;
}

// ============================================================================
// ADMIN SCREEN CREATION
// ============================================================================

/**
 * Creates the admin settings screen with title bar, content area, and status bar.
 * Uses the standard screen creation pattern.
 */
void create_admin_screen(void) {
    lv_obj_t *admin_screen = create_screen_base(SCREEN_ADMIN);
    if (!admin_screen) return;

    create_standard_title_bar(admin_screen, SCREEN_ADMIN);
    create_admin_content(admin_screen);
    create_standard_status_bar(admin_screen);

    finalize_screen(admin_screen, SCREEN_ADMIN);
}
