
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"
#include "../include/label.h"
#include "../include/state.h"
#include "../lvgl/lvgl.h"
#include <stdio.h>
#include <string.h>

static lv_obj_t *number_input_label = NULL;
static char temp_number_input[32] = "";


static void number_input_update_display(void) {
    if (number_input_label) {
        if (strlen(temp_number_input) == 0) {
            lv_label_set_text(number_input_label, get_label("camera_screen.person_id_placeholder"));
        } else {
            lv_label_set_text(number_input_label, temp_number_input);
        }
    }
}

static void number_input_number_btn_callback(lv_event_t *e) {
    char ch = (char)(intptr_t)lv_event_get_user_data(e);
    size_t len = strlen(temp_number_input);
    if (len < sizeof(temp_number_input) - 1) {
        temp_number_input[len] = ch;
        temp_number_input[len + 1] = '\0';
        number_input_update_display();
    }
}

static void number_input_backspace_callback(lv_event_t *e) {
    (void)e;
    int len = strlen(temp_number_input);
    if (len > 0) {
        temp_number_input[len - 1] = '\0';
        number_input_update_display();
    }
}

static void number_input_clear_callback(lv_event_t *e) {
    (void)e;
    temp_number_input[0] = '\0';
    number_input_update_display();
}

static void number_input_cancel_callback(lv_event_t *e) {
    (void)e;
    show_screen(SCREEN_MENU);
}

static void number_input_save_callback(lv_event_t *e) {
    (void)e;
    // You can add logic to use temp_number_input here
    show_screen(SCREEN_MENU);
}

// ============================================================================
// NUMBER INPUT SCREEN COMPONENTS
// ============================================================================

#define INPUT_BOX_HEIGHT 60
#define BUTTON_SIZE 80
#define BUTTON_SPACING 10




static lv_obj_t *create_number_input_content(lv_obj_t *parent) {
    lv_obj_t *content = create_standard_content(parent);

    // Calculate full width minus 5px margin on each side
    const int box_width = SCREEN_WIDTH - 10;

    // Input display area (full width minus margin)
    lv_obj_t *input_container = lv_obj_create(content);
    lv_obj_set_size(input_container, box_width, 38);
    lv_obj_align(input_container, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_set_style_bg_color(input_container, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_border_width(input_container, 2, 0);
    lv_obj_set_style_border_color(input_container, lv_color_hex(get_button_border_color()), 0);
    lv_obj_clear_flag(input_container, LV_OBJ_FLAG_SCROLLABLE);

    number_input_label = lv_label_create(input_container);
    lv_label_set_text(number_input_label, get_label("camera_screen.person_id_placeholder"));
    lv_obj_set_style_text_color(number_input_label, lv_color_hex(0xAAAAAA), 0);
    lv_obj_align(number_input_label, LV_ALIGN_LEFT_MID, 10, 0);
    apply_label_style(number_input_label);

    // Number keypad (0-9, CLR, DEL) - container fits buttons exactly and is centered
    const int btn_width = 60;
    const int btn_height = 38;
    const int spacing = 8;
    const int keypad_width = 3 * btn_width + 2 * spacing;
    const int keypad_height = 4 * btn_height + 3 * spacing;

    // Center keypad vertically in available space below input
    int available_height = SCREEN_HEIGHT - (8 + 38 + 20 + 80); // input top margin + input height + est. action area + some margin
    int keypad_y = 8 + 38 + (available_height - keypad_height) / 2;
    lv_obj_t *keypad_container = lv_obj_create(content);
    lv_obj_set_size(keypad_container, keypad_width, keypad_height);
    lv_obj_align(keypad_container, LV_ALIGN_TOP_MID, 0, keypad_y);
    lv_obj_set_style_bg_opa(keypad_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(keypad_container, 0, 0);
    lv_obj_clear_flag(keypad_container, LV_OBJ_FLAG_SCROLLABLE);

    for (int i = 0; i < 12; i++) {
        int row = i / 3;
        int col = i % 3;

        lv_obj_t *btn = lv_btn_create(keypad_container);
        lv_obj_set_size(btn, btn_width, btn_height);
        lv_obj_set_pos(btn, col * (btn_width + spacing), row * (btn_height + spacing));
        apply_button_style(btn, 0);

        lv_obj_t *label = lv_label_create(btn);
        if (i < 9) {
            char num_text[2];
            num_text[0] = '1' + i;
            num_text[1] = '\0';
            lv_label_set_text(label, num_text);
            lv_obj_add_event_cb(btn, number_input_number_btn_callback, LV_EVENT_CLICKED, (void*)(intptr_t)('1' + i));
        } else if (i == 9) {
            lv_label_set_text(label, "CLR");
            lv_obj_add_event_cb(btn, number_input_clear_callback, LV_EVENT_CLICKED, NULL);
        } else if (i == 10) {
            lv_label_set_text(label, "0");
            lv_obj_add_event_cb(btn, number_input_number_btn_callback, LV_EVENT_CLICKED, (void*)(intptr_t)'0');
        } else {
            lv_label_set_text(label, "< DEL");
            lv_obj_add_event_cb(btn, number_input_backspace_callback, LV_EVENT_CLICKED, NULL);
        }
        apply_label_style(label);
        lv_obj_center(label);
    }

    // Save and Cancel buttons - container is taller and centered below keypad
    const int action_btn_width = 100;
    const int action_btn_height = 48;
    const int action_btn_spacing = 24;
    const int action_container_height = action_btn_height + 16;
    int action_y = keypad_y + keypad_height + 20;
    lv_obj_t *action_container = lv_obj_create(content);
    lv_obj_set_size(action_container, keypad_width, action_container_height);
    lv_obj_align(action_container, LV_ALIGN_TOP_MID, 0, action_y);
    lv_obj_set_style_bg_opa(action_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(action_container, 0, 0);
    lv_obj_clear_flag(action_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *save_btn = lv_btn_create(action_container);
    lv_obj_set_size(save_btn, action_btn_width, action_btn_height);
    lv_obj_set_pos(save_btn, 0, 8);
    lv_obj_set_style_bg_color(save_btn, lv_color_hex(0x00AA00), 0);
    lv_obj_add_event_cb(save_btn, number_input_save_callback, LV_EVENT_CLICKED, NULL);

    lv_obj_t *save_label = lv_label_create(save_btn);
    lv_label_set_text(save_label, get_label("camera_screen.person_id_save"));
    apply_label_style(save_label);
    lv_obj_center(save_label);

    lv_obj_t *cancel_btn = lv_btn_create(action_container);
    lv_obj_set_size(cancel_btn, action_btn_width, action_btn_height);
    lv_obj_set_pos(cancel_btn, action_btn_width + action_btn_spacing, 8);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0xAA0000), 0);
    lv_obj_add_event_cb(cancel_btn, number_input_cancel_callback, LV_EVENT_CLICKED, NULL);

    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, get_label("camera_screen.person_id_cancel"));
    apply_label_style(cancel_label);
    lv_obj_center(cancel_label);

    return content;
}

// ============================================================================
// NUMBER INPUT SCREEN CREATION
// ============================================================================

/**
 * Creates the number input screen with title bar, content area, and status bar.
 * Uses the standard screen creation pattern.
 */
void create_number_input_screen(void) {
    temp_number_input[0] = '\0';
    lv_obj_t *number_input_screen = create_screen_base(SCREEN_NUMBER_INPUT);
    create_standard_title_bar(number_input_screen, SCREEN_NUMBER_INPUT);
    create_number_input_content(number_input_screen);
    create_standard_status_bar(number_input_screen);
    finalize_screen(number_input_screen, SCREEN_NUMBER_INPUT);
}
