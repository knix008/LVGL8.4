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
#include "../include/state/app_state.h"

// ============================================================================
// SCREEN STACK (still global, will be encapsulated in Phase 3.5)
// ============================================================================

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

    // Initialize application state
    if (app_state_init() != 0) {
        log_error("Failed to initialize application state");
        log_close();
        return 1;
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
    set_language(app_state_get_language());

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
