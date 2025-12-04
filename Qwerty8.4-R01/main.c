#include "lvgl/lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include "qwerty.h"

// Application state
typedef struct {
    lv_obj_t *screen;
    lv_obj_t *text_area;
    lv_obj_t *status_label;
    lv_obj_t *shift_buttons[2];  // Left and right shift
    lv_obj_t *caps_button;
    lv_obj_t *lang_button;
    lv_obj_t *clear_button;
    lv_obj_t *enter_button;
    QwertyState qwerty;
    lv_font_t *korean_font_14;  // FreeType font for status
    lv_font_t *korean_font_20;  // FreeType font for text area
    lv_font_t *korean_font_16;  // FreeType font for buttons
    lv_font_t *korean_font_small_20;  // Smaller font for ASCII symbols with better visibility
    lv_font_t *korean_font_20_bold;  // FreeType font for status label (20px bold)
} AppState;

static AppState app_state = {NULL, NULL, NULL, {NULL, NULL}, NULL, NULL, NULL, NULL, {LANG_ENGLISH, 0, 0, {0, 0, 0, 0}}, NULL, NULL, NULL, NULL, NULL};

// Global storage for key buttons to update labels
static lv_obj_t *key_buttons[50];
static KeyMap *key_button_maps[50];
static int num_key_buttons = 0;

// Forward declarations
static void update_status(void);
static void update_button_labels(void);

// Display buffer
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10)

// SDL globals
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

// Input device driver
static lv_indev_drv_t indev_drv;
static lv_indev_t *indev = NULL;

// Global display buffer structure
static lv_disp_draw_buf_t disp_draw_buf;

// Update status label
static void update_status(void) {
    char status_text[128];
    snprintf(status_text, sizeof(status_text),
        "Mode: %s | Shift: %s | Caps: %s",
        app_state.qwerty.current_language == LANG_ENGLISH ? "ENG" : "한국어",
        app_state.qwerty.shift_pressed ? "ON" : "OFF",
        app_state.qwerty.caps_lock ? "ON" : "OFF"
    );
    lv_label_set_text(app_state.status_label, status_text);
}

// Delete last character (handles multi-byte UTF-8 properly)
static void delete_last_char(void) {
    const char *current_text = lv_textarea_get_text(app_state.text_area);
    size_t len = strlen(current_text);
    
    if (len == 0) return;
    
    // Find the start of the last UTF-8 character
    size_t i = len - 1;
    // Skip continuation bytes (10xxxxxx)
    while (i > 0 && (current_text[i] & 0xC0) == 0x80) {
        i--;
    }
    
    // Create a new string without the last character
    char *new_text = malloc(i + 1);
    if (new_text) {
        memcpy(new_text, current_text, i);
        new_text[i] = '\0';
        lv_textarea_set_text(app_state.text_area, new_text);
        free(new_text);
        
        // Move cursor to end
        lv_textarea_set_cursor_pos(app_state.text_area, LV_TEXTAREA_CURSOR_LAST);
    }
}

// Insert text at cursor position
static void insert_text(const char *text) {
    lv_textarea_add_text(app_state.text_area, text);
}

// Button click callback
static void on_key_clicked(lv_event_t *e) {
    KeyMap *key_map = (KeyMap *)lv_event_get_user_data(e);
    const char *text = qwerty_get_key_char(&app_state.qwerty, key_map);

    if (app_state.qwerty.current_language == LANG_KOREAN) {
        char output[21] = {0};  // Enough for 2 Korean syllables
        int delete_prev = 0;
        qwerty_process_korean_char(&app_state.qwerty, text, output, &delete_prev);

        if (delete_prev) {
            delete_last_char();
        }
        insert_text(output);
    } else {
        insert_text(text);
        qwerty_reset_composition(&app_state.qwerty);
    }
}

// Backspace callback
static void on_backspace_clicked(lv_event_t *e) {
    (void)e;
    delete_last_char();
    qwerty_reset_composition(&app_state.qwerty);
}

// Space callback
static void on_space_clicked(lv_event_t *e) {
    (void)e;
    insert_text(" ");
    qwerty_reset_composition(&app_state.qwerty);
}

// Message box button callback - closes the message box
static void on_msgbox_clicked(lv_event_t *e) {
    lv_obj_t *msgbox = lv_event_get_current_target(e);
    lv_msgbox_close(msgbox);
}

// Enter callback - shows popup with text and clears
static void on_enter_clicked(lv_event_t *e) {
    (void)e;

    // Get current text
    const char *text = lv_textarea_get_text(app_state.text_area);
    static const char * btns[] = {"OK", ""};

    // Create the message with the text or empty placeholder
    const char *display_text = (text && strlen(text) > 0) ? text : "(Empty)";

    // Create a message box with title, text, buttons, and add close button
    lv_obj_t *msgbox = lv_msgbox_create(lv_scr_act(), "Input Result", display_text, btns, true);

    // Apply Korean font to the message box text if available
    if (app_state.korean_font_20) {
        lv_obj_t *text_label = lv_msgbox_get_text(msgbox);
        if (text_label) {
            lv_obj_set_style_text_font(text_label, app_state.korean_font_20, 0);
        }
    }

    // Add event handler to close the message box when OK button is clicked
    lv_obj_add_event_cb(msgbox, on_msgbox_clicked, LV_EVENT_VALUE_CHANGED, NULL);

    // Center the message box
    lv_obj_align(msgbox, LV_ALIGN_CENTER, 0, 0);

    // Clear the text area
    lv_textarea_set_text(app_state.text_area, "");
    qwerty_reset_composition(&app_state.qwerty);
}

// Tab callback
static void on_tab_clicked(lv_event_t *e) {
    (void)e;
    insert_text("    ");  // Insert 4 spaces instead of tab character
    qwerty_reset_composition(&app_state.qwerty);
}

// Shift toggle callback
static void on_shift_clicked(lv_event_t *e) {
    (void)e;
    app_state.qwerty.shift_pressed = !app_state.qwerty.shift_pressed;
    update_status();
    update_button_labels();
}

// Caps lock callback
static void on_caps_clicked(lv_event_t *e) {
    (void)e;
    app_state.qwerty.caps_lock = !app_state.qwerty.caps_lock;
    update_status();
    update_button_labels();
}

// Language switch callback
static void on_lang_clicked(lv_event_t *e) {
    (void)e;
    app_state.qwerty.current_language = (app_state.qwerty.current_language == LANG_ENGLISH)
                                  ? LANG_KOREAN : LANG_ENGLISH;
    qwerty_reset_composition(&app_state.qwerty);
    update_status();
    update_button_labels();
}

// Clear text callback
static void on_clear_clicked(lv_event_t *e) {
    (void)e;
    lv_textarea_set_text(app_state.text_area, "");
    qwerty_reset_composition(&app_state.qwerty);
}

// Save text to file callback
static void on_save_clicked(lv_event_t *e) {
    (void)e;

    const char *text = lv_textarea_get_text(app_state.text_area);
    const char *filename = "saved_input.txt";

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        // Show error message
        static const char * btns[] = {"OK", ""};
        lv_obj_t *msgbox = lv_msgbox_create(lv_scr_act(), "Error", "Failed to save file!", btns, true);
        lv_obj_add_event_cb(msgbox, on_msgbox_clicked, LV_EVENT_VALUE_CHANGED, NULL);
        lv_obj_align(msgbox, LV_ALIGN_CENTER, 0, 0);
        return;
    }

    fprintf(file, "%s", text);
    fclose(file);

    // Show success message
    static const char * btns[] = {"OK", ""};
    lv_obj_t *msgbox = lv_msgbox_create(lv_scr_act(), "Success", "File saved successfully!", btns, true);
    lv_obj_add_event_cb(msgbox, on_msgbox_clicked, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_align(msgbox, LV_ALIGN_CENTER, 0, 0);
}

// Restore text from file callback
static void on_restore_clicked(lv_event_t *e) {
    (void)e;

    const char *filename = "saved_input.txt";

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        // Show error message
        static const char * btns[] = {"OK", ""};
        lv_obj_t *msgbox = lv_msgbox_create(lv_scr_act(), "Error", "Failed to open file!", btns, true);
        lv_obj_add_event_cb(msgbox, on_msgbox_clicked, LV_EVENT_VALUE_CHANGED, NULL);
        lv_obj_align(msgbox, LV_ALIGN_CENTER, 0, 0);
        return;
    }

    // Read file content
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        static const char * btns[] = {"OK", ""};
        lv_obj_t *msgbox = lv_msgbox_create(lv_scr_act(), "Error", "Memory allocation failed!", btns, true);
        lv_obj_add_event_cb(msgbox, on_msgbox_clicked, LV_EVENT_VALUE_CHANGED, NULL);
        lv_obj_align(msgbox, LV_ALIGN_CENTER, 0, 0);
        return;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';
    fclose(file);

    // Set text to text area
    lv_textarea_set_text(app_state.text_area, buffer);
    free(buffer);

    qwerty_reset_composition(&app_state.qwerty);

    // Show success message
    static const char * btns[] = {"OK", ""};
    lv_obj_t *msgbox = lv_msgbox_create(lv_scr_act(), "Success", "File restored successfully!", btns, true);
    lv_obj_add_event_cb(msgbox, on_msgbox_clicked, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_align(msgbox, LV_ALIGN_CENTER, 0, 0);
}

// Create a keyboard button
static lv_obj_t* create_key_button(lv_obj_t *parent, const char *label,
                                    lv_event_cb_t callback, void *user_data, int width) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, width, 39);

    // Set button background color - blue
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x4A90E2), 0);
    lv_obj_set_style_radius(btn, 4, 0);

    // Add subtle border
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x2E5C8A), 0);

    lv_obj_t *label_obj = lv_label_create(btn);
    lv_label_set_text(label_obj, label);
    lv_obj_center(label_obj);

    // Check if needs larger font for backtick/tilde visibility
    int needs_larger = (strcmp(label, "`") == 0 || strcmp(label, "~") == 0);

    // Use font from assets - larger size for backtick/tilde
    if (needs_larger && app_state.korean_font_small_20) {
        lv_obj_set_style_text_font(label_obj, app_state.korean_font_small_20, 0);
    } else {
        lv_obj_set_style_text_font(label_obj, app_state.korean_font_16, 0);
    }

    // Text color - white for excellent contrast on blue background
    lv_obj_set_style_text_color(label_obj, lv_color_hex(0xFFFFFF), 0);

    // Force label to use single line and align center
    lv_label_set_long_mode(label_obj, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(label_obj, LV_TEXT_ALIGN_CENTER, 0);

    // Enable button clicking
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_HIDDEN);

    if (callback) {
        lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, user_data);
    }

    return btn;
}

// Update all button labels based on current state
static void update_button_labels(void) {
    for (int i = 0; i < num_key_buttons; i++) {
        const char *label = qwerty_get_key_char(&app_state.qwerty, key_button_maps[i]);
        lv_obj_t *label_obj = lv_obj_get_child(key_buttons[i], 0);
        if (label_obj) {
            lv_label_set_text(label_obj, label);
            
            // Check if needs larger font for visibility
            int needs_larger = (strcmp(label, "`") == 0 || strcmp(label, "~") == 0);
            
            // Ensure style is reapplied after text change
            if (needs_larger && app_state.korean_font_small_20) {
                lv_obj_set_style_text_font(label_obj, app_state.korean_font_small_20, 0);
            } else {
                lv_obj_set_style_text_font(label_obj, app_state.korean_font_16, 0);
            }
            lv_obj_set_style_text_color(label_obj, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_invalidate(label_obj);
        }
    }

    // Update shift button appearance
    for (int i = 0; i < 2; i++) {
        if (app_state.shift_buttons[i]) {
            if (app_state.qwerty.shift_pressed) {
                lv_obj_set_style_bg_color(app_state.shift_buttons[i], 
                                          lv_palette_main(LV_PALETTE_GREEN), 0);
            } else {
                lv_obj_set_style_bg_color(app_state.shift_buttons[i], 
                                          lv_palette_main(LV_PALETTE_GREY), 0);
            }
        }
    }

    // Update caps button appearance
    if (app_state.caps_button) {
        if (app_state.qwerty.caps_lock) {
            lv_obj_set_style_bg_color(app_state.caps_button, 
                                      lv_palette_main(LV_PALETTE_GREEN), 0);
        } else {
            lv_obj_set_style_bg_color(app_state.caps_button, 
                                      lv_palette_main(LV_PALETTE_GREY), 0);
        }
    }

    // Update language button appearance and text
    if (app_state.lang_button) {
        // Set text based on current language (show opposite language)
        lv_obj_t *label_obj = lv_obj_get_child(app_state.lang_button, 0);
        if (label_obj) {
            if (app_state.qwerty.current_language == LANG_ENGLISH) {
                lv_label_set_text(label_obj, "한글");
            } else {
                lv_label_set_text(label_obj, "ENG");
            }
            lv_obj_set_style_text_font(label_obj, app_state.korean_font_16, 0);
            lv_obj_set_style_text_color(label_obj, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_invalidate(label_obj);
        }
        
        // Always set orange color
        lv_obj_set_style_bg_color(app_state.lang_button, 
                                  lv_color_hex(0xFF8C00), 0);  // Orange color
    }
}

// Create the GUI
static void create_gui(void) {
    // Initialize qwerty state
    qwerty_init(&app_state.qwerty);

    // Create main screen
    app_state.screen = lv_scr_act();
    lv_obj_set_style_bg_color(app_state.screen, lv_color_hex(0xF0F0F0), 0);

    // Main container
    lv_obj_t *main_cont = lv_obj_create(app_state.screen);
    lv_obj_set_size(main_cont, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20);
    lv_obj_center(main_cont);
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(main_cont, 10, 0);
    lv_obj_set_style_pad_gap(main_cont, 5, 0);

    // Status label - use font from assets
    app_state.status_label = lv_label_create(main_cont);
    lv_obj_set_style_text_font(app_state.status_label, app_state.korean_font_20_bold, 0);
    update_status();

    // Text area - use font from assets
    app_state.text_area = lv_textarea_create(main_cont);
    lv_obj_set_size(app_state.text_area, SCREEN_WIDTH - 40, 100);
    lv_textarea_set_placeholder_text(app_state.text_area, "Type here...");
    lv_obj_set_style_text_font(app_state.text_area, app_state.korean_font_20, 0);
    // Ensure text is visible
    lv_obj_set_style_text_color(app_state.text_area, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(app_state.text_area, lv_color_hex(0x000000), LV_PART_TEXTAREA_PLACEHOLDER);

    // Keyboard container
    lv_obj_t *keyboard_cont = lv_obj_create(main_cont);
    lv_obj_set_size(keyboard_cont, SCREEN_WIDTH - 40, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(keyboard_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(keyboard_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(keyboard_cont, 5, 0);
    lv_obj_set_style_pad_gap(keyboard_cont, 2, 0);

    // Row 0: Numbers and symbols
    lv_obj_t *row = lv_obj_create(keyboard_cont);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_gap(row, 2, 0);
    lv_obj_set_style_border_width(row, 0, 0);

    for (int i = 0; i < 13; i++) {
        key_buttons[num_key_buttons] = create_key_button(
            row,
            qwerty_get_key_char(&app_state.qwerty, &key_maps[i]),
            on_key_clicked,
            &key_maps[i],
            35  // Reduced from 38
        );
        key_button_maps[num_key_buttons] = &key_maps[i];
        num_key_buttons++;
    }
    create_key_button(row, "←", on_backspace_clicked, NULL, 73);  // Reduced from 76

    // Row 1: QWERTY
    row = lv_obj_create(keyboard_cont);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_gap(row, 2, 0);
    lv_obj_set_style_border_width(row, 0, 0);

    create_key_button(row, "Tab", on_tab_clicked, NULL, 55);  // Reduced from 58

    for (int i = 13; i < 26; i++) {
        key_buttons[num_key_buttons] = create_key_button(
            row,
            qwerty_get_key_char(&app_state.qwerty, &key_maps[i]),
            on_key_clicked,
            &key_maps[i],
            35  // Reduced from 38
        );
        key_button_maps[num_key_buttons] = &key_maps[i];
        num_key_buttons++;
    }

    // Row 2: ASDF
    row = lv_obj_create(keyboard_cont);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_gap(row, 2, 0);
    lv_obj_set_style_border_width(row, 0, 0);

    app_state.caps_button = create_key_button(row, "Caps", on_caps_clicked, NULL, 67);  // Reduced from 70

    for (int i = 26; i < 37; i++) {
        key_buttons[num_key_buttons] = create_key_button(
            row,
            qwerty_get_key_char(&app_state.qwerty, &key_maps[i]),
            on_key_clicked,
            &key_maps[i],
            35  // Reduced from 38
        );
        key_button_maps[num_key_buttons] = &key_maps[i];
        num_key_buttons++;
    }

    // Create Enter button and set it to green color
    app_state.enter_button = create_key_button(row, "⏎", on_enter_clicked, NULL, 61);  // Reduced from 64
    lv_obj_set_style_bg_color(app_state.enter_button, lv_color_hex(0x28A745), 0);  // Green color
    
    // Set Enter button label color to white for better contrast
    lv_obj_t *enter_label = lv_obj_get_child(app_state.enter_button, 0);
    if (enter_label) {
        lv_obj_set_style_text_color(enter_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    // Row 3: ZXCV
    row = lv_obj_create(keyboard_cont);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_gap(row, 2, 0);
    lv_obj_set_style_border_width(row, 0, 0);

    app_state.shift_buttons[0] = create_key_button(row, "Shift", on_shift_clicked, NULL, 81);  // Reduced from 84

    for (int i = 37; i < 47; i++) {
        key_buttons[num_key_buttons] = create_key_button(
            row,
            qwerty_get_key_char(&app_state.qwerty, &key_maps[i]),
            on_key_clicked,
            &key_maps[i],
            35  // Reduced from 38
        );
        key_button_maps[num_key_buttons] = &key_maps[i];
        num_key_buttons++;
    }

    app_state.shift_buttons[1] = create_key_button(row, "Shift", on_shift_clicked, NULL, 81);  // Reduced from 84

    // Row 4: Space bar and controls
    row = lv_obj_create(keyboard_cont);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_gap(row, 2, 0);
    lv_obj_set_style_border_width(row, 0, 0);

    app_state.lang_button = create_key_button(row, "한글", on_lang_clicked, NULL, 58);  // Reduced from 61
    // Set language button to orange color
    lv_obj_set_style_bg_color(app_state.lang_button, lv_color_hex(0xFF8C00), 0);  // Orange color
    
    create_key_button(row, "Space", on_space_clicked, NULL, 343);  // Reduced from 346
    
    // Create Clear button and set it to orange color
    app_state.clear_button = create_key_button(row, "Clear", on_clear_clicked, NULL, 58);  // Reduced from 61
    lv_obj_set_style_bg_color(app_state.clear_button, lv_color_hex(0xFF8C00), 0);  // Orange color

    // Row 5: Save and Restore buttons
    row = lv_obj_create(keyboard_cont);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_gap(row, 2, 0);
    lv_obj_set_style_border_width(row, 0, 0);

    // Create Save button with green color
    lv_obj_t *save_button = create_key_button(row, "Save", on_save_clicked, NULL, 229);
    lv_obj_set_style_bg_color(save_button, lv_color_hex(0x28A745), 0);  // Green color

    // Create Restore button with blue color
    lv_obj_t *restore_button = create_key_button(row, "Restore", on_restore_clicked, NULL, 229);
    lv_obj_set_style_bg_color(restore_button, lv_color_hex(0x007BFF), 0);  // Blue color

    // Initial button state update
    update_button_labels();
}

// Initialize FreeType fonts
static int init_fonts(void) {
    // Initialize FreeType library with default cache settings
    if (!lv_freetype_init(0, 0, 0)) {
        fprintf(stderr, "Error: Failed to initialize FreeType\n");
        return -1;
    }

    // Initialize FreeType fonts with different sizes from assets directory
    // Using NotoSansKR fonts which include ASCII and Korean characters

    // Font for status label (14px light)
    static lv_ft_info_t info_14;
    info_14.name = "assets/NotoSansKR-Light.ttf";
    info_14.weight = 14;
    info_14.style = FT_FONT_STYLE_NORMAL;
    info_14.mem = NULL;
    if (!lv_ft_font_init(&info_14)) {
        fprintf(stderr, "Error: Failed to load NotoSansKR-Light font from assets (14px)\n");
        return -1;
    }
    app_state.korean_font_14 = info_14.font;

    // Font for text area (20px regular)
    static lv_ft_info_t info_20;
    info_20.name = "assets/NotoSansKR-Regular.ttf";
    info_20.weight = 20;
    info_20.style = FT_FONT_STYLE_NORMAL;
    info_20.mem = NULL;
    if (!lv_ft_font_init(&info_20)) {
        fprintf(stderr, "Error: Failed to load NotoSansKR-Regular font from assets (20px)\n");
        return -1;
    }
    app_state.korean_font_20 = info_20.font;

    // Font for keyboard buttons (16px medium)
    static lv_ft_info_t info_16;
    info_16.name = "assets/NotoSansKR-Medium.ttf";
    info_16.weight = 16;
    info_16.style = FT_FONT_STYLE_NORMAL;
    info_16.mem = NULL;
    if (!lv_ft_font_init(&info_16)) {
        fprintf(stderr, "Error: Failed to load NotoSansKR-Medium font from assets (16px)\n");
        return -1;
    }
    app_state.korean_font_16 = info_16.font;

    // Larger font for special characters (20px medium)
    static lv_ft_info_t info_20m;
    info_20m.name = "assets/NotoSansKR-Medium.ttf";
    info_20m.weight = 20;
    info_20m.style = FT_FONT_STYLE_NORMAL;
    info_20m.mem = NULL;
    if (!lv_ft_font_init(&info_20m)) {
        fprintf(stderr, "Warning: Failed to load NotoSansKR-Medium 20px font for special characters\n");
    } else {
        app_state.korean_font_small_20 = info_20m.font;
    }

    // Font for status label (20px semibold)
    static lv_ft_info_t info_20b;
    info_20b.name = "assets/NotoSansKR-SemiBold.ttf";
    info_20b.weight = 20;
    info_20b.style = FT_FONT_STYLE_NORMAL;
    info_20b.mem = NULL;
    if (!lv_ft_font_init(&info_20b)) {
        fprintf(stderr, "Warning: Failed to load NotoSansKR-SemiBold 20px font for status label\n");
    } else {
        app_state.korean_font_20_bold = info_20b.font;
    }

    return 0;
}

// Input device read callback for mouse/pointer
static void indev_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    (void)drv;  // Unused parameter

    // Get current mouse state
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
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    // Create SDL window
    window = SDL_CreateWindow(
        "Qwerty Input with LVGL 8.4",
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

    // Create SDL renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Create texture for framebuffer
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

// Initialize LVGL with SDL
static int init_lvgl(void) {
    // Initialize LVGL
    lv_init();

    // Allocate display buffer (static to persist)
    static lv_color_t buf[BUF_SIZE];

    // Initialize the draw buffer
    lv_disp_draw_buf_init(&disp_draw_buf, buf, NULL, BUF_SIZE);

    // Create display driver for LVGL v8.4
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    // Set display driver parameters
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = display_flush_cb;
    disp_drv.draw_buf = &disp_draw_buf;

    // Register display driver
    lv_disp_drv_register(&disp_drv);

    // Initialize input device driver for mouse/pointer
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = indev_read;
    indev = lv_indev_drv_register(&indev_drv);

    // Initialize FreeType fonts
    if (init_fonts() != 0) {
        fprintf(stderr, "Warning: Font initialization had issues, but continuing...\n");
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

    // Initialize LVGL with SDL
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

        // Update LVGL timing - CRITICAL for input and animation handling!
        uint32_t current_time = SDL_GetTicks();
        uint32_t elapsed = current_time - last_time;
        if (elapsed > 0) {
            lv_tick_inc(elapsed);
            last_time = current_time;
        }

        // Handle LVGL tasks (including input device reading and event processing)
        lv_timer_handler();

        // Small delay to reduce CPU usage and allow other processes
        SDL_Delay(5);
    }

    // Cleanup
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
