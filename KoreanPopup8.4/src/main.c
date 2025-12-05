#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <SDL2/SDL.h>
#include "lvgl/lvgl.h"
#include "lvgl/src/extra/libs/sjpg/lv_sjpg.h"

#include "../include/config.h"
#include "../include/types.h"
#include "../include/screen.h"
#include "../include/style.h"
#include "../include/init.h"
#include "../include/home.h"
#include "../include/label.h"
#include "../include/logger.h"
#include "../include/font.h"

// ============================================================================
// GLOBAL APPLICATION STATE
// ============================================================================

AppState app_state = {
    .screen = NULL,
    .title_bar = NULL,
    .title_label = NULL,
    .current_title_label = NULL,
    .font_20 = NULL,
    .font_button = NULL,
    .status_bar = NULL,
    .menu_item_selected = {false, false, false, false, false},
    .status_icons = {NULL, NULL, NULL, NULL, NULL},
    .bg_color = COLOR_BG_DARK,
    .title_bar_color = COLOR_BG_TITLE,
    .status_bar_color = COLOR_BG_TITLE,
    .button_color = COLOR_BUTTON_BG,
    .button_border_color = COLOR_BORDER,
    .current_language = "ko",
    .font_size_title_bar = FONT_SIZE_TITLE_BAR,
    .font_size_label = FONT_SIZE_REGULAR,
    .font_size_button_label = FONT_SIZE_BUTTON,
    .font_size_bold = FONT_SIZE_BOLD,
    .font_name_title = "NotoSansKR-Bold.ttf",
    .font_name_status_bar = "NotoSansKR-Regular.ttf",
    .font_name_button_label = "NotoSansKR-Medium.ttf"
};
ScreenState screen_stack[MAX_SCREENS];
int screen_stack_top = -1;

// ============================================================================
// MAIN EVENT LOOP
// ============================================================================

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    setlocale(LC_ALL, "");

    // Initialize logging system
    if (log_init() != 0) {
        fprintf(stderr, "Warning: Failed to initialize logging system\n");
    }

    // Initialize SDL2
    if (init_sdl() != 0) {
        log_error("Failed to initialize SDL2");
        log_close();
        return 1;
    }

    // Initialize LVGL
    if (init_lvgl() != 0) {
        return 1;
    }

    // Load labels (default to Korean)
    if (load_labels() != 0) {
        log_warning("Failed to load labels, using defaults");
    }

    // Load configuration
    load_status_bar_config();
    load_theme_config();
    load_font_config();

    // Set language based on loaded config
    set_language(app_state.current_language);

    // Create GUI
    create_gui();

    // Main event loop
    int running = 1;
    uint32_t last_time = SDL_GetTicks();

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                }
            }
        }

        uint32_t current_time = SDL_GetTicks();
        uint32_t elapsed = current_time - last_time;
        if (elapsed > 0) {
            lv_tick_inc(elapsed);
            last_time = current_time;
        }

        uint32_t sleep_time = lv_timer_handler();
        
        // Only delay if LVGL has no pending tasks
        if (sleep_time > 0) {
            SDL_Delay(sleep_time < FRAME_DELAY_MS ? sleep_time : FRAME_DELAY_MS);
        }
    }

    // Close logging system
    log_close();

    // Cleanup is handled by the OS on exit
    return 0;
}
