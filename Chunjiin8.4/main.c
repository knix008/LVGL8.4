/*
* Chunjiin Korean Input Method - LVGL 8.4 GUI
* Main application file with SDL2 integration
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <locale.h>
#include <time.h>
#include <unistd.h>
#include <SDL2/SDL.h>

#include "lvgl/lvgl.h"
#include "lvgl/src/extra/libs/freetype/lv_freetype.h"
#include "chunjiin.h"

/*******************************************************************************
 * Display Configuration (LVGL 8.4 with SDL2)
 ******************************************************************************/
#define DISP_HOR_RES 320
#define DISP_VER_RES 640
#define BUF_SIZE (DISP_HOR_RES * DISP_VER_RES / 10)  // 10% of screen for double-buffering

static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf1[BUF_SIZE];
static lv_color_t buf2[BUF_SIZE];

// SDL2 Window and Renderer
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

// Input device driver
static lv_indev_drv_t indev_drv;
static lv_indev_t *indev;

typedef struct {
    lv_obj_t *text_area;
    lv_obj_t *buttons[12];
    lv_obj_t *mode_button;
    lv_obj_t *clear_button;
    lv_obj_t *enter_button;
    lv_obj_t *save_button;
    lv_obj_t *load_button;
    ChunjiinState state;
} AppWidgets;

static AppWidgets app_widgets;
static lv_obj_t *active_mbox = NULL; // Track active message box
static lv_font_t *korean_font_16 = NULL;
static lv_font_t *korean_font_20 = NULL;
static lv_font_t *korean_font_16_bold = NULL;
static lv_font_t *korean_font_20_bold = NULL;
static lv_font_t *korean_font_14_bold = NULL;
static lv_font_t *korean_font_14 = NULL;
static lv_font_t *korean_font_12 = NULL;

// Font configuration structure
typedef struct {
    const char *font_path;
    uint16_t size;
    uint32_t style;
    lv_font_t **font_ptr;
    const char *font_name;
} FontConfig;

/*******************************************************************************
 * File I/O Functions
 ******************************************************************************/

#define DEFAULT_SAVE_FILE "chunjiin_text.txt"

/**
 * Save current text buffer to file
 * @param state Current Chunjiin state
 * @param filename File to save to (NULL for default)
 * @return true on success, false on failure
 */
static bool save_text_to_file(ChunjiinState *state, const char *filename) {
    if (!filename) {
        filename = DEFAULT_SAVE_FILE;
    }

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("Failed to open file for writing: %s\n", filename);
        return false;
    }

    // Convert wide string to UTF-8
    char *utf8_text = wchar_to_utf8(state->text_buffer, MAX_TEXT_LEN);
    if (!utf8_text) {
        fclose(fp);
        return false;
    }

    // Write to file
    size_t len = strlen(utf8_text);
    size_t written = fwrite(utf8_text, 1, len, fp);
    fclose(fp);

    if (written != len) {
        printf("Failed to write complete text to file\n");
        return false;
    }

    printf("Saved text to file: %s (%zu bytes)\n", filename, written);
    return true;
}

/**
 * Load text from file to buffer
 * @param state Current Chunjiin state
 * @param filename File to load from (NULL for default)
 * @return true on success, false on failure
 */
static bool load_text_from_file(ChunjiinState *state, const char *filename) {
    if (!filename) {
        filename = DEFAULT_SAVE_FILE;
    }

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Failed to open file for reading: %s\n", filename);
        return false;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size <= 0) {
        printf("File is empty or invalid\n");
        fclose(fp);
        return false;
    }

    // Read file content
    char *utf8_buffer = malloc(file_size + 1);
    if (!utf8_buffer) {
        printf("Failed to allocate memory for file content\n");
        fclose(fp);
        return false;
    }

    size_t bytes_read = fread(utf8_buffer, 1, file_size, fp);
    fclose(fp);
    utf8_buffer[bytes_read] = '\0';

    // Convert UTF-8 to wide string
    size_t converted = mbstowcs(state->text_buffer, utf8_buffer, MAX_TEXT_LEN - 1);
    free(utf8_buffer);

    if (converted == (size_t)-1) {
        printf("Failed to convert UTF-8 to wide string\n");
        return false;
    }

    state->text_buffer[converted] = L'\0';
    state->cursor_pos = wcslen(state->text_buffer);

    printf("Loaded text from file: %s (%zu characters)\n", filename, converted);
    return true;
}

/*******************************************************************************
 * SDL2 Display Driver
 ******************************************************************************/

/**
 * Flush the display buffer to SDL2 texture
 * Called by LVGL whenever display buffer needs to be rendered
 */
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    if (renderer == NULL || texture == NULL) {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    // Lock texture for direct pixel access
    void *pixels;
    int pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    uint32_t *pixel_data = (uint32_t *)pixels;
    (void)pitch;  // Pitch is fixed

    // Copy the LVGL framebuffer to SDL2 texture
    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            int index = y * DISP_HOR_RES + x;
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

/*******************************************************************************
 * Input Device Driver (Mouse/Touch)
 ******************************************************************************/

/**
 * Read input device (mouse/touch) state
 * Called by LVGL to get current input
 */
static void indev_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    (void)drv;  // Unused parameter

    // Get current mouse state
    int x, y;
    uint32_t mouse_state = SDL_GetMouseState(&x, &y);

    data->point.x = x;
    data->point.y = y;
    data->state = (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

// wchar_t buffer to UTF-8 string conversion helper

/**
 * Load a single font with error handling
 * @param font_path Path to the TTF font file
 * @param size Font size in pixels
 * @param style Font style (normal, bold, italic, etc.)
 * @return Pointer to loaded font, or NULL if failed
 */
static lv_font_t* load_korean_font(const char *font_path, uint16_t size, uint16_t style) {
    if (font_path == NULL) {
        LV_LOG_ERROR("Font path is NULL");
        return NULL;
    }

    // Check if font file exists
    if (access(font_path, F_OK) != 0) {
        LV_LOG_ERROR("Font file not found: %s", font_path);
        return NULL;
    }

    // Use LVGL 8.4 FreeType API
    lv_ft_info_t info = {0};
    info.name = font_path;
    info.weight = size;
    info.style = style;

    if (!lv_ft_font_init(&info)) {
        LV_LOG_ERROR("Failed to load font from: %s (size: %d)", font_path, size);
        return NULL;
    }

    printf("✓ Loaded font: %s (size: %d)\n", font_path, size);
    return info.font;
}

/**
 * Initialize all Korean fonts
 * @return true if all fonts loaded successfully, false otherwise
 */
static bool init_all_fonts(void) {
    printf("Initializing Korean fonts...\n");

    // Initialize FreeType library - LVGL 8.4 API with default cache settings
    if (!lv_freetype_init(0, 0, 0)) {
        printf("✗ FreeType initialization failed\n");
        return false;
    }

    // Font files - try regular font first, fallback to coding variant
    const char *font_regular = "assets/NanumGothic-Regular.ttf";
    const char *font_coding = "assets/NanumGothicCoding.ttf";
    const char *font_bold = "assets/NanumGothic-Bold.ttf";
    const char *font_coding_bold = "assets/NanumGothicCoding-Bold.ttf";

    // Determine which font files to use
    const char *font_file = NULL;
    const char *font_file_bold = NULL;

    // Check for regular fonts first
    if (access(font_regular, F_OK) == 0) {
        font_file = font_regular;
        printf("Using NanumGothic-Regular.ttf\n");
    } else if (access(font_coding, F_OK) == 0) {
        font_file = font_coding;
        printf("Using NanumGothicCoding.ttf\n");
    } else {
        LV_LOG_ERROR("No Korean font file found!");
        printf("✗ Font files missing in assets/ directory\n");
        return false;
    }

    // Check for bold fonts
    if (access(font_bold, F_OK) == 0) {
        font_file_bold = font_bold;
    } else if (access(font_coding_bold, F_OK) == 0) {
        font_file_bold = font_coding_bold;
    } else {
        font_file_bold = font_file; // Use regular font if bold not available
    }

    printf("Regular font: %s\n", font_file);
    printf("Bold font: %s\n", font_file_bold);
    printf("\n");

    // Load regular fonts
    korean_font_20 = load_korean_font(font_file, 20, FT_FONT_STYLE_NORMAL);
    korean_font_16 = load_korean_font(font_file, 16, FT_FONT_STYLE_NORMAL);
    korean_font_14 = load_korean_font(font_file, 14, FT_FONT_STYLE_NORMAL);
    korean_font_12 = load_korean_font(font_file, 12, FT_FONT_STYLE_NORMAL);

    // Load bold fonts
    korean_font_20_bold = load_korean_font(font_file_bold, 20, FT_FONT_STYLE_BOLD);
    korean_font_16_bold = load_korean_font(font_file_bold, 16, FT_FONT_STYLE_BOLD);
    korean_font_14_bold = load_korean_font(font_file_bold, 14, FT_FONT_STYLE_BOLD);

    // Verify all fonts loaded successfully
    if (!korean_font_16 || !korean_font_20 || !korean_font_14 || !korean_font_12 ||
        !korean_font_16_bold || !korean_font_20_bold || !korean_font_14_bold) {
        LV_LOG_ERROR("Failed to load one or more Korean fonts!");
        printf("✗ Font initialization failed\n");
        return false;
    }

    printf("✓ All Korean fonts loaded successfully\n");
    return true;
}

// Button click event handler
static void on_button_clicked(lv_event_t *e) {
    int button_num = (int)(intptr_t)lv_event_get_user_data(e);
    //printf("Button clicked: %d, Mode: %d\n", button_num, app_widgets.state.now_mode);

    // Process input
    chunjiin_process_input(&app_widgets.state, button_num);

    // Update text area
    char *utf8_text = wchar_to_utf8(app_widgets.state.text_buffer, MAX_TEXT_LEN);
    //printf("Text buffer after input: %ls\n", app_widgets.state.text_buffer);
    //printf("UTF-8 text: %s\n", utf8_text);
    lv_textarea_set_text(app_widgets.text_area, utf8_text);
}

// Mode button click handler
static void on_mode_button_clicked(lv_event_t *e) {
    (void)e;
    change_mode(&app_widgets.state);

    // Update button labels
    for (int i = 0; i < 12; i++) {
        if (app_widgets.buttons[i] == NULL) continue;
        const wchar_t *wtext = get_button_text(app_widgets.state.now_mode, i);
        char *utf8_text = wchar_to_utf8(wtext, 20);
        lv_obj_t *label = lv_obj_get_child(app_widgets.buttons[i], 0);
        if (label) {
            lv_label_set_text(label, utf8_text);
        }
    }
}

// Clear button handler
static void on_clear_clicked(lv_event_t *e) {
    (void)e;
    
    // Defensive: check if text area is valid
    if (!app_widgets.text_area) {
        printf("Error: Text area not initialized\n");
        return;
    }
    
    // Save current mode
    InputMode current_mode = app_widgets.state.now_mode;

    // Clear text (preserve mode)
    chunjiin_init(&app_widgets.state);
    app_widgets.state.now_mode = current_mode;
    app_widgets.state.cursor_pos = 0;
    memset(app_widgets.state.text_buffer, 0, sizeof(app_widgets.state.text_buffer));
    lv_textarea_set_text(app_widgets.text_area, "");
    
    // Defensive: reset message box pointer
    if (active_mbox) {
        if (lv_obj_is_valid(active_mbox)) {
            lv_obj_del(active_mbox);
        }
        active_mbox = NULL;
    }
}


// Close button event handler
static void on_close_button_clicked(lv_event_t *e) {
    (void)e; // Suppress unused parameter warning
    if (active_mbox && lv_obj_is_valid(active_mbox)) {
        lv_obj_del(active_mbox);
        active_mbox = NULL;
    }
}

// Create persistent result window
static lv_obj_t* create_result_window(const char* title, const char* message) {
    // Create a container for the window
    lv_obj_t *window = lv_obj_create(lv_scr_act());
    if (!window) return NULL;
    
    // Set window properties - fixed height
    lv_obj_set_size(window, 280, 200);
    lv_obj_center(window);
    lv_obj_set_style_bg_opa(window, LV_OPA_90, 0);
    lv_obj_set_style_bg_color(window, lv_color_hex(0x2C2C2C), 0);
    lv_obj_set_style_border_width(window, 3, 0);
    lv_obj_set_style_border_color(window, lv_color_hex(0x4A90E2), 0);
    lv_obj_set_style_radius(window, 15, 0);
    lv_obj_set_style_pad_all(window, 20, 0);
    lv_obj_set_style_shadow_width(window, 20, 0);
    lv_obj_set_style_shadow_color(window, lv_color_black(), 0);
    
    // Create title label
    lv_obj_t *title_label = lv_label_create(window);
    if (title_label) {
        lv_label_set_text(title_label, title);
        lv_obj_set_style_text_color(title_label, lv_color_hex(0x4A90E2), 0);
        lv_obj_set_style_text_font(title_label, korean_font_20, 0);
        lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 5);
    }
    
    // Create message label with fixed height
    lv_obj_t *msg_cont = lv_obj_create(window);
    lv_obj_set_size(msg_cont, 220, 100);
    lv_obj_align(msg_cont, LV_ALIGN_TOP_MID, 0, 40); // Position below title
    lv_obj_set_style_bg_opa(msg_cont, LV_OPA_0, 0);
    lv_obj_set_style_border_width(msg_cont, 0, 0);
    lv_obj_set_style_pad_all(msg_cont, 10, 0);
    
    lv_obj_t *msg_label = lv_label_create(msg_cont);
    if (msg_label) {
        lv_label_set_text(msg_label, message);
        lv_obj_set_style_text_color(msg_label, lv_color_white(), 0);
        lv_obj_set_style_text_font(msg_label, korean_font_16, 0);
        lv_obj_align(msg_label, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_label_set_long_mode(msg_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(msg_label, 200);
    }
    
    // Create button container for cancel and confirm buttons
    lv_obj_t *btn_container = lv_obj_create(window);
    lv_obj_set_size(btn_container, 220, 35);
    lv_obj_align(btn_container, LV_ALIGN_BOTTOM_MID, 0, -15);
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_0, 0);
    lv_obj_set_style_border_width(btn_container, 0, 0);
    lv_obj_set_style_pad_all(btn_container, 0, 0);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Create cancel button
    lv_obj_t *cancel_btn = lv_btn_create(btn_container);
    lv_obj_set_size(cancel_btn, 100, 35);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0x808080), 0);
    lv_obj_set_style_radius(cancel_btn, 8, 0);

    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, "취소");
    lv_obj_set_style_text_color(cancel_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(cancel_label, korean_font_16, 0);
    lv_obj_center(cancel_label);

    lv_obj_add_event_cb(cancel_btn, on_close_button_clicked, LV_EVENT_CLICKED, NULL);

    // Create confirm button
    lv_obj_t *confirm_btn = lv_btn_create(btn_container);
    lv_obj_set_size(confirm_btn, 100, 35);
    lv_obj_set_style_bg_color(confirm_btn, lv_color_hex(0x4A90E2), 0);
    lv_obj_set_style_radius(confirm_btn, 8, 0);

    lv_obj_t *confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, "확인");
    lv_obj_set_style_text_color(confirm_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(confirm_label, korean_font_16, 0);
    lv_obj_center(confirm_label);

    lv_obj_add_event_cb(confirm_btn, on_close_button_clicked, LV_EVENT_CLICKED, NULL);
    
    return window;
}

// Enter button handler - show result window then clear
static void on_enter_clicked(lv_event_t *e) {
    (void)e;

    // Save current mode
    InputMode current_mode = app_widgets.state.now_mode;

    // Clean up any existing message box first
    if (active_mbox) {
        if (lv_obj_is_valid(active_mbox)) {
            lv_obj_del(active_mbox);
        }
        active_mbox = NULL;
    }

    // Get current text
    const char *text = lv_textarea_get_text(app_widgets.text_area);

    // Create window based on content
    if (text == NULL || text[0] == '\0') {
        // If buffer is empty, show a warning window
        active_mbox = create_result_window("주의!!!", "입력된 내용이 없습니다.");
    } else {
        // Create window with input result
        active_mbox = create_result_window("입력 결과", text);

        // Clear text (preserve mode)
        chunjiin_init(&app_widgets.state);
        app_widgets.state.now_mode = current_mode;
        lv_textarea_set_text(app_widgets.text_area, "");
    }
}

// Save button handler - save text to file
static void on_save_clicked(lv_event_t *e) {
    (void)e;

    // Clean up any existing message box first
    if (active_mbox) {
        if (lv_obj_is_valid(active_mbox)) {
            lv_obj_del(active_mbox);
        }
        active_mbox = NULL;
    }

    // Check if there's text to save
    const char *text = lv_textarea_get_text(app_widgets.text_area);
    if (text == NULL || text[0] == '\0') {
        active_mbox = create_result_window("저장 실패", "저장할 내용이 없습니다.");
        return;
    }

    // Save to file
    if (save_text_to_file(&app_widgets.state, NULL)) {
        active_mbox = create_result_window("저장 완료", "파일에 저장되었습니다.\n(chunjiin_text.txt)");
    } else {
        active_mbox = create_result_window("저장 실패", "파일 저장에 실패했습니다.");
    }
}

// Load button handler - load text from file
static void on_load_clicked(lv_event_t *e) {
    (void)e;

    // Clean up any existing message box first
    if (active_mbox) {
        if (lv_obj_is_valid(active_mbox)) {
            lv_obj_del(active_mbox);
        }
        active_mbox = NULL;
    }

    // Save current mode
    InputMode current_mode = app_widgets.state.now_mode;

    // Load from file
    if (load_text_from_file(&app_widgets.state, NULL)) {
        // Restore mode
        app_widgets.state.now_mode = current_mode;

        // Update text area
        char *utf8_text = wchar_to_utf8(app_widgets.state.text_buffer, MAX_TEXT_LEN);
        lv_textarea_set_text(app_widgets.text_area, utf8_text);

        active_mbox = create_result_window("불러오기 완료", "파일에서 불러왔습니다.\n(chunjiin_text.txt)");
    } else {
        active_mbox = create_result_window("불러오기 실패", "파일을 찾을 수 없거나\n읽기에 실패했습니다.");
    }
}

void create_ui(void) {
    chunjiin_init(&app_widgets.state);

    // Initialize all Korean fonts
    if (!init_all_fonts()) {
        printf("Error: Failed to initialize fonts. Exiting.\n");
        return;
    }

    // Create main container
    lv_obj_t *main_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_cont, 320, 640);
    lv_obj_center(main_cont);
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(main_cont, 10, 0);
    lv_obj_set_style_pad_row(main_cont, 10, 0);

    // Title label
    lv_obj_t *title_label = lv_label_create(main_cont);
    lv_label_set_text(title_label, "천지인 한글/영어/숫자/특수키 입력기");
    lv_obj_set_style_text_font(title_label, korean_font_16, 0);

    // Text area (scrollable)
    app_widgets.text_area = lv_textarea_create(main_cont);
    lv_obj_set_size(app_widgets.text_area, 300, 150);
    lv_textarea_set_text(app_widgets.text_area, "");
    lv_obj_set_style_text_font(app_widgets.text_area, korean_font_16, 0);

    // Set font for the textarea's internal label
    lv_obj_t *textarea_label = lv_textarea_get_label(app_widgets.text_area);
    if (textarea_label) {
        lv_obj_set_style_text_font(textarea_label, korean_font_16, 0);
    }

    // Button grid container
    lv_obj_t *button_grid = lv_obj_create(main_cont);
    lv_obj_set_size(button_grid, 300, 330);
    lv_obj_set_style_pad_all(button_grid, 3, 0);
    lv_obj_set_style_pad_row(button_grid, 2, 0);
    lv_obj_set_style_pad_column(button_grid, 2, 0);
    lv_obj_set_layout(button_grid, LV_LAYOUT_GRID);
    lv_obj_set_style_grid_column_align(button_grid, LV_GRID_ALIGN_CENTER, 0);
    lv_obj_set_style_grid_row_align(button_grid, LV_GRID_ALIGN_CENTER, 0);

    // Grid: 3 columns, 5 rows (buttons - 60px height)
    static lv_coord_t col_dsc[] = {90, 90, 90, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {60, 60, 60, 60, 60, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(button_grid, col_dsc, row_dsc);

    // Button positions: each row has 3 buttons
    // Row 0: 천(1), 지(2), 인(3)
    // Row 1: ㄱ(4), ㄴ(5), ㄷ(6)
    // Row 2: ㅂ(7), ㅅ(8), ㅈ(9)
    // Row 3: 공백(10), ㅇㅁ(0), 삭제(11)
    // Row 4: 모드, 지우기, 엔터
    int positions[12][2] = {
        {1, 3}, // 0: Row 3, Col 1 (ㅇㅁ)
        {0, 0}, {1, 0}, {2, 0}, // 1-3: Row 0 (천, 지, 인)
        {0, 1}, {1, 1}, {2, 1}, // 4-6: Row 1 (ㄱ, ㄴ, ㄷ)
        {0, 2}, {1, 2}, {2, 2}, // 7-9: Row 2 (ㅂ, ㅅ, ㅈ)
        {0, 3}, {2, 3}  // 10-11: Row 3 (Space, Del)
    };

    // Create number buttons (0-11)
    for (int i = 0; i < 12; i++) {
        const wchar_t *wtext = get_button_text(app_widgets.state.now_mode, i);
        char *utf8_text = wchar_to_utf8(wtext, 20);

        app_widgets.buttons[i] = lv_btn_create(button_grid);
        lv_obj_set_grid_cell(app_widgets.buttons[i], LV_GRID_ALIGN_CENTER, positions[i][0], 1,
                            LV_GRID_ALIGN_CENTER, positions[i][1], 1);
        lv_obj_set_size(app_widgets.buttons[i], 80, 55);

        lv_obj_t *label = lv_label_create(app_widgets.buttons[i]);
        lv_label_set_text(label, utf8_text);
        lv_obj_set_style_text_font(label, korean_font_16, 0);
        lv_obj_center(label);

        lv_obj_add_event_cb(app_widgets.buttons[i], on_button_clicked, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }

    // Row 4: Mode, Clear, Enter buttons
    app_widgets.mode_button = lv_btn_create(button_grid);
    lv_obj_set_grid_cell(app_widgets.mode_button, LV_GRID_ALIGN_CENTER, 0, 1,
                        LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_set_size(app_widgets.mode_button, 80, 55);
    lv_obj_set_style_bg_color(app_widgets.mode_button, lv_color_hex(0xFF8C00), 0);  // Orange color
    lv_obj_t *mode_label = lv_label_create(app_widgets.mode_button);
    lv_label_set_text(mode_label, "Mode");
    lv_obj_set_style_text_font(mode_label, korean_font_16, 0);
    lv_obj_center(mode_label);
    lv_obj_add_event_cb(app_widgets.mode_button, on_mode_button_clicked, LV_EVENT_CLICKED, NULL);

    app_widgets.clear_button = lv_btn_create(button_grid);
    lv_obj_set_grid_cell(app_widgets.clear_button, LV_GRID_ALIGN_CENTER, 1, 1,
                        LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_set_size(app_widgets.clear_button, 80, 55);
    lv_obj_t *clear_label = lv_label_create(app_widgets.clear_button);
    lv_label_set_text(clear_label, "Clear");
    lv_obj_set_style_text_font(clear_label, korean_font_16, 0);
    lv_obj_center(clear_label);
    lv_obj_add_event_cb(app_widgets.clear_button, on_clear_clicked, LV_EVENT_CLICKED, NULL);

    app_widgets.enter_button = lv_btn_create(button_grid);
    lv_obj_set_grid_cell(app_widgets.enter_button, LV_GRID_ALIGN_CENTER, 2, 1,
                        LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_set_size(app_widgets.enter_button, 80, 55);
    lv_obj_set_style_bg_color(app_widgets.enter_button, lv_color_hex(0x28A745), 0);  // Green color
    lv_obj_t *enter_label = lv_label_create(app_widgets.enter_button);
    lv_label_set_text(enter_label, "Enter");
    lv_obj_set_style_text_font(enter_label, korean_font_16, 0);
    lv_obj_center(enter_label);
    lv_obj_add_event_cb(app_widgets.enter_button, on_enter_clicked, LV_EVENT_CLICKED, NULL);

    // Create horizontal container for Save and Load buttons
    lv_obj_t *file_btn_cont = lv_obj_create(main_cont);
    lv_obj_set_size(file_btn_cont, 300, 50);
    lv_obj_set_style_bg_opa(file_btn_cont, LV_OPA_0, 0);
    lv_obj_set_style_border_width(file_btn_cont, 0, 0);
    lv_obj_set_style_pad_all(file_btn_cont, 0, 0);
    lv_obj_set_flex_flow(file_btn_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(file_btn_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Save button
    app_widgets.save_button = lv_btn_create(file_btn_cont);
    lv_obj_set_size(app_widgets.save_button, 140, 45);
    lv_obj_set_style_bg_color(app_widgets.save_button, lv_color_hex(0x007BFF), 0);  // Blue color
    lv_obj_set_style_radius(app_widgets.save_button, 8, 0);
    lv_obj_t *save_label = lv_label_create(app_widgets.save_button);
    lv_label_set_text(save_label, "저장하기");
    lv_obj_set_style_text_font(save_label, korean_font_16, 0);
    lv_obj_center(save_label);
    lv_obj_add_event_cb(app_widgets.save_button, on_save_clicked, LV_EVENT_CLICKED, NULL);

    // Load button
    app_widgets.load_button = lv_btn_create(file_btn_cont);
    lv_obj_set_size(app_widgets.load_button, 140, 45);
    lv_obj_set_style_bg_color(app_widgets.load_button, lv_color_hex(0x6C757D), 0);  // Gray color
    lv_obj_set_style_radius(app_widgets.load_button, 8, 0);
    lv_obj_t *load_label = lv_label_create(app_widgets.load_button);
    lv_label_set_text(load_label, "불러오기");
    lv_obj_set_style_text_font(load_label, korean_font_16, 0);
    lv_obj_center(load_label);
    lv_obj_add_event_cb(app_widgets.load_button, on_load_clicked, LV_EVENT_CLICKED, NULL);

    // Info label
    lv_obj_t *info_label = lv_label_create(main_cont);
    lv_label_set_text(info_label, "천지인 한글/영어/숫자/특수키 입력 방식");
    lv_obj_set_style_text_font(info_label, korean_font_12, 0);
}


int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    // Set locale
    setlocale(LC_ALL, "");

    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL2: %s\n", SDL_GetError());
        return 1;
    }

    // Create window
    window = SDL_CreateWindow(
        "Chunjiin Korean Input Method (LVGL 8.4)",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        DISP_HOR_RES,
        DISP_VER_RES,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create texture for framebuffer
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        DISP_HOR_RES,
        DISP_VER_RES
    );

    if (texture == NULL) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("✓ SDL2 initialized\n");

    // Initialize LVGL
    lv_init();

    // Initialize display buffer (double-buffering)
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, BUF_SIZE);

    // Create and register display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = disp_flush;
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;
    lv_disp_drv_register(&disp_drv);

    // Create and register input device driver
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = indev_read;
    indev = lv_indev_drv_register(&indev_drv);

    printf("✓ LVGL 8.4 initialized with SDL2\n");

    // Create UI
    create_ui();

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
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("✓ Application terminated\n");

    return 0;
}
