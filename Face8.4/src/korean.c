#include "../include/korean.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"
#include "../include/chunjiin.h"
#include "../include/border.h"
#include <string.h>

// ============================================================================
// GLOBAL STATE
// ============================================================================

static ChunjiinState chunjiin_state;
static lv_obj_t *text_display;
static lv_obj_t *mode_label;
static lv_obj_t *keyboard_buttons[12];

// ============================================================================
// EVENT CALLBACKS
// ============================================================================

static void mode_switch_callback(lv_event_t *e) {
    (void)e;
    change_mode(&chunjiin_state);

    // Update mode label
    const char *mode_text = "";
    switch (chunjiin_state.now_mode) {
        case MODE_HANGUL:
            mode_text = "한글";
            break;
        case MODE_UPPER_ENGLISH:
            mode_text = "영문(대)";
            break;
        case MODE_ENGLISH:
            mode_text = "영문(소)";
            break;
        case MODE_NUMBER:
            mode_text = "숫자";
            break;
        case MODE_SPECIAL:
            mode_text = "특수문자";
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

    // Update text display
    char *utf8_text = wchar_to_utf8(chunjiin_state.text_buffer, MAX_TEXT_LEN);
    lv_label_set_text(text_display, utf8_text);
}

static void clear_btn_callback(lv_event_t *e) {
    (void)e;
    chunjiin_init(&chunjiin_state);
    lv_label_set_text(text_display, "");
}

static void msgbox_event_callback(lv_event_t *e) {
    lv_obj_t *mbox = lv_event_get_current_target(e);
    lv_msgbox_close(mbox);
    
    // Remove the green border rectangle
    remove_green_border();
}

static void enter_btn_callback(lv_event_t *e) {
    (void)e;

    // Get the current text
    char *utf8_text = wchar_to_utf8(chunjiin_state.text_buffer, MAX_TEXT_LEN);

    // Create a message box with OK button and no close icon
    static const char *btns[] = {"OK", ""};
    lv_obj_t *mbox = lv_msgbox_create(NULL, "입력 결과", utf8_text, btns, false);
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

    // Clear the text area
    chunjiin_init(&chunjiin_state);
    lv_label_set_text(text_display, "");
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

    int y_offset = 10;

    // Mode indicator
    mode_label = lv_label_create(content);
    lv_label_set_text(mode_label, "한글");
    apply_label_style(mode_label);
    lv_obj_set_style_text_align(mode_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(mode_label, LV_ALIGN_TOP_MID, 0, y_offset);
    y_offset += 30;

    // Text display area
    lv_obj_t *text_container = lv_obj_create(content);
    lv_obj_set_size(text_container, SCREEN_WIDTH - 45, 100);
    lv_obj_align(text_container, LV_ALIGN_TOP_MID, 0, y_offset);
    lv_obj_set_style_bg_color(text_container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_color(text_container, lv_color_hex(COLOR_BORDER), 0);
    lv_obj_set_style_border_width(text_container, 2, 0);
    lv_obj_set_style_pad_all(text_container, 10, 0);
    lv_obj_add_flag(text_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(text_container, LV_DIR_HOR);

    text_display = lv_label_create(text_container);
    lv_label_set_long_mode(text_display, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(text_display, SCREEN_WIDTH - 70);
    apply_label_style(text_display);
    lv_label_set_text(text_display, "");
    lv_obj_align(text_display, LV_ALIGN_TOP_LEFT, 0, 0);

    y_offset += 110;

    // Keyboard grid - matching Chunjiin8.4 layout
    // Button positions: each row has 3 buttons
    // Row 0: 천(1), 지(2), 인(3)
    // Row 1: ㄱ(4), ㄴ(5), ㄷ(6)
    // Row 2: ㅂ(7), ㅅ(8), ㅈ(9)
    // Row 3: 공백(10), ㅇㅁ(0), 삭제(11)
    // All buttons have same size
    int btn_width = 85;
    int btn_height = 60;
    int btn_spacing = 10;
    int grid_width = btn_width * 3 + btn_spacing * 2;
    int grid_height = btn_height * 4 + btn_spacing * 3;

    // Create a centered container for the button grid
    lv_obj_t *button_grid = lv_obj_create(content);
    lv_obj_set_size(button_grid, grid_width, grid_height);
    lv_obj_align(button_grid, LV_ALIGN_TOP_MID, 0, y_offset);
    lv_obj_set_style_bg_opa(button_grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(button_grid, 0, 0);
    lv_obj_set_style_pad_all(button_grid, 0, 0);

    // Button positions mapping from reference implementation
    // positions[button_num][0] = column, positions[button_num][1] = row
    int positions[12][2] = {
        {1, 3}, // 0: Row 3, Col 1 (ㅇㅁ)
        {0, 0}, {1, 0}, {2, 0}, // 1-3: Row 0 (천, 지, 인)
        {0, 1}, {1, 1}, {2, 1}, // 4-6: Row 1 (ㄱ, ㄴ, ㄷ)
        {0, 2}, {1, 2}, {2, 2}, // 7-9: Row 2 (ㅂ, ㅅ, ㅈ)
        {0, 3}, {2, 3}  // 10-11: Row 3 (Space, Del)
    };

    // Create 12 buttons with correct positions (relative to button_grid)
    for (int i = 0; i < 12; i++) {
        const wchar_t *btn_text = get_button_text(MODE_HANGUL, i);
        char *utf8_text = wchar_to_utf8(btn_text, 16);

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

    // Control buttons row - 3 buttons: 모드, 지우기, Enter
    // Use same size as keyboard buttons
    int ctrl_btn_width = btn_width;
    int ctrl_btn_spacing = btn_spacing;
    int ctrl_btn_height = btn_height;
    int ctrl_row_width = ctrl_btn_width * 3 + ctrl_btn_spacing * 2;

    lv_obj_t *ctrl_container = lv_obj_create(content);
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
    lv_label_set_text(mode_btn_label, "모드");
    apply_label_style(mode_btn_label);
    lv_obj_center(mode_btn_label);

    lv_obj_add_event_cb(mode_btn, mode_switch_callback, LV_EVENT_CLICKED, NULL);

    // Clear button
    lv_obj_t *clear_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(clear_btn, ctrl_btn_width, ctrl_btn_height);
    lv_obj_set_pos(clear_btn, ctrl_btn_width + ctrl_btn_spacing, 0);
    apply_button_style(clear_btn, 0);

    lv_obj_t *clear_btn_label = lv_label_create(clear_btn);
    lv_label_set_text(clear_btn_label, "지우기");
    apply_label_style(clear_btn_label);
    lv_obj_center(clear_btn_label);

    lv_obj_add_event_cb(clear_btn, clear_btn_callback, LV_EVENT_CLICKED, NULL);

    // Enter button
    lv_obj_t *enter_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(enter_btn, ctrl_btn_width, ctrl_btn_height);
    lv_obj_set_pos(enter_btn, (ctrl_btn_width + ctrl_btn_spacing) * 2, 0);
    apply_button_style(enter_btn, 0);

    lv_obj_t *enter_label = lv_label_create(enter_btn);
    lv_label_set_text(enter_label, "Enter");
    apply_label_style(enter_label);
    lv_obj_center(enter_label);

    lv_obj_add_event_cb(enter_btn, enter_btn_callback, LV_EVENT_CLICKED, NULL);

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
