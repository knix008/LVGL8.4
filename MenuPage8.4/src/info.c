#include "../include/info.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"
#include "../include/label.h"
#include <stdio.h>

// ============================================================================
// INFO SCREEN COMPONENTS
// ============================================================================

static lv_obj_t *create_info_content(lv_obj_t *parent) {
    lv_obj_t *content = create_standard_content(parent);

    // Create info text
    lv_obj_t *info_label = lv_label_create(content);
    lv_label_set_long_mode(info_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(info_label, SCREEN_WIDTH - CONTENT_WIDTH_PADDING);
    apply_label_style(info_label);
    lv_obj_set_style_pad_all(info_label, CONTENT_PADDING, 0);
    lv_obj_align(info_label, LV_ALIGN_TOP_LEFT, CONTENT_PADDING, CONTENT_PADDING);

    char info_text[512];
    snprintf(info_text, sizeof(info_text),
        "%s\n\n"
        "%s\n\n"
        "%s\n\n"
        "%s\n"
        "%s\n\n"
        "%s",
        get_label("info_screen.title"),
        get_label("info_screen.app_name"),
        get_label("info_screen.version"),
        get_label("info_screen.description_label"),
        get_label("info_screen.description_text"),
        get_label("info_screen.copyright")
    );
    lv_label_set_text(info_label, info_text);

    return content;
}

// ============================================================================
// INFO SCREEN CREATION
// ============================================================================

/**
 * Creates the information screen with title bar, content area, and status bar.
 * Uses the standard screen creation pattern.
 */
void create_info_screen(void) {
    lv_obj_t *info_screen = create_screen_base(SCREEN_INFO);

    create_standard_title_bar(info_screen, SCREEN_INFO);
    create_info_content(info_screen);
    create_standard_status_bar(info_screen);

    finalize_screen(info_screen, SCREEN_INFO);
}
