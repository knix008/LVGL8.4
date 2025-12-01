#include "../include/admin.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"

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

// ============================================================================
// ADMIN SCREEN COMPONENTS
// ============================================================================

static lv_obj_t *create_admin_title_bar(lv_obj_t *parent) {
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
    update_title_bar_location(SCREEN_ADMIN);

    return title_bar;
}

static lv_obj_t *create_admin_content(lv_obj_t *parent) {
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, SCREEN_WIDTH, SCREEN_HEIGHT - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, TITLE_BAR_HEIGHT);
    lv_obj_set_style_bg_color(content, lv_color_hex(COLOR_BG_DARK), 0);
    lv_obj_set_style_border_width(content, 0, 0);

    // Allow only vertical scrolling
    lv_obj_set_scroll_dir(content, LV_DIR_VER);

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

static lv_obj_t *create_admin_status_bar(lv_obj_t *parent) {
    lv_obj_t *status_bar = lv_obj_create(parent);
    lv_obj_set_size(status_bar, SCREEN_WIDTH, STATUS_BAR_HEIGHT);
    lv_obj_align(status_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    apply_bar_style(status_bar, COLOR_BG_TITLE);

    return status_bar;
}

// ============================================================================
// ADMIN SCREEN CREATION
// ============================================================================

void create_admin_screen(void) {
    lv_obj_t *admin_screen = lv_obj_create(NULL);
    lv_obj_set_size(admin_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(admin_screen, lv_color_hex(COLOR_BG_DARK), 0);

    // Disable scrolling on admin screen
    lv_obj_set_scrollbar_mode(admin_screen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(admin_screen, LV_OBJ_FLAG_SCROLLABLE);

    // Add to screen stack BEFORE creating title bar so breadcrumb can be built correctly
    if (screen_stack_top + 1 < MAX_SCREENS) {
        screen_stack_top++;
        screen_stack[screen_stack_top].screen = admin_screen;
        screen_stack[screen_stack_top].screen_id = SCREEN_ADMIN;
    }

    create_admin_title_bar(admin_screen);
    create_admin_content(admin_screen);
    create_admin_status_bar(admin_screen);

    lv_scr_load(admin_screen);
}
