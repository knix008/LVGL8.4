#include "lvgl/lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "lvgl/src/extra/libs/sjpg/lv_sjpg.h"

// Display dimensions
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 640
#define BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10)

// SDL globals
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

// Application state
typedef struct {
    lv_obj_t *screen;
    lv_obj_t *title_bar;
    lv_obj_t *title_label;
    lv_font_t *font_20;  // Main font for title
} AppState;

static AppState app_state = {NULL, NULL, NULL, NULL};

// Global display buffer with double-buffering
static lv_disp_draw_buf_t disp_draw_buf;
static lv_color_t buf1[BUF_SIZE];
static lv_color_t buf2[BUF_SIZE];
static lv_indev_drv_t indev_drv;
static lv_indev_t *indev = NULL;

// Exit button callback
static void exit_btn_callback(lv_event_t *e) {
    (void)e;  // Unused
    // Set running flag to 0 to exit main loop
    exit(0);  // Exit the program
}

// Update title bar with current date and time
static void update_title_bar(void) {
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);

    const char *days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

    char title_text[256];
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

// Create the GUI
static void create_gui(void) {
    // Create main screen
    app_state.screen = lv_scr_act();

    // Create background image widget using lv_img (proper image widget)
    lv_obj_t *bg_img = lv_img_create(app_state.screen);
    lv_img_set_src(bg_img, "A:assets/background-bikini-woman-big.jpg");

    // Set explicit size to match screen resolution (320x640)
    lv_obj_set_width(bg_img, SCREEN_WIDTH);
    lv_obj_set_height(bg_img, SCREEN_HEIGHT);

    // Position the image to fill the screen
    lv_obj_align(bg_img, LV_ALIGN_TOP_LEFT, 0, 0);

    // Move image to background so other objects appear on top
    lv_obj_move_background(bg_img);

    // Create title bar area (container for positioning)
    app_state.title_bar = lv_obj_create(app_state.screen);
    lv_obj_set_size(app_state.title_bar, SCREEN_WIDTH, 60);
    lv_obj_align(app_state.title_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(app_state.title_bar, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_bg_opa(app_state.title_bar, 128, 0);  // 50% transparency
    lv_obj_set_style_border_width(app_state.title_bar, 0, 0);  // No border
    lv_obj_set_style_radius(app_state.title_bar, 0, 0);  // Sharp corners (no rounding)
    lv_obj_set_style_pad_all(app_state.title_bar, 5, 0);

    // Create title label - directly on title bar
    app_state.title_label = lv_label_create(app_state.title_bar);
    lv_obj_set_style_text_color(app_state.title_label, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_long_mode(app_state.title_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(app_state.title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(app_state.title_label, SCREEN_WIDTH - 20);
    // Center label both horizontally and vertically in the title bar
    lv_obj_align(app_state.title_label, LV_ALIGN_CENTER, 0, 0);

    update_title_bar();  // Initial update

    // Create a timer to update the title bar every second
    lv_timer_create(
        (lv_timer_cb_t)update_title_bar,
        1000,  // 1 second
        NULL
    );

    // Create status bar at the bottom
    lv_obj_t *status_bar = lv_obj_create(app_state.screen);
    lv_obj_set_size(status_bar, SCREEN_WIDTH, 50);
    lv_obj_align(status_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_bg_opa(status_bar, 128, 0);  // 50% transparency
    lv_obj_set_style_border_width(status_bar, 0, 0);  // No border
    lv_obj_set_style_radius(status_bar, 0, 0);  // Sharp corners
    lv_obj_set_style_pad_all(status_bar, 5, 0);

    // Create menu buttons on the status bar
    // Button 1 - Menu (메뉴)
    lv_obj_t *btn1 = lv_btn_create(status_bar);
    lv_obj_set_size(btn1, 60, 40);
    lv_obj_align(btn1, LV_ALIGN_LEFT_MID, 5, 0);
    lv_obj_set_style_bg_color(btn1, lv_color_hex(0x1A1A1A), 0);  // Dark black
    lv_obj_set_style_border_width(btn1, 1, 0);
    lv_obj_set_style_border_color(btn1, lv_color_hex(0x888888), 0);
    lv_obj_t *label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "메뉴");
    lv_obj_set_style_text_color(label1, lv_color_hex(0xFFFFFF), 0);  // White text
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);  // Center label in button
    if (app_state.font_20) {
        lv_obj_set_style_text_font(label1, app_state.font_20, 0);
    }

    // Button 2 - Back (뒤로)
    lv_obj_t *btn2 = lv_btn_create(status_bar);
    lv_obj_set_size(btn2, 60, 40);
    lv_obj_align(btn2, LV_ALIGN_LEFT_MID, 70, 0);
    lv_obj_set_style_bg_color(btn2, lv_color_hex(0x1A1A1A), 0);  // Dark black
    lv_obj_set_style_border_width(btn2, 1, 0);
    lv_obj_set_style_border_color(btn2, lv_color_hex(0x888888), 0);
    lv_obj_t *label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "뒤로");
    lv_obj_set_style_text_color(label2, lv_color_hex(0xFFFFFF), 0);  // White text
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);  // Center label in button
    if (app_state.font_20) {
        lv_obj_set_style_text_font(label2, app_state.font_20, 0);
    }

    // Button 3 (right side) - Exit button (종료)
    lv_obj_t *btn3 = lv_btn_create(status_bar);
    lv_obj_set_size(btn3, 60, 40);
    lv_obj_align(btn3, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_set_style_bg_color(btn3, lv_color_hex(0x1A1A1A), 0);  // Dark black
    lv_obj_set_style_border_width(btn3, 1, 0);
    lv_obj_set_style_border_color(btn3, lv_color_hex(0x888888), 0);
    lv_obj_t *label3 = lv_label_create(btn3);
    lv_label_set_text(label3, "종료");
    lv_obj_set_style_text_color(label3, lv_color_hex(0xFFFFFF), 0);  // White text
    lv_obj_align(label3, LV_ALIGN_CENTER, 0, 0);  // Center label in button
    if (app_state.font_20) {
        lv_obj_set_style_text_font(label3, app_state.font_20, 0);
    }

    // Add exit callback to Exit button
    lv_obj_add_event_cb(btn3, exit_btn_callback, LV_EVENT_CLICKED, NULL);
}

// Initialize fonts
static int init_fonts(void) {
    // Initialize FreeType for Korean font support
    if (!lv_freetype_init(0, 0, 0)) {
        fprintf(stderr, "Warning: FreeType initialization failed\n");
    }

    // Load NotoSansKR font for Korean character support
    static lv_ft_info_t info;
    info.name = "assets/NotoSansKR-Regular.ttf";
    info.weight = 16;
    info.style = FT_FONT_STYLE_NORMAL;
    if (lv_ft_font_init(&info)) {
        app_state.font_20 = info.font;
        fprintf(stderr, "NotoSansKR font loaded successfully\n");
    } else {
        fprintf(stderr, "Warning: Failed to load NotoSansKR font\n");
        app_state.font_20 = NULL;
    }

    return 0;
}

// Input device read callback for mouse/pointer
static void indev_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    (void)drv;

    int x, y;
    uint32_t mouse_state = SDL_GetMouseState(&x, &y);

    data->point.x = x;
    data->point.y = y;
    data->state = (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

// SDL display flush callback for LVGL
static void display_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    if (renderer == NULL || texture == NULL) {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    // Lock texture for direct pixel access
    void *pixels;
    int pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    uint32_t *pixel_data = (uint32_t *)pixels;
    (void)pitch;

    // Copy the LVGL framebuffer to SDL2 texture
    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            int index = y * SCREEN_WIDTH + x;
            uint32_t color = lv_color_to32(*color_p);
            pixel_data[index] = color;
            color_p++;
        }
    }

    SDL_UnlockTexture(texture);

    // Render to screen
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    lv_disp_flush_ready(disp_drv);
}

// Initialize SDL2
static int init_sdl(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow(
        "LVGL Title Bar Application",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
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
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    return 0;
}

    // Initialize LVGL
    static int init_lvgl(void) {
    lv_init();

    // Initialize extra components (file system, image decoders, etc.)
    lv_extra_init();

    // Initialize the draw buffer with double-buffering
    lv_disp_draw_buf_init(&disp_draw_buf, buf1, buf2, BUF_SIZE);

    // Create display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    // Set display driver parameters
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = display_flush_cb;
    disp_drv.draw_buf = &disp_draw_buf;

    // Register display driver
    lv_disp_drv_register(&disp_drv);

    // Initialize input device driver
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = indev_read;
    indev = lv_indev_drv_register(&indev_drv);

    // Initialize fonts
    if (init_fonts() != 0) {
        fprintf(stderr, "Warning: Font initialization had issues\n");
    }

    return 0;
}

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
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                }
            }
        }

        // Update LVGL timing
        uint32_t current_time = SDL_GetTicks();
        uint32_t elapsed = current_time - last_time;
        if (elapsed > 0) {
            lv_tick_inc(elapsed);
            last_time = current_time;
        }

        // Handle LVGL tasks
        lv_timer_handler();

        // Small delay to reduce CPU usage
        SDL_Delay(5);
    }

    // Cleanup
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
