#include "../include/home.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/label.h"
#include "../include/slideshow.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ============================================================================
// EVENT CALLBACKS
// ============================================================================

static void menu_btn_callback(lv_event_t *e) {
    (void)e;
    show_screen(SCREEN_MENU);
}

static void exit_btn_callback(lv_event_t *e) {
    (void)e;
    exit(0);
}


// ============================================================================
// TITLE BAR
// ============================================================================

static void update_title_bar(void) {
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);

    const char *days[] = {
        get_label("days_of_week.sunday"),
        get_label("days_of_week.monday"),
        get_label("days_of_week.tuesday"),
        get_label("days_of_week.wednesday"),
        get_label("days_of_week.thursday"),
        get_label("days_of_week.friday"),
        get_label("days_of_week.saturday")
    };

    char title_text[MAX_TITLE_LENGTH];
    snprintf(title_text, sizeof(title_text),
        "%s %02d:%02d:%02d\n%04d-%02d-%02d",
        days[timeinfo->tm_wday],
        timeinfo->tm_hour,
        timeinfo->tm_min,
        timeinfo->tm_sec,
        timeinfo->tm_year + 1900,
        timeinfo->tm_mon + 1,
        timeinfo->tm_mday
    );

    if (app_state.title_label) {
        lv_label_set_text(app_state.title_label, title_text);
    }
}

static void create_main_title_bar(void) {
    app_state.title_bar = lv_obj_create(app_state.screen);
    lv_obj_set_size(app_state.title_bar, SCREEN_WIDTH, TITLE_BAR_HEIGHT);
    lv_obj_align(app_state.title_bar, LV_ALIGN_TOP_MID, 0, 0);
    apply_bar_style(app_state.title_bar, get_title_bar_color());
    
    // Set user data to identify as title bar
    lv_obj_set_user_data(app_state.title_bar, (void*)1);  // ID: 1 = title bar

    // Disable scrolling on title bar - must stay fixed
    lv_obj_set_scrollbar_mode(app_state.title_bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(app_state.title_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_scroll_to(app_state.title_bar, 0, 0, LV_ANIM_OFF);

    app_state.title_label = lv_label_create(app_state.title_bar);
    lv_obj_set_style_text_color(app_state.title_label, lv_color_hex(COLOR_TEXT), 0);
    lv_label_set_long_mode(app_state.title_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(app_state.title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(app_state.title_label, TITLE_LABEL_WIDTH);
    lv_obj_align(app_state.title_label, LV_ALIGN_CENTER, 0, 0);
    
    // Set Korean-supporting font
    if (app_state.font_20) {
        lv_obj_set_style_text_font(app_state.title_label, app_state.font_20, 0);
    }

    update_title_bar();
    lv_timer_create((lv_timer_cb_t)update_title_bar, UPDATE_INTERVAL_TIMER, NULL);
}

// ============================================================================
// STATUS BAR
// ============================================================================

static void create_main_status_bar(void) {
    lv_obj_t *status_bar = lv_obj_create(app_state.screen);
    lv_obj_set_size(status_bar, SCREEN_WIDTH, STATUS_BAR_HEIGHT);
    lv_obj_align(status_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    apply_bar_style(status_bar, get_status_bar_color());
    
    // Set user data to identify as status bar
    lv_obj_set_user_data(status_bar, (void*)2);  // ID: 2 = status bar

    // Disable scrolling on status bar - buttons must stay fixed
    lv_obj_set_scrollbar_mode(status_bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_scroll_to(status_bar, 0, 0, LV_ANIM_OFF);

    // Menu button
    lv_obj_t *menu_btn = lv_btn_create(status_bar);
    lv_obj_set_size(menu_btn, BUTTON_WIDTH, BUTTON_HEIGHT);
    lv_obj_align(menu_btn, LV_ALIGN_LEFT_MID, PADDING_HORIZONTAL, 0);
    apply_button_style(menu_btn, 0);
    lv_obj_set_scrollbar_mode(menu_btn, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(menu_btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_scroll_to(menu_btn, 0, 0, LV_ANIM_OFF);

    lv_obj_t *menu_label = lv_label_create(menu_btn);
    lv_label_set_text(menu_label, get_label("home_screen.menu_button"));
    apply_label_style(menu_label);
    lv_obj_align(menu_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(menu_btn, menu_btn_callback, LV_EVENT_CLICKED, NULL);

    // Exit button
    lv_obj_t *exit_btn = lv_btn_create(status_bar);
    lv_obj_set_size(exit_btn, BUTTON_WIDTH, BUTTON_HEIGHT);
    lv_obj_align(exit_btn, LV_ALIGN_RIGHT_MID, -PADDING_HORIZONTAL, 0);
    apply_button_style(exit_btn, 0);
    lv_obj_set_scrollbar_mode(exit_btn, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(exit_btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_scroll_to(exit_btn, 0, 0, LV_ANIM_OFF);

    lv_obj_t *exit_label = lv_label_create(exit_btn);
    lv_label_set_text(exit_label, get_label("home_screen.exit_button"));
    apply_label_style(exit_label);
    lv_obj_align(exit_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(exit_btn, exit_btn_callback, LV_EVENT_CLICKED, NULL);
}

// ============================================================================
// HOME SCREEN CREATION
// ============================================================================

void create_gui(void) {
    app_state.screen = lv_scr_act();

    // Add main screen to screen stack
    if (screen_stack_top < 0) {
        screen_stack_top = 0;
        screen_stack[0].screen = app_state.screen;
        screen_stack[0].screen_id = SCREEN_MAIN;
    }

    // Disable all scrolling on main screen - buttons must stay fixed
    lv_obj_set_scrollbar_mode(app_state.screen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(app_state.screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_scroll_to(app_state.screen, 0, 0, LV_ANIM_OFF);

    // Create title bar and status bar first (they will be on top)
    create_main_title_bar();
    create_main_status_bar();

    // Initialize slideshow
    if (slideshow_init(app_state.screen) != 0) {
        printf("Warning: Slideshow initialization failed\n");
    }
}
