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
static lv_obj_t *keyboard_buttons[10];  // 8 letter buttons + 'yz.' + 1 space button
static lv_obj_t *keyboard_popup = NULL;
static lv_obj_t *text_input_box = NULL;
static lv_obj_t *mode_label = NULL;
static bool uppercase_mode = false;
static char pending_text[512] = "";  // Text waiting for OK confirmation

// Cursor state
static bool cursor_visible = true;
static lv_timer_t *cursor_timer = NULL;

// Multi-tap timeout (milliseconds)
#define MULTI_TAP_TIMEOUT 1000

// Mobile keypad layout - 8 letter buttons + space

// 9 letter buttons + 1 space button
static const char *keypad_lower[10] = {
    "abc",    // 0: abc
    "def",    // 1: def
    "ghi",    // 2: ghi
    "jkl",    // 3: jkl
    "mno",    // 4: mno
    "pqr",    // 5: pqr
    "stu",    // 6: stu
    "vwx",    // 7: vwx
    "yz.",    // 8: yz.
    " "       // 9: space
};

static const char *keypad_upper[10] = {
    "ABC",    // 0
    "DEF",    // 1
    "GHI",    // 2
    "JKL",    // 3
    "MNO",    // 4
    "PQR",    // 5
    "STU",    // 6
    "VWX",    // 7
    "YZ.",    // 8
    " "       // 9
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

// Multi-tap candidate logic
static int candidate_active = 0;
static char candidate_char = 0;

static void commit_current_char(void) {
    if (candidate_active) {
        // Commit candidate_char to buffer
        if (mobile_state.cursor_pos < (int)sizeof(mobile_state.buffer) - 1) {
            mobile_state.buffer[mobile_state.cursor_pos] = candidate_char;
            mobile_state.cursor_pos++;
            mobile_state.buffer[mobile_state.cursor_pos] = '\0';
        }
        candidate_active = 0;
        candidate_char = 0;
    }
    mobile_state.last_key = -1;
    mobile_state.repeat_count = 0;
}

static void process_key_press(int key_num) {
    const char *key_chars = uppercase_mode ? keypad_upper[key_num] : keypad_lower[key_num];
    int num_chars = strlen(key_chars);

    if (num_chars == 0) return;

    // Check if this is the same key (no timer)
    if (key_num == mobile_state.last_key) {
        // Same key pressed again - cycle to next character (candidate only)
        mobile_state.repeat_count++;
        if (mobile_state.repeat_count >= num_chars) {
            mobile_state.repeat_count = 0;
        }
        candidate_char = key_chars[mobile_state.repeat_count];
        candidate_active = 1;
    } else {
        // Different key - commit previous and start new
        commit_current_char();

        mobile_state.last_key = key_num;
        mobile_state.repeat_count = 0;

        // Add new character as candidate
        candidate_char = key_chars[0];
        candidate_active = 1;
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

    int len = mobile_state.cursor_pos;
    strcpy(display_text, mobile_state.buffer);
    if (candidate_active && len < (int)sizeof(display_text) - 2) {
        display_text[len] = candidate_char;
        display_text[len + 1] = '\0';
        len++;
    }

    if (cursor_visible) {
        display_text[len] = '|';
        display_text[len + 1] = '\0';
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

    // Do not update the main screen's text box here - only update on OK confirmation
}

static void backspace_btn_callback(lv_event_t *e) {
    (void)e;
    delete_last_char();

    update_text_display_with_cursor();

    // Do not update the main screen's text box here - only update on OK confirmation
}

static void space_btn_callback(lv_event_t *e) {
    (void)e;
    commit_current_char();
    // Insert a space character at the current cursor position
    if (mobile_state.cursor_pos < (int)sizeof(mobile_state.buffer) - 1) {
        mobile_state.buffer[mobile_state.cursor_pos] = ' ';
        mobile_state.cursor_pos++;
        mobile_state.buffer[mobile_state.cursor_pos] = '\0';
    }
    update_text_display_with_cursor();
    // Do not update the main screen's text box here - only update on OK confirmation
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
    
    // Update the text box when OK is clicked
    if (text_input_box && pending_text[0] != '\0') {
        lv_label_set_text(text_input_box, pending_text);
    }
    
    // Clear pending text
    pending_text[0] = '\0';
    
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

    // Store text for confirmation, but don't update text box yet
    strncpy(pending_text, text_copy, sizeof(pending_text) - 1);
    pending_text[sizeof(pending_text) - 1] = '\0';

    mobile_input_init();
    hide_keyboard_popup();

    if (text_copy[0] != '\0') {
        static const char *btns[] = {"OK", ""};
        lv_obj_t *mbox = lv_msgbox_create(NULL, get_label("english_input_screen.result_title"), text_copy, btns, false);

        if (mbox) {
            setup_msgbox_timer_management(mbox);
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

    // 10 buttons: 3x4 grid (last row has 1 button centered for space)
    lv_obj_t *button_grid = lv_obj_create(keyboard_container);
    int grid_rows = 4;
    lv_obj_set_size(button_grid, grid_width, btn_height * grid_rows + btn_spacing * (grid_rows - 1));
    lv_obj_align(button_grid, LV_ALIGN_TOP_MID, 0, y_offset);
    lv_obj_set_style_bg_opa(button_grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(button_grid, 0, 0);
    lv_obj_set_style_pad_all(button_grid, 0, 0);

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
    // Add the 10th button (space) centered in the last row
    int space_row = 3;
    int space_col = 1; // center column
    int i = 9;
    lv_obj_t *btn = lv_btn_create(button_grid);
    lv_obj_set_size(btn, btn_width, btn_height);
    lv_obj_set_pos(btn, space_col * (btn_width + btn_spacing), space_row * (btn_height + btn_spacing));
    apply_button_style(btn, 0);

    lv_obj_t *label = lv_label_create(btn);
    const char *key_text = uppercase_mode ? keypad_upper[i] : keypad_lower[i];
    lv_label_set_text(label, key_text);
    apply_label_style(label);
    lv_obj_center(label);

    lv_obj_add_event_cb(btn, key_btn_callback, LV_EVENT_CLICKED, (void *)(intptr_t)i);
    keyboard_buttons[i] = btn;

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

    // Space button
    lv_obj_t *space_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(space_btn, ctrl_btn_width, ctrl_btn_height);
    lv_obj_set_pos(space_btn, 0, 0);
    apply_button_style(space_btn, 0);

    lv_obj_t *space_btn_label = lv_label_create(space_btn);
    lv_label_set_text(space_btn_label, get_label("english_input_screen.space_button"));
    apply_label_style(space_btn_label);
    lv_obj_center(space_btn_label);

    lv_obj_add_event_cb(space_btn, space_btn_callback, LV_EVENT_CLICKED, NULL);

    // Mode button
    lv_obj_t *mode_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(mode_btn, ctrl_btn_width, ctrl_btn_height);
    lv_obj_set_pos(mode_btn, ctrl_btn_width + ctrl_btn_spacing, 0);
    apply_button_style(mode_btn, 0);

    lv_obj_t *mode_btn_label = lv_label_create(mode_btn);
    lv_label_set_text(mode_btn_label, get_label("english_input_screen.mode_button"));
    apply_label_style(mode_btn_label);
    lv_obj_center(mode_btn_label);

    lv_obj_add_event_cb(mode_btn, mode_switch_callback, LV_EVENT_CLICKED, NULL);

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
