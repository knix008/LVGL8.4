#include "../include/info.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"

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

    lv_label_set_text(info_label,
        "애플리케이션 정보\n\n"
        "이름: LVGL Menu\n\n"
        "버전: 8.4\n\n"
        "설명:\n"
        "LVGL 기반의 메뉴 시스템입니다.\n"
        "한글 입력을 지원합니다.\n\n"
        "저작권: 2024"
    );

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
