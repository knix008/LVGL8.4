#include "../include/init.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/label.h"
#include "../include/logger.h"
#include "../include/font.h"
#include "../include/state.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#include "lvgl/lvgl.h"

// ============================================================================
// SDL GLOBALS
// ============================================================================

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

// ============================================================================
// LVGL GLOBALS
// ============================================================================

static lv_disp_draw_buf_t disp_draw_buf;
static lv_color_t buf1[BUF_SIZE];
static lv_color_t buf2[BUF_SIZE];
static lv_indev_drv_t indev_drv;
static lv_indev_t *indev = NULL;

// ============================================================================
// FONT INITIALIZATION
// ============================================================================

/**
 * Initializes custom fonts used by the application.
 * Loads NotoSansKR font for Korean text support.
 *
 * @return 0 on success, -1 on failure
 */
int init_fonts(void) {
    if (!lv_freetype_init(0, 0, 0)) {
        log_warning("FreeType initialization failed");
        return -1;
    }

    // Build full path for title font
    static char title_font_path[128];
    snprintf(title_font_path, sizeof(title_font_path), "assets/fonts/%s", app_state_get_font_name_title());

    // Load title bar font
    static lv_ft_info_t info;
    info.name = title_font_path;
    info.weight = app_state_get_font_size_title_bar();
    info.style = FT_FONT_STYLE_NORMAL;

    if (lv_ft_font_init(&info)) {
        app_state_set_font_20(info.font);
    } else {
        log_warning("Failed to load title bar font: %s", title_font_path);
        app_state_set_font_20(NULL);
    }

    // Build full path for button font
    static char button_font_path[128];
    snprintf(button_font_path, sizeof(button_font_path), "assets/fonts/%s", app_state_get_font_name_button_label());

    // Load button font
    static lv_ft_info_t info_button;
    info_button.name = button_font_path;
    info_button.weight = app_state_get_font_size_button_label();
    info_button.style = FT_FONT_STYLE_NORMAL;

    if (lv_ft_font_init(&info_button)) {
        app_state_set_font_button(info_button.font);
    } else {
        log_warning("Failed to load button font: %s", button_font_path);
        app_state_set_font_button(NULL);
    }

    // Load bold font (for welcome message) - still using FONT_PATH_BOLD for backward compatibility
    static lv_ft_info_t info_bold;
    info_bold.name = FONT_PATH_BOLD;
    info_bold.weight = app_state_get_font_size_bold();
    info_bold.style = FT_FONT_STYLE_NORMAL;

    if (lv_ft_font_init(&info_bold)) {
        app_state_set_font_24_bold(info_bold.font);
    } else {
        log_warning("Failed to load bold font");
        app_state_set_font_24_bold(NULL);
    }

    return 0;
}

// ============================================================================
// DISPLAY DRIVER
// ============================================================================

static void indev_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    (void)drv;

    int x, y;
    uint32_t mouse_state = SDL_GetMouseState(&x, &y);

    data->point.x = x;
    data->point.y = y;
    data->state = (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

static void display_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    if (renderer == NULL || texture == NULL) {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    void *pixels;
    int pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    uint32_t *pixel_data = (uint32_t *)pixels;
    (void)pitch;

    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            int index = y * SCREEN_WIDTH + x;
            uint32_t color = lv_color_to32(*color_p);
            pixel_data[index] = color;
            color_p++;
        }
    }

    SDL_UnlockTexture(texture);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    lv_disp_flush_ready(disp_drv);
}

// ============================================================================
// SDL INITIALIZATION
// ============================================================================

/**
 * Initializes the SDL2 subsystem for rendering.
 * Sets up window, display driver, and input devices.
 * 
 * @return 0 on success, -1 on failure
 */
int init_sdl(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        log_error("Failed to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow(
        "LVGL Menu Application",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        log_error("Failed to create SDL window: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        log_error("Failed to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );

    if (texture == NULL) {
        log_error("Failed to create texture: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    return 0;
}

// ============================================================================
// LVGL INITIALIZATION
// ============================================================================

/**
 * Initializes the LVGL graphics library.
 * Sets up display buffer and registers display/input drivers.
 * 
 * @return 0 on success, -1 on failure
 */
int init_lvgl(void) {
    lv_init();
    lv_extra_init();

    lv_disp_draw_buf_init(&disp_draw_buf, buf1, buf2, BUF_SIZE);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = display_flush_cb;
    disp_drv.draw_buf = &disp_draw_buf;

    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = indev_read;
    indev = lv_indev_drv_register(&indev_drv);

    if (init_fonts() != 0) {
        log_warning("Font initialization had issues");
    }

    return 0;
}
