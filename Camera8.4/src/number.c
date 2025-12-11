/**
 * @file number.c
 * @brief Number input screen implementation
 *
 * Provides a dedicated numeric keypad for number input with
 * large, centered buttons and save/cancel functionality.
 * Follows the same pattern as the Korean input screen.
 */

#include "../include/number.h"
#include "../include/state.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"
#include "../include/border.h"
#include "../include/label.h"
#include "../include/ui_helpers.h"
#include "../include/layout.h"
#include "../include/colors.h"
#include <string.h>
#include <stdio.h>

// ============================================================================
// NUMBER SCREEN LAYOUT CONSTANTS
// ============================================================================

#define NUMBER_SCREEN_CONTAINER_WIDTH       340
#define NUMBER_SCREEN_CONTAINER_HEIGHT      600

#define NUMBER_BUTTON_SIZE                  80
#define NUMBER_BUTTON_SPACING               12

#define NUMBER_TEXT_DISPLAY_WIDTH           300
#define NUMBER_TEXT_DISPLAY_HEIGHT          80

#define NUMBER_CONTROL_BUTTON_WIDTH         140
#define NUMBER_CONTROL_BUTTON_HEIGHT        55

// ============================================================================
// MODULE STATE
// ============================================================================

static char number_buffer[64] = "";
static int cursor_pos = 0;
static lv_obj_t *number_popup = NULL;
static lv_obj_t *number_display = NULL;
static lv_obj_t *text_input_box = NULL;  // Text input box on main screen
static bool cursor_visible = true;
static lv_timer_t *cursor_timer = NULL;

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void create_number_popup_content(void);
static void update_number_display(void);
static void update_text_input_box(void);
static void cursor_blink_callback(lv_timer_t *timer);

// ============================================================================
// DISPLAY UPDATE FUNCTIONS
// ============================================================================

static void cursor_blink_callback(lv_timer_t *timer) {
    (void)timer;
    cursor_visible = !cursor_visible;
    update_number_display();
}

static void update_number_display(void) {
    if (!number_display) return;

    char display_text[128];
    char left_part[64];
    char right_part[64];

    // Split text at cursor position
    strncpy(left_part, number_buffer, cursor_pos);
    left_part[cursor_pos] = '\0';
    strcpy(right_part, number_buffer + cursor_pos);

    // Add cursor
    if (cursor_visible) {
        snprintf(display_text, sizeof(display_text), "%s|%s", left_part, right_part);
    } else {
        snprintf(display_text, sizeof(display_text), "%s %s", left_part, right_part);
    }

    lv_label_set_text(number_display, display_text);
}

static void update_text_input_box(void) {
    if (text_input_box) {
        lv_label_set_text(text_input_box, number_buffer);
    }
}

// ============================================================================
// BUTTON CALLBACKS
// ============================================================================

static void number_btn_callback(lv_event_t *e) {
    int num = (int)(intptr_t)lv_event_get_user_data(e);

    if (cursor_pos < 63) {
        // Insert number at cursor position
        memmove(&number_buffer[cursor_pos + 1], &number_buffer[cursor_pos],
                strlen(number_buffer) - cursor_pos + 1);
        number_buffer[cursor_pos] = '0' + num;
        cursor_pos++;
        update_number_display();
    }
}

static void clear_btn_callback(lv_event_t *e) {
    (void)e;
    memset(number_buffer, 0, sizeof(number_buffer));
    cursor_pos = 0;
    update_number_display();
}

static void backspace_btn_callback(lv_event_t *e) {
    (void)e;
    if (cursor_pos > 0) {
        memmove(&number_buffer[cursor_pos - 1], &number_buffer[cursor_pos],
                strlen(number_buffer) - cursor_pos + 1);
        cursor_pos--;
        update_number_display();
    }
}

static void save_btn_callback(lv_event_t *e) {
    (void)e;
    // Save the number and update text input box
    printf("Number saved: %s\n", number_buffer);
    update_text_input_box();
    close_number_popup();
}

static void cancel_btn_callback(lv_event_t *e) {
    (void)e;
    close_number_popup();
}

static void close_btn_callback(lv_event_t *e) {
    (void)e;
    close_number_popup();
}

static void text_input_clicked_callback(lv_event_t *e) {
    (void)e;
    show_number_popup();
}

// ============================================================================
// POPUP CREATION
// ============================================================================

static void create_number_popup_content(void) {
    lv_obj_t *scr = lv_scr_act();
    number_popup = lv_obj_create(scr);
    lv_obj_set_size(number_popup, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_pos(number_popup, 0, 0);
    lv_obj_set_style_bg_color(number_popup, lv_color_hex(UI_COLOR_BG_POPUP), 0);
    lv_obj_set_style_bg_opa(number_popup, LV_OPA_50, 0);
    lv_obj_set_style_border_width(number_popup, 0, 0);
    lv_obj_clear_flag(number_popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(number_popup);

    // Create number input container
    lv_obj_t *number_container = lv_obj_create(number_popup);
    lv_obj_set_size(number_container, NUMBER_SCREEN_CONTAINER_WIDTH, NUMBER_SCREEN_CONTAINER_HEIGHT);
    lv_obj_align(number_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(number_container, lv_color_hex(UI_COLOR_BG_CONTAINER), 0);
    lv_obj_set_style_bg_opa(number_container, LV_OPA_70, 0);
    lv_obj_set_style_border_color(number_container, lv_color_hex(get_button_border_color()), 0);
    lv_obj_set_style_border_width(number_container, 2, 0);
    lv_obj_set_style_pad_all(number_container, 0, 0);
    lv_obj_clear_flag(number_container, LV_OBJ_FLAG_SCROLLABLE);

    int y_offset = 10;

    // Close button
    lv_obj_t *close_btn = create_close_button(number_container, close_btn_callback, NULL);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, -5, 5);

    // Title
    lv_obj_t *title_label = lv_label_create(number_container);
    lv_label_set_text(title_label, get_label("number_screen.title"));
    apply_label_style(title_label);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, y_offset);
    y_offset += 40;

    // Number display area
    lv_obj_t *display_container = lv_obj_create(number_container);
    lv_obj_set_size(display_container, NUMBER_TEXT_DISPLAY_WIDTH, NUMBER_TEXT_DISPLAY_HEIGHT);
    lv_obj_align(display_container, LV_ALIGN_TOP_MID, 0, y_offset);
    apply_button_style(display_container, 0);
    lv_obj_set_style_pad_all(display_container, 10, 0);
    lv_obj_clear_flag(display_container, LV_OBJ_FLAG_SCROLLABLE);

    number_display = lv_label_create(display_container);
    lv_label_set_long_mode(number_display, LV_LABEL_LONG_DOT);
    lv_obj_set_width(number_display, NUMBER_TEXT_DISPLAY_WIDTH - 20);
    apply_label_style(number_display);
    lv_obj_align(number_display, LV_ALIGN_CENTER, 0, 0);

    y_offset += NUMBER_TEXT_DISPLAY_HEIGHT + 20;

    // Number keypad (3x4 grid: 1-9, Clear/0/Backspace)
    int btn_size = NUMBER_BUTTON_SIZE;
    int btn_spacing = NUMBER_BUTTON_SPACING;
    int grid_width = btn_size * 3 + btn_spacing * 2;
    int grid_height = btn_size * 4 + btn_spacing * 3;

    lv_obj_t *keypad_container = lv_obj_create(number_container);
    lv_obj_set_size(keypad_container, grid_width, grid_height);
    lv_obj_align(keypad_container, LV_ALIGN_TOP_MID, 0, y_offset);
    lv_obj_set_style_bg_opa(keypad_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(keypad_container, 0, 0);
    lv_obj_set_style_pad_all(keypad_container, 0, 0);

    // Create number buttons 1-9
    for (int i = 1; i <= 9; i++) {
        int row = (i - 1) / 3;
        int col = (i - 1) % 3;

        lv_obj_t *btn = lv_btn_create(keypad_container);
        lv_obj_set_size(btn, btn_size, btn_size);
        lv_obj_set_pos(btn, col * (btn_size + btn_spacing), row * (btn_size + btn_spacing));
        apply_button_style(btn, 0);

        lv_obj_t *label = lv_label_create(btn);
        char num_str[2] = {i + '0', '\0'};
        lv_label_set_text(label, num_str);
        apply_label_style(label);
        lv_obj_center(label);

        lv_obj_add_event_cb(btn, number_btn_callback, LV_EVENT_CLICKED, (void *)(intptr_t)i);
    }

    // Bottom row: Clear, 0, Backspace
    lv_obj_t *btn_clear = lv_btn_create(keypad_container);
    lv_obj_set_size(btn_clear, btn_size, btn_size);
    lv_obj_set_pos(btn_clear, 0, 3 * (btn_size + btn_spacing));
    apply_button_style(btn_clear, 0);
    lv_obj_t *label_clear = lv_label_create(btn_clear);
    lv_label_set_text(label_clear, get_label("number_screen.clear_button"));
    apply_label_style(label_clear);
    lv_obj_center(label_clear);
    lv_obj_add_event_cb(btn_clear, clear_btn_callback, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_0 = lv_btn_create(keypad_container);
    lv_obj_set_size(btn_0, btn_size, btn_size);
    lv_obj_set_pos(btn_0, btn_size + btn_spacing, 3 * (btn_size + btn_spacing));
    apply_button_style(btn_0, 0);
    lv_obj_t *label_0 = lv_label_create(btn_0);
    lv_label_set_text(label_0, "0");
    apply_label_style(label_0);
    lv_obj_center(label_0);
    lv_obj_add_event_cb(btn_0, number_btn_callback, LV_EVENT_CLICKED, (void *)(intptr_t)0);

    lv_obj_t *btn_backspace = lv_btn_create(keypad_container);
    lv_obj_set_size(btn_backspace, btn_size, btn_size);
    lv_obj_set_pos(btn_backspace, 2 * (btn_size + btn_spacing), 3 * (btn_size + btn_spacing));
    apply_button_style(btn_backspace, 0);
    lv_obj_t *label_backspace = lv_label_create(btn_backspace);
    lv_label_set_text(label_backspace, get_label("number_screen.backspace_button"));
    apply_label_style(label_backspace);
    lv_obj_center(label_backspace);
    lv_obj_add_event_cb(btn_backspace, backspace_btn_callback, LV_EVENT_CLICKED, NULL);

    y_offset += grid_height + 20;

    // Save and Cancel buttons
    int btn_width = NUMBER_CONTROL_BUTTON_WIDTH;
    int btn_height = NUMBER_CONTROL_BUTTON_HEIGHT;
    int btn_gap = 12;
    int total_width = btn_width * 2 + btn_gap;

    lv_obj_t *ctrl_container = lv_obj_create(number_container);
    lv_obj_set_size(ctrl_container, total_width, btn_height);
    lv_obj_align(ctrl_container, LV_ALIGN_TOP_MID, 0, y_offset);
    lv_obj_set_style_bg_opa(ctrl_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ctrl_container, 0, 0);
    lv_obj_set_style_pad_all(ctrl_container, 0, 0);

    lv_obj_t *save_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(save_btn, btn_width, btn_height);
    lv_obj_set_pos(save_btn, 0, 0);
    apply_button_style(save_btn, 0);
    lv_obj_set_style_bg_color(save_btn, lv_color_hex(UI_COLOR_BTN_SUCCESS), 0);

    lv_obj_t *save_label = lv_label_create(save_btn);
    lv_label_set_text(save_label, get_label("number_screen.save_button"));
    apply_label_style(save_label);
    lv_obj_center(save_label);
    lv_obj_add_event_cb(save_btn, save_btn_callback, LV_EVENT_CLICKED, NULL);

    lv_obj_t *cancel_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(cancel_btn, btn_width, btn_height);
    lv_obj_set_pos(cancel_btn, btn_width + btn_gap, 0);
    apply_button_style(cancel_btn, 0);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(UI_COLOR_BTN_DANGER), 0);

    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, get_label("number_screen.cancel_button"));
    apply_label_style(cancel_label);
    lv_obj_center(cancel_label);
    lv_obj_add_event_cb(cancel_btn, cancel_btn_callback, LV_EVENT_CLICKED, NULL);

    // Start cursor blinking
    update_number_display();
    if (cursor_timer == NULL) {
        cursor_timer = lv_timer_create(cursor_blink_callback, 500, NULL);
    }
}

// ============================================================================
// PUBLIC API
// ============================================================================

void show_number_popup(void) {
    if (number_popup) {
        close_number_popup();
    }
    create_number_popup_content();
}

void close_number_popup(void) {
    if (cursor_timer) {
        lv_timer_del(cursor_timer);
        cursor_timer = NULL;
    }
    if (number_popup) {
        lv_obj_del(number_popup);
        number_popup = NULL;
        number_display = NULL;
    }
}

const char* get_number_input(void) {
    return number_buffer;
}

void set_number_input(const char *value) {
    if (value) {
        strncpy(number_buffer, value, sizeof(number_buffer) - 1);
        number_buffer[sizeof(number_buffer) - 1] = '\0';
        cursor_pos = strlen(number_buffer);
        if (number_display) {
            update_number_display();
        }
        update_text_input_box();
    }
}

void create_number_screen(void) {
    // Initialize number buffer
    memset(number_buffer, 0, sizeof(number_buffer));
    cursor_pos = 0;

    // Create screen with standard components
    lv_obj_t *number_screen = create_screen_base(SCREEN_NUMBER_INPUT);

    create_standard_title_bar(number_screen, SCREEN_NUMBER_INPUT);

    // Create content area
    lv_obj_t *content = lv_obj_create(number_screen);
    lv_obj_set_size(content, SCREEN_WIDTH, SCREEN_HEIGHT - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, TITLE_BAR_HEIGHT);
    lv_obj_set_style_bg_color(content, lv_color_hex(get_background_color()), 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 0, 0);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    // Title label at the top
    lv_obj_t *title_label = lv_label_create(content);
    lv_label_set_text(title_label, get_label("number_screen.title"));
    apply_label_style(title_label);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 50);

    // Text input box - clickable to show number popup (centered)
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

    // Add click event to show number popup
    lv_obj_add_event_cb(text_input_container, text_input_clicked_callback, LV_EVENT_CLICKED, NULL);

    // Instruction label at the bottom
    lv_obj_t *instruction_label = lv_label_create(content);
    lv_label_set_text(instruction_label, get_label("number_screen.instruction"));
    apply_label_style(instruction_label);
    lv_obj_set_style_text_align(instruction_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(instruction_label, lv_color_hex(0x888888), 0);
    lv_obj_align(instruction_label, LV_ALIGN_BOTTOM_MID, 0, -100);

    create_standard_status_bar(number_screen);

    finalize_screen(number_screen, SCREEN_NUMBER_INPUT);
}
