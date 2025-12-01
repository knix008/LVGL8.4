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

// ============================================================================
// GLOBAL APPLICATION STATE
// ============================================================================

AppState app_state = {
    .screen = NULL,
    .title_bar = NULL,
    .title_label = NULL,
    .current_title_label = NULL,
    .font_20 = NULL,
    .status_bar = NULL,
    .menu_item_selected = {false, false, false, false},
    .status_icons = {NULL, NULL, NULL, NULL}
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

    // Initialize SDL2
    if (init_sdl() != 0) {
        return 1;
    }

    // Initialize LVGL
    if (init_lvgl() != 0) {
        return 1;
    }

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

        lv_timer_handler();
        SDL_Delay(FRAME_DELAY_MS);
    }

    // Cleanup is handled by the OS on exit
    return 0;
}
