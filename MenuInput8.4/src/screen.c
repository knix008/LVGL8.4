#include "../include/screen.h"
#include "../include/config.h"
#include "../include/style.h"
#include "../include/menu.h"
#include "../include/info.h"
#include "../include/admin.h"
#include "../include/network.h"
#include "../include/korean_input.h"
#include <string.h>

// ============================================================================
// SCREEN MANAGEMENT
// ============================================================================

void update_title_bar_location(int screen_id) {
    (void)screen_id;  // Current screen_id is already in the screen_stack
    static char breadcrumb[256];

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

void show_screen(int screen_id) {
    for (int i = 0; i <= screen_stack_top; i++) {
        if (screen_stack[i].screen_id == screen_id) {
            screen_stack_top = i;
            lv_scr_load(screen_stack[i].screen);
            update_title_bar_location(screen_id);
            return;
        }
    }

    if (screen_id == SCREEN_MENU && (screen_stack_top < 0 || screen_stack[screen_stack_top].screen_id != SCREEN_MENU)) {
        create_menu_screen();
        update_title_bar_location(screen_id);
    }

    if (screen_id == SCREEN_INFO && (screen_stack_top < 0 || screen_stack[screen_stack_top].screen_id != SCREEN_INFO)) {
        create_info_screen();
        update_title_bar_location(screen_id);
    }

    if (screen_id == SCREEN_ADMIN && (screen_stack_top < 0 || screen_stack[screen_stack_top].screen_id != SCREEN_ADMIN)) {
        create_admin_screen();
        update_title_bar_location(screen_id);
    }

    if (screen_id == SCREEN_NETWORK && (screen_stack_top < 0 || screen_stack[screen_stack_top].screen_id != SCREEN_NETWORK)) {
        create_network_screen();
        update_title_bar_location(screen_id);
    }

    if (screen_id == SCREEN_KOREAN_INPUT && (screen_stack_top < 0 || screen_stack[screen_stack_top].screen_id != SCREEN_KOREAN_INPUT)) {
        create_korean_input_screen();
        update_title_bar_location(screen_id);
    }
}
