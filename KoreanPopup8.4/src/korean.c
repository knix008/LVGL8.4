#include "../include/korean.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"
#include "../include/chunjiin.h"
#include "../include/border.h"
#include "../include/label.h"
#include <string.h>

// ============================================================================
// GLOBAL STATE
// ============================================================================

static ChunjiinState chunjiin_state;
static lv_obj_t *text_display;
static lv_obj_t *mode_label;
static lv_obj_t *keyboard_buttons[12];
static lv_obj_t *keyboard_popup = NULL;  // The keyboard popup overlay
static lv_obj_t *text_input_box = NULL;  // The text input box on main screen

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void show_keyboard_popup(void);
static void hide_keyboard_popup(void);

// ============================================================================
// EVENT CALLBACKS
// ============================================================================

static void text_input_clicked_callback(lv_event_t *e) {
    (void)e;
    show_keyboard_popup();
}

static void mode_switch_callback(lv_event_t *e) {
    (void)e;
    change_mode(&chunjiin_state);

    // Update mode label
    const char *mode_text = "";
    switch (chunjiin_state.now_mode) {
        case MODE_HANGUL:
            mode_text = get_label("korean_input_screen.modes.korean");
            break;
        case MODE_UPPER_ENGLISH:
            mode_text = "영문(대)";
            break;
        case MODE_ENGLISH:
            mode_text = "영문(소)";
            break;
        case MODE_NUMBER:
            mode_text = get_label("korean_input_screen.modes.number");
            break;
        case MODE_SPECIAL:
            mode_text = get_label("korean_input_screen.modes.special");
            break;
    }
    lv_label_set_text(mode_label, mode_text);

    // Update button labels
    for (int i = 0; i < 12; i++) {
        const wchar_t *btn_text = get_button_text(chunjiin_state.now_mode, i);
        char *utf8_text = wchar_to_utf8(btn_text, 16);
        lv_obj_t *label = lv_obj_get_child(keyboard_buttons[i], 0);
        if (label) {
            lv_label_set_text(label, utf8_text);
        }
    }
}

static void keyboard_btn_callback(lv_event_t *e) {
    int btn_num = (int)(intptr_t)lv_event_get_user_data(e);

    chunjiin_process_input(&chunjiin_state, btn_num);

    // Update text displays (both popup and main input box)
    char *utf8_text = wchar_to_utf8(chunjiin_state.text_buffer, MAX_TEXT_LEN);
    if (text_display) {
        lv_label_set_text(text_display, utf8_text);
    }
    if (text_input_box) {
        lv_label_set_text(text_input_box, utf8_text);
    }
}

static void clear_btn_callback(lv_event_t *e) {
    (void)e;
    chunjiin_init(&chunjiin_state);
    if (text_display) {
        lv_label_set_text(text_display, "");
    }
    if (text_input_box) {
        lv_label_set_text(text_input_box, "");
    }
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

    // Get the current text before clearing
    char *utf8_text = wchar_to_utf8(chunjiin_state.text_buffer, MAX_TEXT_LEN);

    // Make a copy of the text to avoid dangling pointer
    static char text_copy[1024];
    if (utf8_text) {
        strncpy(text_copy, utf8_text, sizeof(text_copy) - 1);
        text_copy[sizeof(text_copy) - 1] = '\0';
    } else {
        text_copy[0] = '\0';
    }

    // Update the parent screen text box with the input
    if (text_input_box && text_copy[0] != '\0') {
        lv_label_set_text(text_input_box, text_copy);
    }

    // Clear the chunjiin state and popup text display
    chunjiin_init(&chunjiin_state);
    if (text_display) {
        lv_label_set_text(text_display, "");
    }

    // Hide the keyboard popup
    hide_keyboard_popup();

    // Only show message box if there's text
    if (text_copy[0] != '\0') {
        // Create a message box with OK button and no close icon
        static const char *btns[] = {"OK", ""};
        lv_obj_t *mbox = lv_msgbox_create(NULL, "입력 결과", text_copy, btns, false);

        if (mbox) {
            lv_obj_center(mbox);

            // Set msgbox width to match button row width (3 buttons + spacing)
            lv_obj_set_width(mbox, 265);

            // Apply black background with 50% transparency and no border to message box
            lv_obj_set_style_bg_color(mbox, lv_color_hex(0x000000), 0);
            lv_obj_set_style_bg_opa(mbox, LV_OPA_50, 0);
            lv_obj_set_style_border_width(mbox, 0, 0);

            // Apply Korean font and white text color to the message box title and content
            extern AppState app_state;
            if (app_state.font_20) {
                // Get the title label
                lv_obj_t *title = lv_msgbox_get_title(mbox);
                if (title) {
                    lv_obj_set_style_text_font(title, app_state.font_20, 0);
                    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
                }
                // Get the text label
                lv_obj_t *text = lv_msgbox_get_text(mbox);
                if (text) {
                    lv_obj_set_style_text_font(text, app_state.font_20, 0);
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
    lv_obj_set_style_bg_color(keyboard_container, lv_color_hex(get_background_color()), 0);
    lv_obj_set_style_border_color(keyboard_container, lv_color_hex(get_button_border_color()), 0);
    lv_obj_set_style_border_width(keyboard_container, 2, 0);
    lv_obj_clear_flag(keyboard_container, LV_OBJ_FLAG_SCROLLABLE);

    int y_offset = 10;

    // Define button sizes first (needed for text container width calculation)
    int btn_width = 70;
    int btn_height = 50;
    int btn_spacing = 8;
    int grid_width = btn_width * 3 + btn_spacing * 2;  // 226px
    int grid_height = btn_height * 4 + btn_spacing * 3;

    // Close button (X) in top-right corner with 5px gap from top and right
    lv_obj_t *close_btn = lv_btn_create(keyboard_container);
    lv_obj_set_size(close_btn, 40, 40);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, -1, 1);
    apply_circle_button_style(close_btn, 0);

    lv_obj_t *close_img = lv_img_create(close_btn);
    lv_img_set_src(close_img, IMG_CANCEL);
    lv_obj_align(close_img, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(close_btn, close_btn_callback, LV_EVENT_CLICKED, NULL);

    // Mode indicator
    mode_label = lv_label_create(keyboard_container);
    lv_label_set_text(mode_label, get_label("korean_input_screen.modes.korean"));
    apply_label_style(mode_label);
    lv_obj_set_style_text_align(mode_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(mode_label, LV_ALIGN_TOP_MID, 0, y_offset);
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

    lv_obj_t *button_grid = lv_obj_create(keyboard_container);
    lv_obj_set_size(button_grid, grid_width, grid_height);
    lv_obj_align(button_grid, LV_ALIGN_TOP_MID, 0, y_offset);
    lv_obj_set_style_bg_opa(button_grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(button_grid, 0, 0);
    lv_obj_set_style_pad_all(button_grid, 0, 0);

    int positions[12][2] = {
        {1, 3}, // 0: Row 3, Col 1 (ㅇㅁ)
        {0, 0}, {1, 0}, {2, 0}, // 1-3: Row 0 (천, 지, 인)
        {0, 1}, {1, 1}, {2, 1}, // 4-6: Row 1 (ㄱ, ㄴ, ㄷ)
        {0, 2}, {1, 2}, {2, 2}, // 7-9: Row 2 (ㅂ, ㅅ, ㅈ)
        {0, 3}, {2, 3}  // 10-11: Row 3 (Space, Del)
    };

    for (int i = 0; i < 12; i++) {
        const wchar_t *btn_text = get_button_text(MODE_HANGUL, i);
        char *utf8_text = wchar_to_utf8(btn_text, 16);

        if (i == 10) {
            utf8_text = (char*)get_label("korean_input_screen.space_button");
        }

        lv_obj_t *btn = lv_btn_create(button_grid);
        lv_obj_set_size(btn, btn_width, btn_height);
        lv_obj_set_pos(btn,
            positions[i][0] * (btn_width + btn_spacing),
            positions[i][1] * (btn_height + btn_spacing));
        apply_button_style(btn, 0);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, utf8_text);
        apply_label_style(label);
        lv_obj_center(label);

        lv_obj_add_event_cb(btn, keyboard_btn_callback, LV_EVENT_CLICKED, (void *)(intptr_t)i);

        keyboard_buttons[i] = btn;
    }

    y_offset += grid_height + 10;

    // Control buttons row
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

    // Mode switch button
    lv_obj_t *mode_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(mode_btn, ctrl_btn_width, ctrl_btn_height);
    lv_obj_set_pos(mode_btn, 0, 0);
    apply_button_style(mode_btn, 0);

    lv_obj_t *mode_btn_label = lv_label_create(mode_btn);
    lv_label_set_text(mode_btn_label, get_label("korean_input_screen.mode_button"));
    apply_label_style(mode_btn_label);
    lv_obj_center(mode_btn_label);

    lv_obj_add_event_cb(mode_btn, mode_switch_callback, LV_EVENT_CLICKED, NULL);

    // Clear button
    lv_obj_t *clear_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(clear_btn, ctrl_btn_width, ctrl_btn_height);
    lv_obj_set_pos(clear_btn, ctrl_btn_width + ctrl_btn_spacing, 0);
    apply_button_style(clear_btn, 0);

    lv_obj_t *clear_btn_label = lv_label_create(clear_btn);
    lv_label_set_text(clear_btn_label, get_label("korean_input_screen.clear_button"));
    apply_label_style(clear_btn_label);
    lv_obj_center(clear_btn_label);

    lv_obj_add_event_cb(clear_btn, clear_btn_callback, LV_EVENT_CLICKED, NULL);

    // Enter button
    lv_obj_t *enter_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(enter_btn, ctrl_btn_width, ctrl_btn_height);
    lv_obj_set_pos(enter_btn, (ctrl_btn_width + ctrl_btn_spacing) * 2, 0);
    apply_button_style(enter_btn, 0);

    lv_obj_t *enter_label = lv_label_create(enter_btn);
    lv_label_set_text(enter_label, get_label("korean_input_screen.enter_button"));
    apply_label_style(enter_label);
    lv_obj_center(enter_label);

    lv_obj_add_event_cb(enter_btn, enter_btn_callback, LV_EVENT_CLICKED, NULL);
}

static void show_keyboard_popup(void) {
    // Delete old popup if it exists
    if (keyboard_popup) {
        lv_obj_del(keyboard_popup);
        keyboard_popup = NULL;
    }
    // Create new popup on current screen
    create_keyboard_popup_content();
}

static void hide_keyboard_popup(void) {
    if (keyboard_popup) {
        lv_obj_del(keyboard_popup);
        keyboard_popup = NULL;
    }
}

// ============================================================================
// KOREAN INPUT SCREEN COMPONENTS
// ============================================================================

static lv_obj_t *create_korean_input_content(lv_obj_t *parent) {
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, SCREEN_WIDTH, SCREEN_HEIGHT - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, TITLE_BAR_HEIGHT);
    lv_obj_set_style_bg_color(content, lv_color_hex(get_background_color()), 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    // Title label
    lv_obj_t *title_label = lv_label_create(content);
    lv_label_set_text(title_label, get_label("korean_input_screen.modes.korean"));
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
    lv_label_set_text(instruction_label, get_label("korean_input_screen.instruction"));
    apply_label_style(instruction_label);
    lv_obj_set_style_text_align(instruction_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(instruction_label, lv_color_hex(0x888888), 0);
    lv_obj_align(instruction_label, LV_ALIGN_BOTTOM_MID, 0, -100);

    return content;
}

// ============================================================================
// KOREAN INPUT SCREEN CREATION
// ============================================================================

void create_korean_input_screen(void) {
    // Initialize Chunjiin state
    chunjiin_init(&chunjiin_state);

    lv_obj_t *korean_input_screen = create_screen_base(SCREEN_KOREAN_INPUT);

    create_standard_title_bar(korean_input_screen, SCREEN_KOREAN_INPUT);
    create_korean_input_content(korean_input_screen);
    create_standard_status_bar(korean_input_screen);

    finalize_screen(korean_input_screen, SCREEN_KOREAN_INPUT);
}
