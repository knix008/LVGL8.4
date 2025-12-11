#include "../include/english.h"
#include "../include/state.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"
#include "../include/input.h"
#include "../include/border.h"
#include "../include/label.h"
#include "../include/ui_helpers.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// ============================================================================
// MOBILE INPUT STATE
// ============================================================================

typedef struct {
    char buffer[256];
    int cursor_pos;
    int last_key;           // Last pressed key (0-11)
    int repeat_count;       // How many times the key was pressed
    uint32_t last_press_time; // Time of last key press
} MobileInputState;

static MobileInputState mobile_state;
static lv_obj_t *text_display;
static lv_obj_t *keyboard_buttons[9];  // 8 letter buttons + 1 space button
static lv_obj_t *keyboard_popup = NULL;
static lv_obj_t *text_input_box = NULL;
static lv_obj_t *mode_label = NULL;
static bool uppercase_mode = false;

// Cursor state
static bool cursor_visible = true;
static lv_timer_t *cursor_timer = NULL;

// Multi-tap timeout (milliseconds)
#define MULTI_TAP_TIMEOUT 1000

// Mobile keypad layout - 8 letter buttons + space
static const char *keypad_lower[9] = {
    "abc",    // 0: abc
    "def",    // 1: def
    "ghi",    // 2: ghi
    "jkl",    // 3: jkl
    "mno",    // 4: mno
    "pqr",    // 5: pqr
    "stu",    // 6: stu
    "vwx",    // 7: vwx
    " "       // 8: space
};

static const char *keypad_upper[9] = {
    "ABC",    // 0
    "DEF",    // 1
    "GHI",    // 2
    "JKL",    // 3
    "MNO",    // 4
    "PQR",    // 5
    "STU",    // 6
    "VWX",    // 7
    " "       // 8
};

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void show_keyboard_popup(void);
static void hide_keyboard_popup(void);
static void update_text_display_with_cursor(void);
static void commit_current_char(void);

// ============================================================================
// CURSOR ANIMATION
// ============================================================================

static void cursor_blink_callback(lv_timer_t *timer) {
    (void)timer;
    cursor_visible = !cursor_visible;
    update_text_display_with_cursor();
}

static void start_cursor_timer(void) {
    if (cursor_timer) {
        lv_timer_del(cursor_timer);
    }
    cursor_visible = true;
    cursor_timer = lv_timer_create(cursor_blink_callback, 500, NULL);
}

static void stop_cursor_timer(void) {
    if (cursor_timer) {
        lv_timer_del(cursor_timer);
        cursor_timer = NULL;
    }
    cursor_visible = true;
}

// ============================================================================
// MOBILE INPUT LOGIC
// ============================================================================

static void mobile_input_init(void) {
    memset(&mobile_state, 0, sizeof(mobile_state));
    mobile_state.last_key = -1;
    uppercase_mode = false;
}

static void commit_current_char(void) {
    if (mobile_state.last_key >= 0 && mobile_state.repeat_count > 0) {
        // Character is already in the buffer, just reset the state
        mobile_state.last_key = -1;
        mobile_state.repeat_count = 0;
    }
}

static void process_key_press(int key_num) {
    uint32_t current_time = lv_tick_get();
    const char *key_chars = uppercase_mode ? keypad_upper[key_num] : keypad_lower[key_num];
    int num_chars = strlen(key_chars);

    if (num_chars == 0) return;

    // Check if this is the same key within timeout
    if (key_num == mobile_state.last_key &&
        (current_time - mobile_state.last_press_time) < MULTI_TAP_TIMEOUT) {
        // Same key pressed again - cycle to next character
        mobile_state.repeat_count++;
        if (mobile_state.repeat_count >= num_chars) {
            mobile_state.repeat_count = 0;
        }

        // Replace the last character
        if (mobile_state.cursor_pos > 0) {
            mobile_state.buffer[mobile_state.cursor_pos - 1] = key_chars[mobile_state.repeat_count];
        }
    } else {
        // Different key or timeout - commit previous and start new
        commit_current_char();

        mobile_state.last_key = key_num;
        mobile_state.repeat_count = 0;
        mobile_state.last_press_time = current_time;

        // Add new character
        if (mobile_state.cursor_pos < (int)sizeof(mobile_state.buffer) - 1) {
            mobile_state.buffer[mobile_state.cursor_pos] = key_chars[0];
            mobile_state.cursor_pos++;
            mobile_state.buffer[mobile_state.cursor_pos] = '\0';
        }
    }
}

static void delete_last_char(void) {
    commit_current_char();

    if (mobile_state.cursor_pos > 0) {
        mobile_state.cursor_pos--;
        mobile_state.buffer[mobile_state.cursor_pos] = '\0';
    }
}

// ============================================================================
// TEXT DISPLAY UPDATE
// ============================================================================

static void update_text_display_with_cursor(void) {
    if (!text_display) return;

    static char display_text[512];

    if (mobile_state.cursor_pos == 0) {
        if (cursor_visible) {
            strcpy(display_text, "|");
        } else {
            display_text[0] = '\0';
        }
    } else {
        if (cursor_visible) {
            snprintf(display_text, sizeof(display_text), "%s|", mobile_state.buffer);
        } else {
            strcpy(display_text, mobile_state.buffer);
        }
    }

    lv_label_set_text(text_display, display_text);
}

// ============================================================================
// EVENT CALLBACKS
// ============================================================================

static void text_input_clicked_callback(lv_event_t *e) {
    (void)e;
    show_keyboard_popup();
}

static void key_btn_callback(lv_event_t *e) {
    int key_num = (int)(intptr_t)lv_event_get_user_data(e);

    process_key_press(key_num);

    update_text_display_with_cursor();

    if (text_input_box) {
        lv_label_set_text(text_input_box, mobile_state.buffer);
    }
}

static void backspace_btn_callback(lv_event_t *e) {
    (void)e;
    delete_last_char();

    update_text_display_with_cursor();

    if (text_input_box) {
        lv_label_set_text(text_input_box, mobile_state.buffer);
    }
}

static void clear_btn_callback(lv_event_t *e) {
    (void)e;
    mobile_input_init();

    update_text_display_with_cursor();

    if (text_input_box) {
        lv_label_set_text(text_input_box, "");
    }
}

static void mode_switch_callback(lv_event_t *e) {
    (void)e;
    commit_current_char();

    // Toggle between uppercase and lowercase
    uppercase_mode = !uppercase_mode;

    if (mode_label) {
        lv_label_set_text(mode_label, uppercase_mode ?
            get_label("english_input_screen.mode_uppercase") :
            get_label("english_input_screen.mode_lowercase"));
    }

    // Update button labels
    for (int i = 0; i < 9; i++) {
        if (keyboard_buttons[i]) {
            lv_obj_t *label = lv_obj_get_child(keyboard_buttons[i], 0);
            if (label) {
                const char *key_text = uppercase_mode ? keypad_upper[i] : keypad_lower[i];
                lv_label_set_text(label, key_text);
            }
        }
    }
}

static void msgbox_event_callback(lv_event_t *e) {
    lv_obj_t *mbox = lv_event_get_current_target(e);
    lv_msgbox_close(mbox);
    remove_green_border();
}

static void close_btn_callback(lv_event_t *e) {
    (void)e;
    hide_keyboard_popup();
}

static void enter_btn_callback(lv_event_t *e) {
    (void)e;

    commit_current_char();

    static char text_copy[512];
    strncpy(text_copy, mobile_state.buffer, sizeof(text_copy) - 1);
    text_copy[sizeof(text_copy) - 1] = '\0';

    if (text_input_box && text_copy[0] != '\0') {
        lv_label_set_text(text_input_box, text_copy);
    }

    mobile_input_init();
    hide_keyboard_popup();

    if (text_copy[0] != '\0') {
        static const char *btns[] = {"OK", ""};
        lv_obj_t *mbox = lv_msgbox_create(NULL, get_label("english_input_screen.result_title"), text_copy, btns, false);

        if (mbox) {
            lv_obj_center(mbox);
            lv_obj_set_width(mbox, 265);

            lv_obj_set_style_bg_color(mbox, lv_color_hex(0x000000), 0);
            lv_obj_set_style_bg_opa(mbox, LV_OPA_50, 0);
            lv_obj_set_style_border_width(mbox, 0, 0);

            if (app_state_get_font_20()) {
                lv_obj_t *title = lv_msgbox_get_title(mbox);
                if (title) {
                    lv_obj_set_style_text_font(title, app_state_get_font_20(), 0);
                    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
                }
                lv_obj_t *text = lv_msgbox_get_text(mbox);
                if (text) {
                    lv_obj_set_style_text_font(text, app_state_get_font_20(), 0);
                    lv_obj_set_style_text_color(text, lv_color_hex(0xFFFFFF), 0);
                }
            }

            lv_obj_t *btns_obj = lv_msgbox_get_btns(mbox);
            if (btns_obj) {
                lv_obj_set_height(btns_obj, 60);
                lv_obj_t *ok_btn = lv_obj_get_child(btns_obj, 0);
                if (ok_btn) {
                    lv_obj_set_style_bg_color(ok_btn, lv_color_hex(0x00FF00), 0);
                    lv_obj_set_size(ok_btn, lv_pct(100), lv_pct(100));
                }
                lv_obj_set_style_bg_opa(btns_obj, LV_OPA_TRANSP, 0);
                lv_obj_set_width(btns_obj, lv_pct(100));
                lv_obj_set_style_text_align(btns_obj, LV_TEXT_ALIGN_CENTER, 0);
            }

            lv_obj_add_event_cb(mbox, msgbox_event_callback, LV_EVENT_VALUE_CHANGED, NULL);
            show_green_border();
        }
    }
}

// ============================================================================
// KEYBOARD POPUP FUNCTIONS
// ============================================================================

static void create_keyboard_popup_content(void) {
    lv_obj_t *scr = lv_scr_act();
    keyboard_popup = lv_obj_create(scr);
    lv_obj_set_size(keyboard_popup, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_pos(keyboard_popup, 0, 0);
    lv_obj_set_style_bg_color(keyboard_popup, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(keyboard_popup, LV_OPA_50, 0);
    lv_obj_set_style_border_width(keyboard_popup, 0, 0);
    lv_obj_clear_flag(keyboard_popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(keyboard_popup);

    lv_obj_t *keyboard_container = lv_obj_create(keyboard_popup);
    lv_obj_set_size(keyboard_container, 260, 460);
    lv_obj_align(keyboard_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(keyboard_container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(keyboard_container, LV_OPA_70, 0);
    lv_obj_set_style_border_color(keyboard_container, lv_color_hex(get_button_border_color()), 0);
    lv_obj_set_style_border_width(keyboard_container, 2, 0);
    lv_obj_clear_flag(keyboard_container, LV_OBJ_FLAG_SCROLLABLE);

    int y_offset = 10;
    int btn_width = 70;
    int btn_height = 50;
    int btn_spacing = 8;
    int grid_width = btn_width * 3 + btn_spacing * 2;

    lv_obj_t *close_btn = create_close_button(keyboard_container, close_btn_callback, NULL);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, -1, 1);

    mode_label = lv_label_create(keyboard_container);
    lv_label_set_text(mode_label, get_label("english_input_screen.mode_lowercase"));
    apply_label_style(mode_label);
    lv_obj_set_style_text_align(mode_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(mode_label, LV_ALIGN_TOP_MID, 0, y_offset);
    y_offset += 30;

    lv_obj_t *text_container = lv_obj_create(keyboard_container);
    lv_obj_set_size(text_container, grid_width, 60);
    lv_obj_align(text_container, LV_ALIGN_TOP_MID, 0, y_offset + 10);
    apply_button_style(text_container, 0);
    lv_obj_set_style_pad_all(text_container, 10, 0);
    lv_obj_clear_flag(text_container, LV_OBJ_FLAG_SCROLLABLE);

    text_display = lv_label_create(text_container);
    lv_label_set_long_mode(text_display, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(text_display, grid_width - 20);
    apply_label_style(text_display);
    lv_label_set_text(text_display, "");
    lv_obj_align(text_display, LV_ALIGN_TOP_LEFT, 0, 0);

    y_offset += 80;

    // Mobile keypad grid (3x3 = 9 buttons)
    lv_obj_t *button_grid = lv_obj_create(keyboard_container);
    lv_obj_set_size(button_grid, grid_width, btn_height * 3 + btn_spacing * 2);
    lv_obj_align(button_grid, LV_ALIGN_TOP_MID, 0, y_offset);
    lv_obj_set_style_bg_opa(button_grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(button_grid, 0, 0);
    lv_obj_set_style_pad_all(button_grid, 0, 0);

    // Create 3x3 grid: ABC, DEF, GHI, JKL, MNO, PQR, STU, VWX, Space
    for (int i = 0; i < 9; i++) {
        int row = i / 3;
        int col = i % 3;

        lv_obj_t *btn = lv_btn_create(button_grid);
        lv_obj_set_size(btn, btn_width, btn_height);
        lv_obj_set_pos(btn, col * (btn_width + btn_spacing), row * (btn_height + btn_spacing));
        apply_button_style(btn, 0);

        lv_obj_t *label = lv_label_create(btn);
        const char *key_text = uppercase_mode ? keypad_upper[i] : keypad_lower[i];
        lv_label_set_text(label, key_text);
        apply_label_style(label);
        lv_obj_center(label);

        lv_obj_add_event_cb(btn, key_btn_callback, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        keyboard_buttons[i] = btn;
    }

    y_offset += btn_height * 3 + btn_spacing * 2 + 10;

    // Control buttons
    int ctrl_btn_width = btn_width;
    int ctrl_btn_height = btn_height;
    int ctrl_btn_spacing = btn_spacing;
    int ctrl_row_width = ctrl_btn_width * 3 + ctrl_btn_spacing * 2;

    lv_obj_t *ctrl_container = lv_obj_create(keyboard_container);
    lv_obj_set_size(ctrl_container, ctrl_row_width, ctrl_btn_height);
    lv_obj_align(ctrl_container, LV_ALIGN_TOP_MID, 0, y_offset);
    lv_obj_set_style_bg_opa(ctrl_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ctrl_container, 0, 0);
    lv_obj_set_style_pad_all(ctrl_container, 0, 0);

    // Mode button
    lv_obj_t *mode_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(mode_btn, ctrl_btn_width, ctrl_btn_height);
    lv_obj_set_pos(mode_btn, 0, 0);
    apply_button_style(mode_btn, 0);

    lv_obj_t *mode_btn_label = lv_label_create(mode_btn);
    lv_label_set_text(mode_btn_label, get_label("english_input_screen.mode_button"));
    apply_label_style(mode_btn_label);
    lv_obj_center(mode_btn_label);

    lv_obj_add_event_cb(mode_btn, mode_switch_callback, LV_EVENT_CLICKED, NULL);

    // Clear button
    lv_obj_t *clear_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(clear_btn, ctrl_btn_width, ctrl_btn_height);
    lv_obj_set_pos(clear_btn, ctrl_btn_width + ctrl_btn_spacing, 0);
    apply_button_style(clear_btn, 0);

    lv_obj_t *clear_btn_label = lv_label_create(clear_btn);
    lv_label_set_text(clear_btn_label, get_label("english_input_screen.clear_button"));
    apply_label_style(clear_btn_label);
    lv_obj_center(clear_btn_label);

    lv_obj_add_event_cb(clear_btn, clear_btn_callback, LV_EVENT_CLICKED, NULL);

    // Delete button
    lv_obj_t *del_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(del_btn, ctrl_btn_width, ctrl_btn_height);
    lv_obj_set_pos(del_btn, (ctrl_btn_width + ctrl_btn_spacing) * 2, 0);
    apply_button_style(del_btn, 0);

    lv_obj_t *del_btn_label = lv_label_create(del_btn);
    lv_label_set_text(del_btn_label, get_label("english_input_screen.delete_button"));
    apply_label_style(del_btn_label);
    lv_obj_center(del_btn_label);

    lv_obj_add_event_cb(del_btn, backspace_btn_callback, LV_EVENT_CLICKED, NULL);

    y_offset += ctrl_btn_height + 10;

    // Enter button (full width)
    lv_obj_t *enter_btn = lv_btn_create(keyboard_container);
    lv_obj_set_size(enter_btn, ctrl_row_width, ctrl_btn_height);
    lv_obj_align(enter_btn, LV_ALIGN_TOP_MID, 0, y_offset);
    apply_button_style(enter_btn, 0);

    lv_obj_t *enter_label = lv_label_create(enter_btn);
    lv_label_set_text(enter_label, get_label("english_input_screen.enter_button"));
    apply_label_style(enter_label);
    lv_obj_center(enter_label);

    lv_obj_add_event_cb(enter_btn, enter_btn_callback, LV_EVENT_CLICKED, NULL);

    update_text_display_with_cursor();
}

static void show_keyboard_popup(void) {
    if (keyboard_popup) {
        lv_obj_del(keyboard_popup);
        keyboard_popup = NULL;
    }
    create_keyboard_popup_content();
    start_cursor_timer();
}

static void hide_keyboard_popup(void) {
    stop_cursor_timer();
    if (keyboard_popup) {
        lv_obj_del(keyboard_popup);
        keyboard_popup = NULL;
    }
}

// ============================================================================
// ENGLISH INPUT SCREEN COMPONENTS
// ============================================================================

static lv_obj_t *create_english_input_content(lv_obj_t *parent) {
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, SCREEN_WIDTH, SCREEN_HEIGHT - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, TITLE_BAR_HEIGHT);
    lv_obj_set_style_bg_color(content, lv_color_hex(get_background_color()), 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title_label = lv_label_create(content);
    lv_label_set_text(title_label, get_label("english_input_screen.title"));
    apply_label_style(title_label);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 50);

    lv_obj_t *text_input_container = lv_obj_create(content);
    lv_obj_set_size(text_input_container, SCREEN_WIDTH - 40, 120);
    lv_obj_align(text_input_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(text_input_container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_color(text_input_container, lv_color_hex(get_button_border_color()), 0);
    lv_obj_set_style_border_width(text_input_container, 3, 0);
    lv_obj_set_style_pad_all(text_input_container, 15, 0);
    lv_obj_add_flag(text_input_container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(text_input_container, LV_OBJ_FLAG_SCROLLABLE);

    text_input_box = lv_label_create(text_input_container);
    lv_label_set_long_mode(text_input_box, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(text_input_box, SCREEN_WIDTH - 70);
    apply_label_style(text_input_box);
    lv_label_set_text(text_input_box, "");
    lv_obj_align(text_input_box, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_add_event_cb(text_input_container, text_input_clicked_callback, LV_EVENT_CLICKED, NULL);

    lv_obj_t *instruction_label = lv_label_create(content);
    lv_label_set_text(instruction_label, get_label("english_input_screen.instruction"));
    apply_label_style(instruction_label);
    lv_obj_set_style_text_align(instruction_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(instruction_label, lv_color_hex(0x888888), 0);
    lv_obj_align(instruction_label, LV_ALIGN_BOTTOM_MID, 0, -100);

    return content;
}

// ============================================================================
// ENGLISH INPUT SCREEN CREATION
// ============================================================================

void create_english_input_screen(void) {
    mobile_input_init();

    lv_obj_t *english_input_screen = create_screen_base(SCREEN_ENGLISH_INPUT);

    create_standard_title_bar(english_input_screen, SCREEN_ENGLISH_INPUT);
    create_english_input_content(english_input_screen);
    create_standard_status_bar(english_input_screen);

    finalize_screen(english_input_screen, SCREEN_ENGLISH_INPUT);
}
