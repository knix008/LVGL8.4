#include "../include/admin.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/screen_components.h"
#include "../include/navigation.h"

// ============================================================================
// ADMIN SCREEN COMPONENTS
// ============================================================================

static lv_obj_t *create_admin_content(lv_obj_t *parent) {
    lv_obj_t *content = create_standard_content(parent);

    // Create admin settings text
    lv_obj_t *admin_label = lv_label_create(content);
    lv_label_set_long_mode(admin_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(admin_label, SCREEN_WIDTH - 20);
    apply_label_style(admin_label);
    lv_obj_set_style_pad_all(admin_label, 10, 0);
    lv_obj_align(admin_label, LV_ALIGN_TOP_LEFT, 10, 10);

    lv_label_set_text(admin_label,
        "관리자 설정\n\n"
        "시스템 관리\n"
        "- 사용자 관리\n"
        "- 권한 설정\n\n"
        "보안 설정\n"
        "- 암호 변경\n"
        "- 로그 확인\n\n"
        "시스템 유지보수\n"
        "- 백업\n"
        "- 복구"
    );

    return content;
}

// ============================================================================
// ADMIN SCREEN CREATION
// ============================================================================

void create_admin_screen(void) {
    lv_obj_t *admin_screen = create_screen_base(SCREEN_ADMIN);

    create_standard_title_bar(admin_screen, SCREEN_ADMIN);
    create_admin_content(admin_screen);
    create_standard_status_bar(admin_screen);

    finalize_screen(admin_screen, SCREEN_ADMIN);
}
