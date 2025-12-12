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

    // Build full path for status bar font
    static char status_bar_font_path[128];
    snprintf(status_bar_font_path, sizeof(status_bar_font_path), "assets/fonts/%s", app_state_get_font_name_status_bar());

    // Load status bar font
    static lv_ft_info_t info_status_bar;
    info_status_bar.name = status_bar_font_path;
    info_status_bar.weight = app_state_get_font_size_status_bar();
    info_status_bar.style = FT_FONT_STYLE_NORMAL;

    if (lv_ft_font_init(&info_status_bar)) {
        app_state_set_font_status_bar(info_status_bar.font);
    } else {
        log_warning("Failed to load status bar font: %s", status_bar_font_path);
        app_state_set_font_status_bar(NULL);
    }

    // Build full path for label font
    static char label_font_path[128];
    snprintf(label_font_path, sizeof(label_font_path), "assets/fonts/%s", app_state_get_font_name_label());

    // Load label font
    static lv_ft_info_t info_label;
    info_label.name = label_font_path;
    info_label.weight = app_state_get_font_size_label();
    info_label.style = FT_FONT_STYLE_NORMAL;

    if (lv_ft_font_init(&info_label)) {
        app_state_set_font_label(info_label.font);
    } else {
        log_warning("Failed to load label font: %s", label_font_path);
        app_state_set_font_label(NULL);
    }

    // Build full path for home screen contents font
    static char home_contents_font_path[128];
    snprintf(home_contents_font_path, sizeof(home_contents_font_path), "assets/fonts/%s", app_state_get_font_name_home_contents());

    // Load home screen contents font
    static lv_ft_info_t info_home_contents;
    info_home_contents.name = home_contents_font_path;
    info_home_contents.weight = app_state_get_font_size_home_contents();
    info_home_contents.style = FT_FONT_STYLE_NORMAL;

    if (lv_ft_font_init(&info_home_contents)) {
        app_state_set_font_home_contents(info_home_contents.font);
    } else {
        log_warning("Failed to load home screen contents font: %s", home_contents_font_path);
        app_state_set_font_home_contents(NULL);
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
// FONT RELOADING
// ============================================================================

/**
 * Reloads the title bar font with current settings
 * @return 0 on success, -1 on failure
 */
int reload_title_font(void) {
    char title_font_path[128];
    snprintf(title_font_path, sizeof(title_font_path), "assets/fonts/%s", app_state_get_font_name_title());

    lv_ft_info_t info;
    info.name = title_font_path;
    info.weight = app_state_get_font_size_title_bar();
    info.style = FT_FONT_STYLE_NORMAL;

    if (lv_ft_font_init(&info)) {
        app_state_set_font_20(info.font);
        return 0;
    } else {
        log_warning("Failed to reload title bar font: %s", title_font_path);
        return -1;
    }
}

/**
 * Reloads the status bar font with current settings
 * @return 0 on success, -1 on failure
 */
int reload_status_bar_font(void) {
    char status_bar_font_path[128];
    snprintf(status_bar_font_path, sizeof(status_bar_font_path), "assets/fonts/%s", app_state_get_font_name_status_bar());

    lv_ft_info_t info;
    info.name = status_bar_font_path;
    info.weight = app_state_get_font_size_status_bar();
    info.style = FT_FONT_STYLE_NORMAL;

    if (lv_ft_font_init(&info)) {
        app_state_set_font_status_bar(info.font);
        return 0;
    } else {
        log_warning("Failed to reload status bar font: %s", status_bar_font_path);
        return -1;
    }
}

/**
 * Reloads the button font with current settings
 * @return 0 on success, -1 on failure
 */
int reload_button_font(void) {
    char button_font_path[128];
    snprintf(button_font_path, sizeof(button_font_path), "assets/fonts/%s", app_state_get_font_name_button_label());

    lv_ft_info_t info;
    info.name = button_font_path;
    info.weight = app_state_get_font_size_button_label();
    info.style = FT_FONT_STYLE_NORMAL;

    if (lv_ft_font_init(&info)) {
        app_state_set_font_button(info.font);
        return 0;
    } else {
        log_warning("Failed to reload button font: %s", button_font_path);
        return -1;
    }
}

/**
 * Reloads the label font with current settings
 * @return 0 on success, -1 on failure
 */
int reload_label_font(void) {
    char label_font_path[128];
    snprintf(label_font_path, sizeof(label_font_path), "assets/fonts/%s", app_state_get_font_name_label());

    lv_ft_info_t info;
    info.name = label_font_path;
    info.weight = app_state_get_font_size_label();
    info.style = FT_FONT_STYLE_NORMAL;

    if (lv_ft_font_init(&info)) {
        app_state_set_font_label(info.font);
        return 0;
    } else {
        log_warning("Failed to reload label font: %s", label_font_path);
        return -1;
    }
}

/**
 * Reloads the home screen contents font with current settings
 * @return 0 on success, -1 on failure
 */
int reload_home_contents_font(void) {
    char home_contents_font_path[128];
    snprintf(home_contents_font_path, sizeof(home_contents_font_path), "assets/fonts/%s", app_state_get_font_name_home_contents());

    lv_ft_info_t info;
    info.name = home_contents_font_path;
    info.weight = app_state_get_font_size_home_contents();
    info.style = FT_FONT_STYLE_NORMAL;

    if (lv_ft_font_init(&info)) {
        app_state_set_font_home_contents(info.font);
        return 0;
    } else {
        log_warning("Failed to reload home screen contents font: %s", home_contents_font_path);
        return -1;
    }
}

// ============================================================================
// UI UPDATE HELPERS
// ============================================================================

// Forward declarations
static void update_button_fonts_recursive(lv_obj_t *obj, lv_font_t *font);

/**
 * Recursively updates all label fonts in a widget tree
 */
static void update_label_fonts_recursive(lv_obj_t *obj, lv_font_t *font) {
    if (!obj || !font) return;

    // Check if this object is a label
    if (lv_obj_check_type(obj, &lv_label_class)) {
        lv_obj_set_style_text_font(obj, font, 0);
    }

    // Recursively update children
    uint32_t child_cnt = lv_obj_get_child_cnt(obj);
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(obj, i);
        update_label_fonts_recursive(child, font);
    }
}

/**
 * Updates all title bar labels with the current title font
 * ONLY affects title bar labels - nothing else
 */
void update_title_bar_fonts(void) {
    lv_font_t *font = app_state_get_font_20();
    if (!font) return;

    // Update known title labels
    if (app_state_get_title_label()) {
        lv_obj_set_style_text_font(app_state_get_title_label(), font, 0);
    }
    if (app_state_get_current_title_label()) {
        lv_obj_set_style_text_font(app_state_get_current_title_label(), font, 0);
    }

    // Update title bar if it exists - ONLY title bar, nothing else
    if (app_state_get_title_bar()) {
        update_label_fonts_recursive(app_state_get_title_bar(), font);
    }
}

/**
 * Updates all status bar labels with the current status bar font
 * This includes status bar button labels (menu and exit buttons)
 */
void update_status_bar_fonts(void) {
    lv_font_t *font = app_state_get_font_status_bar();
    if (!font) return;

    // Update known status bar button labels
    if (app_state_get_menu_button_label()) {
        lv_obj_set_style_text_font(app_state_get_menu_button_label(), font, 0);
    }
    if (app_state_get_exit_button_label()) {
        lv_obj_set_style_text_font(app_state_get_exit_button_label(), font, 0);
    }

    // Update status bar if it exists (this will update all labels in status bar)
    if (app_state_get_status_bar()) {
        update_label_fonts_recursive(app_state_get_status_bar(), font);
    }
}

/**
 * Updates all button labels with the current button font
 * ONLY affects button labels - not status bar buttons (they use status bar font)
 */
void update_button_fonts(void) {
    lv_font_t *font = app_state_get_font_button();
    if (!font) return;

    // Note: menu_button_label and exit_button_label are status bar buttons,
    // so they use status bar font, not button font. They are NOT updated here.

    // Update current active screen (including admin screen)
    lv_obj_t *active_screen = lv_scr_act();
    if (active_screen) {
        // Find all buttons and update their labels
        // Skip status bar buttons - they use status bar font
        // Skip title bar - it doesn't have buttons
        uint32_t child_cnt = lv_obj_get_child_cnt(active_screen);
        for (uint32_t i = 0; i < child_cnt; i++) {
            lv_obj_t *child = lv_obj_get_child(active_screen, i);
            // Skip status bar - its buttons use status bar font
            // Skip title bar - it doesn't have buttons
            if (child != app_state_get_status_bar() && child != app_state_get_title_bar()) {
                update_button_fonts_recursive(child, font);
            }
        }
    }

    // Also update stored screen if different from active screen
    lv_obj_t *screen = app_state_get_screen();
    if (screen && screen != active_screen) {
        uint32_t child_cnt = lv_obj_get_child_cnt(screen);
        for (uint32_t i = 0; i < child_cnt; i++) {
            lv_obj_t *child = lv_obj_get_child(screen, i);
            if (child != app_state_get_status_bar() && child != app_state_get_title_bar()) {
                update_button_fonts_recursive(child, font);
            }
        }
    }
}

/**
 * Recursively updates all button label fonts
 */
static void update_button_fonts_recursive(lv_obj_t *obj, lv_font_t *font) {
    if (!obj || !font) return;

    // Check if this object is a button
    if (lv_obj_check_type(obj, &lv_btn_class)) {
        // Find label child and update its font
        uint32_t child_cnt = lv_obj_get_child_cnt(obj);
        for (uint32_t i = 0; i < child_cnt; i++) {
            lv_obj_t *child = lv_obj_get_child(obj, i);
            if (lv_obj_check_type(child, &lv_label_class)) {
                lv_obj_set_style_text_font(child, font, 0);
            }
        }
    }

    // Recursively update children
    uint32_t child_cnt = lv_obj_get_child_cnt(obj);
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(obj, i);
        update_button_fonts_recursive(child, font);
    }
}

/**
 * Recursively updates label fonts, but skips labels that are children of buttons
 * (those should use button font), title bar labels (use title font), and status bar labels (use status bar font)
 */
static void update_label_fonts_recursive_selective(lv_obj_t *obj, lv_font_t *font, bool skip_buttons) {
    if (!obj || !font) return;

    // Skip title bar - its labels use title font
    if (obj == app_state_get_title_bar()) {
        return;
    }

    // Skip status bar - its labels use status bar font
    if (obj == app_state_get_status_bar()) {
        return;
    }

    // Check if this object is a button - if so, skip its children
    if (lv_obj_check_type(obj, &lv_btn_class)) {
        skip_buttons = true;
    }

    // Check if this object is a label
    if (lv_obj_check_type(obj, &lv_label_class)) {
        // Skip if this label is a child of a button (button labels use button font)
        if (!skip_buttons) {
            // Also skip title labels (they use title font) and status bar labels (they use status bar font)
            lv_obj_t *parent = lv_obj_get_parent(obj);
            if (parent != app_state_get_title_bar() && parent != app_state_get_status_bar()) {
                lv_obj_set_style_text_font(obj, font, 0);
            }
        }
    }

    // Recursively update children
    uint32_t child_cnt = lv_obj_get_child_cnt(obj);
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(obj, i);
        update_label_fonts_recursive_selective(child, font, skip_buttons);
    }
}

/**
 * Updates all regular labels with the current label font
 * Skips button labels (which use button font) and title labels (which use title font)
 */
void update_label_fonts(void) {
    lv_font_t *font = app_state_get_font_label();
    if (!font) return;

    // Update current active screen (including admin screen)
    lv_obj_t *active_screen = lv_scr_act();
    if (active_screen) {
        update_label_fonts_recursive_selective(active_screen, font, false);
    }

    // Also update stored screen if different from active screen
    lv_obj_t *screen = app_state_get_screen();
    if (screen && screen != active_screen) {
        update_label_fonts_recursive_selective(screen, font, false);
    }
}

void update_home_contents_fonts(void) {
    lv_font_t *font = app_state_get_font_home_contents();
    if (!font) return;

    // Update welcome message label
    if (app_state_get_welcome_label()) {
        lv_obj_set_style_text_font(app_state_get_welcome_label(), font, 0);
    }
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
