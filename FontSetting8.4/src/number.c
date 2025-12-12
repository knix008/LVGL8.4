#include "../include/number.h"
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

// ============================================================================
// MODULE STATE
// ============================================================================

static lv_obj_t *text_display;        // Text display in keyboard popup
static lv_obj_t *keyboard_buttons[12]; // Keyboard button references
static lv_obj_t *keyboard_popup = NULL;  // The keyboard popup overlay
static lv_obj_t *text_input_box = NULL;  // The text input box on main screen
static char number_buffer[32] = "";    // Number input buffer

// Cursor state
static bool cursor_visible = true;
static lv_timer_t *cursor_timer = NULL;

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void show_keyboard_popup(void);
static void hide_keyboard_popup(void);
static void update_text_display_with_cursor(void);

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
// TEXT DISPLAY UPDATE
// ============================================================================

static void update_text_display_with_cursor(void) {
    if (!text_display) return;

    static char display_text[64];

    // Handle empty buffer
    if (number_buffer[0] == '\0') {
        if (cursor_visible) {
            strcpy(display_text, "|");
        } else {
            display_text[0] = '\0';
        }
        lv_label_set_text(text_display, display_text);
        return;
    }

    if (cursor_visible) {
        // Append cursor at end
        snprintf(display_text, sizeof(display_text), "%s|", number_buffer);
    } else {
        strcpy(display_text, number_buffer);
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

static void number_btn_callback(lv_event_t *e) {
    char ch = (char)(intptr_t)lv_event_get_user_data(e);
    size_t len = strlen(number_buffer);

    if (len < sizeof(number_buffer) - 1) {
        number_buffer[len] = ch;
        number_buffer[len + 1] = '\0';

        // Update popup text display with cursor
        update_text_display_with_cursor();
        // Do NOT update main input box here
    }
}

static void backspace_btn_callback(lv_event_t *e) {
    (void)e;
    int len = strlen(number_buffer);

    if (len > 0) {
        number_buffer[len - 1] = '\0';

        // Update popup text display with cursor
        update_text_display_with_cursor();
        // Do NOT update main input box here
    }
}

static void clear_btn_callback(lv_event_t *e) {
    (void)e;
    number_buffer[0] = '\0';

    // Update popup text display with cursor
    update_text_display_with_cursor();
    // Do NOT update main input box here
}

static void msgbox_event_callback(lv_event_t *e) {
    lv_obj_t *mbox = lv_event_get_current_target(e);
    lv_msgbox_close(mbox);

    // Remove the green border rectangle
    remove_green_border();
}

static void close_btn_callback(lv_event_t *e) {
    (void)e;
    hide_keyboard_popup();
}

static void enter_btn_callback(lv_event_t *e) {
    (void)e;

    // Make a copy of the number buffer
    static char text_copy[64];
    strncpy(text_copy, number_buffer, sizeof(text_copy) - 1);
    text_copy[sizeof(text_copy) - 1] = '\0';

    // Update the parent screen text box with the input
    if (text_input_box && text_copy[0] != '\0') {
        lv_label_set_text(text_input_box, text_copy);
    }

    // Clear the buffer
    number_buffer[0] = '\0';

    // Hide the keyboard popup (this will stop cursor timer)
    hide_keyboard_popup();

    // Only show message box if there's text
    if (text_copy[0] != '\0') {
        // Create a message box with OK button and no close icon
        static const char *btns[] = {"OK", ""};
        lv_obj_t *mbox = lv_msgbox_create(NULL, get_label("number_input_screen.result_title"), text_copy, btns, false);

        if (mbox) {
            setup_msgbox_timer_management(mbox);
            lv_obj_center(mbox);

            // Set msgbox width to match button row width (3 buttons + spacing)
            lv_obj_set_width(mbox, 265);

            // Apply black background with 50% transparency and no border to message box
            lv_obj_set_style_bg_color(mbox, lv_color_hex(0x000000), 0);
            lv_obj_set_style_bg_opa(mbox, LV_OPA_50, 0);
            lv_obj_set_style_border_width(mbox, 0, 0);

            // Apply font and white text color to the message box title and content
            if (app_state_get_font_20()) {
                // Get the title label
                lv_obj_t *title = lv_msgbox_get_title(mbox);
                if (title) {
                    lv_obj_set_style_text_font(title, app_state_get_font_20(), 0);
                    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
                }
                // Get the text label
                lv_obj_t *text = lv_msgbox_get_text(mbox);
                if (text) {
                    lv_obj_set_style_text_font(text, app_state_get_font_20(), 0);
                    lv_obj_set_style_text_color(text, lv_color_hex(0xFFFFFF), 0);
                }
            }

            // Style the OK button with green color and center alignment
            lv_obj_t *btns_obj = lv_msgbox_get_btns(mbox);
            if (btns_obj) {
                // Increase the button container height
                lv_obj_set_height(btns_obj, 60);

                // Get the OK button itself and style it
                lv_obj_t *ok_btn = lv_obj_get_child(btns_obj, 0);
                if (ok_btn) {
                    lv_obj_set_style_bg_color(ok_btn, lv_color_hex(0x00FF00), 0);
                    lv_obj_set_size(ok_btn, lv_pct(100), lv_pct(100));  // Fill container
                }

                // Make the button container transparent
                lv_obj_set_style_bg_opa(btns_obj, LV_OPA_TRANSP, 0);

                // Set the button container to match parent width and center its content
                lv_obj_set_width(btns_obj, lv_pct(100));
                lv_obj_set_style_text_align(btns_obj, LV_TEXT_ALIGN_CENTER, 0);
            }

            // Add event callback to close the message box when OK is clicked
            lv_obj_add_event_cb(mbox, msgbox_event_callback, LV_EVENT_VALUE_CHANGED, NULL);

            // Show the green rectangle border
            show_green_border();
        }
    }
}

// ============================================================================
// KEYBOARD POPUP FUNCTIONS
// ============================================================================

static void create_keyboard_popup_content(void) {
    // Create the popup overlay on the active screen
    lv_obj_t *scr = lv_scr_act();
    keyboard_popup = lv_obj_create(scr);
    lv_obj_set_size(keyboard_popup, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_pos(keyboard_popup, 0, 0);
    lv_obj_set_style_bg_color(keyboard_popup, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(keyboard_popup, LV_OPA_50, 0);
    lv_obj_set_style_border_width(keyboard_popup, 0, 0);
    lv_obj_clear_flag(keyboard_popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(keyboard_popup);

    // Create keyboard container
    lv_obj_t *keyboard_container = lv_obj_create(keyboard_popup);
    lv_obj_set_size(keyboard_container, 260, 460);
    lv_obj_align(keyboard_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(keyboard_container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(keyboard_container, LV_OPA_70, 0);
    lv_obj_set_style_border_color(keyboard_container, lv_color_hex(get_button_border_color()), 0);
    lv_obj_set_style_border_width(keyboard_container, 2, 0);
    lv_obj_clear_flag(keyboard_container, LV_OBJ_FLAG_SCROLLABLE);

    int y_offset = 10;

    // Define button sizes first
    int btn_width = 70;
    int btn_height = 50;
    int btn_spacing = 8;
    int grid_width = btn_width * 3 + btn_spacing * 2;  // 226px
    int grid_height = btn_height * 4 + btn_spacing * 3;

    // Close button using helper function
    lv_obj_t *close_btn = create_close_button(keyboard_container, close_btn_callback, NULL);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, -1, 1);

    // Title label
    lv_obj_t *title_label = lv_label_create(keyboard_container);
    lv_label_set_text(title_label, get_label("number_input_screen.title"));
    apply_label_style(title_label);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, y_offset);
    y_offset += 30;

    // Text display area - width matches the three-button grid width
    lv_obj_t *text_container = lv_obj_create(keyboard_container);
    lv_obj_set_size(text_container, grid_width, 60);
    lv_obj_align(text_container, LV_ALIGN_TOP_MID, 0, y_offset + 10);
    apply_button_style(text_container, 0);
    lv_obj_set_style_pad_all(text_container, 10, 0);
    lv_obj_clear_flag(text_container, LV_OBJ_FLAG_SCROLLABLE);

    text_display = lv_label_create(text_container);
    lv_label_set_long_mode(text_display, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(text_display, grid_width - 20);  // Account for padding
    apply_label_style(text_display);
    lv_label_set_text(text_display, "");
    lv_obj_align(text_display, LV_ALIGN_TOP_LEFT, 0, 0);

    y_offset += 80;

    // Number button grid
    lv_obj_t *button_grid = lv_obj_create(keyboard_container);
    lv_obj_set_size(button_grid, grid_width, grid_height);
    lv_obj_align(button_grid, LV_ALIGN_TOP_MID, 0, y_offset);
    lv_obj_set_style_bg_opa(button_grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(button_grid, 0, 0);
    lv_obj_set_style_pad_all(button_grid, 0, 0);

    // Button layout:
    // 1 2 3
    // 4 5 6
    // 7 8 9
    // CLR 0 DEL
    for (int i = 0; i < 12; i++) {
        int row, col;
        char btn_text[8];
        lv_event_cb_t callback;
        void *user_data;

        if (i < 9) {
            // Numbers 1-9
            row = i / 3;
            col = i % 3;
            btn_text[0] = '1' + i;
            btn_text[1] = '\0';
            callback = number_btn_callback;
            user_data = (void *)(intptr_t)('1' + i);
        } else if (i == 9) {
            // CLR button
            row = 3;
            col = 0;
            strcpy(btn_text, "CLR");
            callback = clear_btn_callback;
            user_data = NULL;
        } else if (i == 10) {
            // 0 button
            row = 3;
            col = 1;
            strcpy(btn_text, "0");
            callback = number_btn_callback;
            user_data = (void *)(intptr_t)'0';
        } else {
            // DEL button
            row = 3;
            col = 2;
            strcpy(btn_text, "DEL");
            callback = backspace_btn_callback;
            user_data = NULL;
        }

        lv_obj_t *btn = lv_btn_create(button_grid);
        lv_obj_set_size(btn, btn_width, btn_height);
        lv_obj_set_pos(btn,
            col * (btn_width + btn_spacing),
            row * (btn_height + btn_spacing));
        apply_button_style(btn, 0);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, btn_text);
        apply_label_style(label);
        lv_obj_center(label);

        lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, user_data);

        keyboard_buttons[i] = btn;
    }

    y_offset += grid_height + 10;

    // Control buttons row (only Enter button)
    int ctrl_btn_width = btn_width;
    int ctrl_btn_spacing = btn_spacing;
    int ctrl_btn_height = btn_height;
    int ctrl_row_width = ctrl_btn_width * 3 + ctrl_btn_spacing * 2;

    lv_obj_t *ctrl_container = lv_obj_create(keyboard_container);
    lv_obj_set_size(ctrl_container, ctrl_row_width, ctrl_btn_height);
    lv_obj_align(ctrl_container, LV_ALIGN_TOP_MID, 0, y_offset);
    lv_obj_set_style_bg_opa(ctrl_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ctrl_container, 0, 0);
    lv_obj_set_style_pad_all(ctrl_container, 0, 0);

    // Enter button (centered, spanning full width)
    lv_obj_t *enter_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(enter_btn, ctrl_row_width, ctrl_btn_height);
    lv_obj_set_pos(enter_btn, 0, 0);
    apply_button_style(enter_btn, 0);

    lv_obj_t *enter_label = lv_label_create(enter_btn);
    lv_label_set_text(enter_label, get_label("number_input_screen.enter_button"));
    apply_label_style(enter_label);
    lv_obj_center(enter_label);

    lv_obj_add_event_cb(enter_btn, enter_btn_callback, LV_EVENT_CLICKED, NULL);

    // Initialize text display with cursor
    update_text_display_with_cursor();
}

static void show_keyboard_popup(void) {
    // Delete old popup if it exists
    if (keyboard_popup) {
        lv_obj_del(keyboard_popup);
        keyboard_popup = NULL;
    }
    // Create new popup on current screen
    create_keyboard_popup_content();

    // Start cursor blinking animation
    start_cursor_timer();
}

static void hide_keyboard_popup(void) {
    // Stop cursor timer
    stop_cursor_timer();

    if (keyboard_popup) {
        lv_obj_del(keyboard_popup);
        keyboard_popup = NULL;
    }
}

// ============================================================================
// NUMBER INPUT SCREEN COMPONENTS
// ============================================================================

static lv_obj_t *create_number_input_content(lv_obj_t *parent) {
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, SCREEN_WIDTH, SCREEN_HEIGHT - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, TITLE_BAR_HEIGHT);
    lv_obj_set_style_bg_color(content, lv_color_hex(get_background_color()), 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    // Title label
    lv_obj_t *title_label = lv_label_create(content);
    lv_label_set_text(title_label, get_label("number_input_screen.title"));
    apply_label_style(title_label);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 50);

    // Text input box - clickable to show keyboard
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

    // Add click event to show keyboard popup
    lv_obj_add_event_cb(text_input_container, text_input_clicked_callback, LV_EVENT_CLICKED, NULL);

    // Instruction label
    lv_obj_t *instruction_label = lv_label_create(content);
    lv_label_set_text(instruction_label, get_label("number_input_screen.instruction"));
    apply_label_style(instruction_label);
    lv_obj_set_style_text_align(instruction_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(instruction_label, lv_color_hex(0x888888), 0);
    lv_obj_align(instruction_label, LV_ALIGN_BOTTOM_MID, 0, -100);

    return content;
}

// ============================================================================
// NUMBER INPUT SCREEN CREATION
// ============================================================================

void create_number_input_screen(void) {
    // Initialize number buffer
    number_buffer[0] = '\0';

    lv_obj_t *number_input_screen = create_screen_base(SCREEN_NUMBER_INPUT);

    create_standard_title_bar(number_input_screen, SCREEN_NUMBER_INPUT);
    create_number_input_content(number_input_screen);
    create_standard_status_bar(number_input_screen);

    finalize_screen(number_input_screen, SCREEN_NUMBER_INPUT);
}
