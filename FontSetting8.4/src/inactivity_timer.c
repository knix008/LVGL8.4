#include "../include/inactivity_timer.h"
#include "../include/types.h"
#include "../include/state.h"
#include "../include/video.h"
#include "../include/slideshow.h"
#include "../include/screen.h"
#include "../include/home.h"
#include <stdio.h>

// ============================================================================
// INACTIVITY TIMER STATE
// ============================================================================

// Timer handles for each context
static lv_timer_t *home_timer = NULL;
static lv_timer_t *non_home_timer = NULL;

// Last activity timestamps
static uint32_t home_last_activity_time = 0;
static uint32_t non_home_last_activity_time = 0;

// Last active screen before going to home (for non-home context)
static int last_active_screen_id = SCREEN_MAIN;

// ============================================================================
// HOME SCREEN INACTIVITY TIMER
// ============================================================================

static void home_inactivity_callback(lv_timer_t *timer) {
    (void)timer;

    uint32_t current_time = lv_tick_get();
    uint32_t elapsed = current_time - home_last_activity_time;

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

static void reset_home_timer(void) {
    home_last_activity_time = lv_tick_get();

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

// ============================================================================
// NON-HOME SCREEN INACTIVITY TIMER
// ============================================================================

static void non_home_inactivity_callback(lv_timer_t *timer) {
    (void)timer;

    // Only check if we're not on home screen
    extern ScreenState screen_stack[];
    extern int screen_stack_top;

    if (screen_stack_top >= 0 && screen_stack[screen_stack_top].screen_id != SCREEN_MAIN) {
        uint32_t current_time = lv_tick_get();
        uint32_t elapsed = current_time - non_home_last_activity_time;

        if (elapsed >= INACTIVITY_TIMEOUT) {
            // Inactivity detected - load home screen without modifying stack
            // This preserves the navigation path for the back button

            // Find home screen in stack (should be at position 0)
            for (int i = 0; i <= screen_stack_top; i++) {
                if (screen_stack[i].screen_id == SCREEN_MAIN && screen_stack[i].screen) {
                    // Load home screen without changing stack position
                    lv_scr_load(screen_stack[i].screen);

                    // Stop non-home timer and start home timer
                    inactivity_timer_stop(INACTIVITY_CONTEXT_NON_HOME);
                    start_inactivity_timer();

                    // Resume slideshow and video
                    slideshow_resume();
                    if (video_is_playing()) {
                        video_resume();
                    }
                    break;
                }
            }
        }
    }
}

static void reset_non_home_timer(void) {
    non_home_last_activity_time = lv_tick_get();
}

// ============================================================================
// PUBLIC API IMPLEMENTATION
// ============================================================================

void inactivity_timer_init(void) {
    home_timer = NULL;
    non_home_timer = NULL;
    home_last_activity_time = 0;
    non_home_last_activity_time = 0;
    last_active_screen_id = SCREEN_MAIN;
}

void inactivity_timer_start(InactivityContext context) {
    if (context == INACTIVITY_CONTEXT_HOME) {
        if (!home_timer) {
            home_last_activity_time = lv_tick_get();
            home_timer = lv_timer_create(home_inactivity_callback, 1000, NULL);
        }
    } else {
        if (!non_home_timer) {
            non_home_last_activity_time = lv_tick_get();
            non_home_timer = lv_timer_create(non_home_inactivity_callback, 1000, NULL);
        } else {
            reset_non_home_timer();
            lv_timer_resume(non_home_timer);
        }
    }
}

void inactivity_timer_stop(InactivityContext context) {
    if (context == INACTIVITY_CONTEXT_HOME) {
        if (home_timer) {
            lv_timer_del(home_timer);
            home_timer = NULL;

            // Stop video if playing
            if (video_is_playing()) {
                video_stop();
                video_hide();
            }
        }
    } else {
        if (non_home_timer) {
            lv_timer_pause(non_home_timer);
        }
    }
}

void inactivity_timer_pause(InactivityContext context) {
    if (context == INACTIVITY_CONTEXT_HOME) {
        if (home_timer) {
            lv_timer_pause(home_timer);
        }
    } else {
        if (non_home_timer) {
            lv_timer_pause(non_home_timer);
        }
    }
}

void inactivity_timer_resume(InactivityContext context) {
    if (context == INACTIVITY_CONTEXT_HOME) {
        if (home_timer) {
            home_last_activity_time = lv_tick_get();
            lv_timer_resume(home_timer);
        }
    } else {
        if (non_home_timer) {
            non_home_last_activity_time = lv_tick_get();
            lv_timer_resume(non_home_timer);
        }
    }
}

void inactivity_timer_reset(InactivityContext context) {
    if (context == INACTIVITY_CONTEXT_HOME) {
        reset_home_timer();
    } else {
        reset_non_home_timer();
    }
}

void inactivity_home_activity_cb(lv_event_t *e) {
    (void)e;
    reset_home_timer();
}

void inactivity_non_home_activity_cb(lv_event_t *e) {
    (void)e;

    extern ScreenState screen_stack[];
    extern int screen_stack_top;

    // Check if we're currently viewing the home screen (not at stack top position)
    lv_obj_t *current_screen = lv_scr_act();
    bool viewing_home = false;

    // Check if current displayed screen is home screen but we're not at position 0
    if (screen_stack_top >= 0) {
        for (int i = 0; i <= screen_stack_top; i++) {
            if (screen_stack[i].screen_id == SCREEN_MAIN &&
                screen_stack[i].screen == current_screen &&
                i != screen_stack_top) {
                // We're viewing home screen but there's a deeper screen in the stack
                viewing_home = true;
                break;
            }
        }
    }

    if (viewing_home && screen_stack_top > 0) {
        // Return to the screen at the top of the stack (preserves full navigation path)
        lv_scr_load(screen_stack[screen_stack_top].screen);

        // Update status bar and title
        move_status_bar_to_screen(screen_stack[screen_stack_top].screen,
                                  screen_stack[screen_stack_top].screen_id);
        update_title_bar_location(screen_stack[screen_stack_top].screen_id);

        // Restart non-home inactivity timer
        inactivity_timer_start(INACTIVITY_CONTEXT_NON_HOME);

        // Stop home timer and pause slideshow/video
        stop_inactivity_timer();
        slideshow_pause();
        if (video_is_playing()) {
            video_pause();
        }
    } else if (screen_stack_top >= 0 && screen_stack[screen_stack_top].screen_id != SCREEN_MAIN) {
        // On non-home screen, reset inactivity timer
        reset_non_home_timer();
    }
}

// ============================================================================
// INTERNAL HELPER - Store last active screen
// ============================================================================

void inactivity_timer_set_last_screen(int screen_id) {
    last_active_screen_id = screen_id;
}

int inactivity_timer_get_last_screen(void) {
    return last_active_screen_id;
}
