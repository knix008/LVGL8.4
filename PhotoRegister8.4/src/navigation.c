#include <stdio.h>
#include "../include/navigation.h"
#include "../include/screen.h"
#include "../include/config.h"

// ============================================================================
// COMMON NAVIGATION CALLBACKS
// ============================================================================

/**
 * Handles back button press events.
 * Navigates to the home screen.
 * 
 * @param e The LVGL event object
 */
void back_btn_callback(lv_event_t *e) {
    (void)e;
    if (screen_stack_top > 0) {
        screen_stack_top--;
        show_screen(screen_stack[screen_stack_top].screen_id);
    }
}

/**
 * Handles info button press events.
 * Navigates to the info screen.
 * 
 * @param e The LVGL event object
 */
void info_btn_callback(lv_event_t *e) {
    (void)e;
    if (screen_stack[screen_stack_top].screen_id != SCREEN_INFO) {
        // Navigate using absolute path: clear stack to MENU then go to INFO
        screen_stack_top = 0;  // Reset to MAIN
        show_screen(SCREEN_MENU);  // Go through MENU
        show_screen(SCREEN_INFO);  // Then to INFO
    }
}

/**
 * Handles admin button press events.
 * Navigates to the admin settings screen.
 * 
 * @param e The LVGL event object
 */
void admin_btn_callback(lv_event_t *e) {
    (void)e;
    if (screen_stack[screen_stack_top].screen_id != SCREEN_ADMIN) {
        // Navigate using absolute path: clear stack to MENU then go to ADMIN
        screen_stack_top = 0;  // Reset to MAIN
        show_screen(SCREEN_MENU);  // Go through MENU
        show_screen(SCREEN_ADMIN);  // Then to ADMIN
    }
}

/**
 * Handles network button press events.
 * Navigates to the network configuration screen.
 * 
 * @param e The LVGL event object
 */
void network_btn_callback(lv_event_t *e) {
    (void)e;
    if (screen_stack[screen_stack_top].screen_id != SCREEN_NETWORK) {
        // Navigate using absolute path: clear stack to MENU then go to NETWORK
        screen_stack_top = 0;  // Reset to MAIN
        show_screen(SCREEN_MENU);  // Go through MENU
        show_screen(SCREEN_NETWORK);  // Then to NETWORK
    }
}

void korean_input_btn_callback(lv_event_t *e) {
    (void)e;
    if (screen_stack[screen_stack_top].screen_id != SCREEN_KOREAN_INPUT) {
        // Navigate using absolute path: clear stack to MENU then go to KOREAN_INPUT
        screen_stack_top = 0;  // Reset to MAIN
        show_screen(SCREEN_MENU);  // Go through MENU
        show_screen(SCREEN_KOREAN_INPUT);  // Then to KOREAN_INPUT
    }
}

void settings_btn_callback(lv_event_t *e) {
    (void)e;
    if (screen_stack[screen_stack_top].screen_id != SCREEN_CAMERA) {
        // Navigate using absolute path: clear stack to MENU then go to CAMERA
        screen_stack_top = 0;  // Reset to MAIN
        show_screen(SCREEN_MENU);  // Go through MENU
        show_screen(SCREEN_CAMERA);  // Then to CAMERA
    }
}

void number_input_btn_callback(lv_event_t *e) {
    (void)e;
    if (screen_stack[screen_stack_top].screen_id != SCREEN_NUMBER_INPUT) {
        // Navigate using absolute path: clear stack to MENU then go to NUMBER_INPUT
        screen_stack_top = 0;  // Reset to MAIN
        show_screen(SCREEN_MENU);  // Go through MENU
        show_screen(SCREEN_NUMBER_INPUT);  // Then to NUMBER_INPUT
    }
}
