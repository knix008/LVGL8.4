#include "../include/home.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/label.h"
#include "../include/slideshow.h"
#include "../include/video.h"
#include "../include/welcome.h"
#include "../include/state/app_state.h"
#include "../include/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ============================================================================
// INACTIVITY DETECTION
// ============================================================================

static lv_timer_t *inactivity_timer = NULL;
static uint32_t last_activity_time = 0;

static void reset_inactivity_timer(void);

static void inactivity_timer_callback(lv_timer_t *timer) {
    (void)timer;

    uint32_t current_time = lv_tick_get();
    uint32_t elapsed = current_time - last_activity_time;

    if (elapsed >= INACTIVITY_TIMEOUT) {
        // Check if video is already playing to avoid repeated starts
        if (!video_is_playing()) {
            // Inactivity timeout reached - hide slideshow and show video
            lv_obj_t *slideshow_img = slideshow_get_image();
            if (slideshow_img) {
                lv_obj_add_flag(slideshow_img, LV_OBJ_FLAG_HIDDEN);
            }

            // Hide welcome message
            if (app_state_get_welcome_label()) {
                lv_obj_add_flag(app_state_get_welcome_label(), LV_OBJ_FLAG_HIDDEN);
            }

            // Show and start video
            video_show();
            video_start();
        }
    }
}

static void reset_inactivity_timer(void) {
    last_activity_time = lv_tick_get();

    // If video is playing, stop it and show slideshow again
    if (video_is_playing()) {
        video_stop();
        video_hide();

        // Show slideshow again
        lv_obj_t *slideshow_img = slideshow_get_image();
        if (slideshow_img) {
            lv_obj_clear_flag(slideshow_img, LV_OBJ_FLAG_HIDDEN);
        }

        // Show welcome message again
        if (app_state_get_welcome_label()) {
            lv_obj_clear_flag(app_state_get_welcome_label(), LV_OBJ_FLAG_HIDDEN);
        }

        printf("Activity detected - stopping video playback\n");
    }
}

static void activity_event_callback(lv_event_t *e) {
    (void)e;
    reset_inactivity_timer();
}

// ============================================================================
// INACTIVITY TIMER CONTROL (PUBLIC)
// ============================================================================

void start_inactivity_timer(void) {
    if (!inactivity_timer) {
        last_activity_time = lv_tick_get();
        inactivity_timer = lv_timer_create(inactivity_timer_callback, 1000, NULL);
    }
}

void stop_inactivity_timer(void) {
    if (inactivity_timer) {
        lv_timer_del(inactivity_timer);
        inactivity_timer = NULL;
        
        // Stop video if playing
        if (video_is_playing()) {
            video_stop();
            video_hide();
        }
    }
}

void pause_inactivity_timer(void) {
    if (inactivity_timer) {
        lv_timer_pause(inactivity_timer);
    }
}

void resume_inactivity_timer(void) {
    if (inactivity_timer) {
        last_activity_time = lv_tick_get();
        lv_timer_resume(inactivity_timer);
    }
}

// ============================================================================
// EVENT CALLBACKS
// ============================================================================

static void menu_btn_callback(lv_event_t *e) {
    (void)e;
    reset_inactivity_timer();
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

    if (app_state_get_title_label()) {
        lv_label_set_text(app_state_get_title_label(), title_text);
    }
}

static void create_main_title_bar(void) {
    lv_obj_t *title_bar = lv_obj_create(app_state_get_screen());
    lv_obj_set_size(title_bar, SCREEN_WIDTH, TITLE_BAR_HEIGHT);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 0);
    apply_bar_style(title_bar, get_title_bar_color());
    app_state_set_title_bar(title_bar);
    
    // Set user data to identify as title bar
    lv_obj_set_user_data(app_state_get_title_bar(), (void*)1);  // ID: 1 = title bar

    // Disable scrolling on title bar - must stay fixed
    lv_obj_set_scrollbar_mode(app_state_get_title_bar(), LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(app_state_get_title_bar(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_scroll_to(app_state_get_title_bar(), 0, 0, LV_ANIM_OFF);

    lv_obj_t *title_label = lv_label_create(app_state_get_title_bar());
    lv_obj_set_style_text_color(title_label, lv_color_hex(COLOR_TEXT), 0);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title_label, TITLE_LABEL_WIDTH);
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);
    app_state_set_title_label(title_label);
    
    // Set Korean-supporting font
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(app_state_get_title_label(), app_state_get_font_20(), 0);
    }

    update_title_bar();
    lv_timer_create((lv_timer_cb_t)update_title_bar, UPDATE_INTERVAL_TIMER, NULL);
}

// ============================================================================
// STATUS BAR
// ============================================================================

static void create_main_status_bar(void) {
    lv_obj_t *status_bar = lv_obj_create(app_state_get_screen());
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
    app_state_set_menu_button_label(menu_label);  // Store reference for language updates
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
    app_state_set_exit_button_label(exit_label);  // Store reference for language updates
    lv_obj_add_event_cb(exit_btn, exit_btn_callback, LV_EVENT_CLICKED, NULL);
}

// ============================================================================
// WELCOME MESSAGE
// ============================================================================

// Color palette for welcome message animation
static const uint32_t welcome_colors[] = {
    WELCOME_COLOR_WHITE,
    WELCOME_COLOR_PINK,
    WELCOME_COLOR_RED_PINK,
    WELCOME_COLOR_GOLD,
    WELCOME_COLOR_CYAN,
    WELCOME_COLOR_GREEN
};
#define WELCOME_COLOR_COUNT (sizeof(welcome_colors) / sizeof(welcome_colors[0]))

static int color_index = 0;

static void update_welcome_message(void) {
    if (app_state_get_welcome_label()) {
        lv_label_set_text(app_state_get_welcome_label(), welcome_get_message());
    }
}

static void welcome_message_timer_callback(lv_timer_t *timer) {
    (void)timer;
    update_welcome_message();
}

static void welcome_color_timer_callback(lv_timer_t *timer) {
    (void)timer;
    if (app_state_get_welcome_label()) {
        color_index = (color_index + 1) % WELCOME_COLOR_COUNT;
        lv_obj_set_style_text_color(app_state_get_welcome_label(),
                                     lv_color_hex(welcome_colors[color_index]), 0);
    }
}

// ============================================================================
// LANGUAGE UPDATE
// ============================================================================

void update_home_screen_labels(void) {
    if (app_state_get_menu_button_label()) {
        lv_label_set_text(app_state_get_menu_button_label(), get_label("home_screen.menu_button"));
    }
    if (app_state_get_exit_button_label()) {
        lv_label_set_text(app_state_get_exit_button_label(), get_label("home_screen.exit_button"));
    }
    // Reload welcome messages when language changes
    welcome_load();
    update_welcome_message();
}

// ============================================================================
// HOME SCREEN CREATION
// ============================================================================

void create_gui(void) {
    lv_obj_t *screen = lv_scr_act();
    app_state_set_screen(screen);

    // Add main screen to screen stack
    if (screen_stack_top < 0) {
        screen_stack_top = 0;
        screen_stack[0].screen = screen;
        screen_stack[0].screen_id = SCREEN_MAIN;
    }

    // Disable all scrolling on main screen - buttons must stay fixed
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_scroll_to(screen, 0, 0, LV_ANIM_OFF);

    // Create title bar and status bar first (they will be on top)
    create_main_title_bar();
    create_main_status_bar();

    // Create welcome message container in the upper 1/3
    // Available space: TITLE_BAR_HEIGHT (60px) to (SCREEN_HEIGHT - STATUS_BAR_HEIGHT = 580px)
    // Upper 1/3 position: 60 + (580 - 60) / 3 = 233px
    lv_obj_t *welcome_container = lv_obj_create(app_state_get_screen());
    lv_obj_set_size(welcome_container, SCREEN_WIDTH, WELCOME_MESSAGE_CONTAINER_HEIGHT);
    lv_obj_set_pos(welcome_container, 0, WELCOME_MESSAGE_Y_POSITION);
    lv_obj_set_style_bg_color(welcome_container, lv_color_hex(get_background_color()), 0);
    lv_obj_set_style_bg_opa(welcome_container, LV_OPA_TRANSP, 0);  // Transparent background
    lv_obj_set_style_border_width(welcome_container, 0, 0);
    lv_obj_set_scrollbar_mode(welcome_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(welcome_container, LV_OBJ_FLAG_SCROLLABLE);

    // Create welcome message label
    lv_obj_t *welcome_label = lv_label_create(welcome_container);
    lv_label_set_long_mode(welcome_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(welcome_label, SCREEN_WIDTH - 20);

    // Style: 30pt bold text, white color, centered, transparent background
    lv_obj_set_style_text_color(welcome_label, lv_color_hex(COLOR_TEXT), 0);
    lv_obj_set_style_text_align(welcome_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_color(welcome_label, lv_color_hex(get_background_color()), 0);
    lv_obj_set_style_bg_opa(welcome_label, LV_OPA_TRANSP, 0);  // Transparent background
    app_state_set_welcome_label(welcome_label);

    // Apply bold 30pt Korean font
    if (app_state_get_font_24_bold()) {
        lv_obj_set_style_text_font(app_state_get_welcome_label(), app_state_get_font_24_bold(), 0);
    }

    // Vertically center the label within its container
    lv_obj_align(app_state_get_welcome_label(), LV_ALIGN_CENTER, 0, 0);

    // Load and display welcome message
    if (welcome_load() == 0) {
        update_welcome_message();
        // Create timer to update welcome message based on time period
        lv_timer_create(welcome_message_timer_callback, WELCOME_MESSAGE_UPDATE_INTERVAL, NULL);
        // Create timer to change welcome message color periodically
        lv_timer_create(welcome_color_timer_callback, WELCOME_COLOR_UPDATE_INTERVAL, NULL);
    } else {
        log_warning("Failed to load welcome messages");
    }

    // Initialize slideshow
    if (slideshow_init(app_state_get_screen()) != 0) {
        log_warning("Slideshow initialization failed");
    }

    // Initialize video player
    if (video_init(app_state_get_screen()) != 0) {
        log_warning("Video player initialization failed");
    }

    // Add event handler to detect user activity (touch/click events) only on home screen
    lv_obj_add_event_cb(app_state_get_screen(), activity_event_callback, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(app_state_get_screen(), activity_event_callback, LV_EVENT_CLICKED, NULL);
    
    // Start inactivity timer for home screen (called directly from main, not via navigation)
    start_inactivity_timer();
}
