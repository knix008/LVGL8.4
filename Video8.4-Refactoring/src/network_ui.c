#include "../include/network_ui.h"
#include "../include/network_ip_config.h"
#include "../include/network_input.h"
#include "../include/config.h"
#include "../include/state.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/label.h"
#include "../include/cursor.h"
#include "../include/colors.h"
#include "../include/layout.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// MODULE STATE
// ============================================================================

static lv_obj_t *ip_popup = NULL;
static lv_obj_t *ip_display_label = NULL;
static lv_obj_t *ip_input_display = NULL;
static lv_obj_t *ip_type_switch = NULL;

// Cursor state
static cursor_state_t cursor_state;

// ============================================================================
// PUBLIC API - CURSOR STATE ACCESS
// ============================================================================

cursor_state_t* get_cursor_state(void) {
    return &cursor_state;
}

// ============================================================================
// IP DISPLAY UPDATE
// ============================================================================

void update_ip_display_label(void) {
    if (!ip_display_label) return;

    ip_config_t *config = get_ip_config();
    if (!config) return;

    char display_text[128];
    if (config->type == IP_TYPE_IPV4) {
        snprintf(display_text, sizeof(display_text), "IP (IPv4): %s", config->ipv4);
    } else {
        snprintf(display_text, sizeof(display_text), "IP (IPv6): %s", config->ipv6);
    }
    lv_label_set_text(ip_display_label, display_text);
    lv_obj_invalidate(ip_display_label);
}

void update_popup_ip_display(void) {
    if (!ip_input_display) return;

    ip_config_t *config = get_ip_config();
    input_state_t *input = get_input_state();
    if (!config || !input) return;

    char display_text[64];
    int cursor_pos = get_cursor_position();

    if (config->type == IP_TYPE_IPV4) {
        if (strlen(input->temp_ipv4) > 0) {
            // Insert cursor at cursor_pos
            int len = strlen(input->temp_ipv4);
            if (cursor_pos > len) {
                cursor_pos = len;
                set_cursor_position(cursor_pos);
            }

            // Build display string with cursor
            if (cursor_is_visible(&cursor_state)) {
                strncpy(display_text, input->temp_ipv4, cursor_pos);
                display_text[cursor_pos] = '|';
                strcpy(display_text + cursor_pos + 1, input->temp_ipv4 + cursor_pos);
            } else {
                strcpy(display_text, input->temp_ipv4);
            }
            lv_label_set_text(ip_input_display, display_text);
        } else {
            if (cursor_is_visible(&cursor_state)) {
                lv_label_set_text(ip_input_display, "|");
            } else {
                lv_label_set_text(ip_input_display, "e.g. 192.168.1.100");
            }
        }
    } else {
        if (strlen(input->temp_ipv6) > 0) {
            // Insert cursor at cursor_pos
            int len = strlen(input->temp_ipv6);
            if (cursor_pos > len) {
                cursor_pos = len;
                set_cursor_position(cursor_pos);
            }

            // Build display string with cursor
            if (cursor_is_visible(&cursor_state)) {
                strncpy(display_text, input->temp_ipv6, cursor_pos);
                display_text[cursor_pos] = '|';
                strcpy(display_text + cursor_pos + 1, input->temp_ipv6 + cursor_pos);
            } else {
                strcpy(display_text, input->temp_ipv6);
            }
            lv_label_set_text(ip_input_display, display_text);
        } else {
            if (cursor_is_visible(&cursor_state)) {
                lv_label_set_text(ip_input_display, "|");
            } else {
                lv_label_set_text(ip_input_display, "e.g. 2001:0db8:85a3::7334");
            }
        }
    }
}

// ============================================================================
// IP POPUP FUNCTIONS
// ============================================================================

static void create_ip_popup_content(void) {
    ip_config_t *config = get_ip_config();
    if (!config) return;

    lv_obj_t *scr = lv_scr_act();
    ip_popup = lv_obj_create(scr);
    lv_obj_set_size(ip_popup, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_pos(ip_popup, UI_POS_ORIGIN_X, UI_POS_ORIGIN_Y);
    lv_obj_set_style_bg_color(ip_popup, lv_color_hex(UI_COLOR_BG_POPUP), 0);
    lv_obj_set_style_bg_opa(ip_popup, LV_OPA_50, 0);
    lv_obj_set_style_border_width(ip_popup, 0, 0);
    lv_obj_clear_flag(ip_popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(ip_popup);

    // Create IP input container
    lv_obj_t *ip_container = lv_obj_create(ip_popup);
    lv_obj_set_size(ip_container, UI_POPUP_IP_CONTAINER_WIDTH, UI_POPUP_IP_CONTAINER_HEIGHT);
    lv_obj_align(ip_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(ip_container, lv_color_hex(UI_COLOR_BG_CONTAINER), 0);
    lv_obj_set_style_bg_opa(ip_container, LV_OPA_70, 0);
    lv_obj_set_style_border_color(ip_container, lv_color_hex(get_button_border_color()), 0);
    lv_obj_set_style_border_width(ip_container, 2, 0);
    lv_obj_clear_flag(ip_container, LV_OBJ_FLAG_SCROLLABLE);

    int y_offset = 10;

    // Title
    lv_obj_t *title_label = lv_label_create(ip_container);
    lv_label_set_text(title_label, get_label("network_screen.ip_popup_title"));
    apply_label_style(title_label);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, y_offset);
    y_offset += 30;

    // IPv4/IPv6 toggle buttons
    // IP Type Switch Container
    lv_obj_t *toggle_container = lv_obj_create(ip_container);
    lv_obj_set_size(toggle_container, UI_CONTAINER_TOGGLE_WIDTH, UI_CONTAINER_TOGGLE_HEIGHT);
    lv_obj_align(toggle_container, LV_ALIGN_TOP_MID, 0, y_offset);
    lv_obj_set_style_bg_opa(toggle_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(toggle_container, 0, 0);
    lv_obj_set_style_pad_all(toggle_container, 0, 0);

    // IPv4 Label (left of switch)
    lv_obj_t *ipv4_label = lv_label_create(toggle_container);
    lv_label_set_text(ipv4_label, get_label("network_screen.ipv4_button"));
    apply_label_style(ipv4_label);
    lv_obj_align(ipv4_label, LV_ALIGN_LEFT_MID, 10, 0);

    // Switch
    ip_type_switch = lv_switch_create(toggle_container);
    lv_obj_set_size(ip_type_switch, UI_SWITCH_WIDTH, UI_SWITCH_HEIGHT);
    lv_obj_align(ip_type_switch, LV_ALIGN_CENTER, 0, 0);

    // Style switch colors - green for OFF (IPv4), red for ON (IPv6)
    lv_obj_set_style_bg_color(ip_type_switch, lv_color_hex(UI_COLOR_SWITCH_IPV4), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ip_type_switch, lv_color_hex(UI_COLOR_SWITCH_IPV6), LV_PART_INDICATOR | LV_STATE_CHECKED);

    lv_obj_add_event_cb(ip_type_switch, ip_type_toggle_callback, LV_EVENT_VALUE_CHANGED, NULL);

    // IPv6 Label (right of switch)
    lv_obj_t *ipv6_label = lv_label_create(toggle_container);
    lv_label_set_text(ipv6_label, get_label("network_screen.ipv6_button"));
    apply_label_style(ipv6_label);
    lv_obj_align(ipv6_label, LV_ALIGN_RIGHT_MID, -10, 0);

    // Set initial switch state based on current type
    if (config->type == IP_TYPE_IPV6) {
        lv_obj_add_state(ip_type_switch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(ip_type_switch, LV_STATE_CHECKED);
    }

    y_offset += 50;

    // IP display area
    lv_obj_t *ip_display_container = lv_obj_create(ip_container);
    lv_obj_set_size(ip_display_container, UI_CONTAINER_IP_DISPLAY_WIDTH, UI_CONTAINER_IP_DISPLAY_HEIGHT);
    lv_obj_align(ip_display_container, LV_ALIGN_TOP_MID, 0, y_offset);
    apply_button_style(ip_display_container, 0);
    lv_obj_set_style_pad_all(ip_display_container, 10, 0);
    lv_obj_clear_flag(ip_display_container, LV_OBJ_FLAG_SCROLLABLE);

    ip_input_display = lv_label_create(ip_display_container);
    lv_label_set_long_mode(ip_input_display, LV_LABEL_LONG_DOT);
    lv_obj_set_width(ip_input_display, UI_INPUT_DISPLAY_WIDTH);
    apply_label_style(ip_input_display);
    lv_obj_align(ip_input_display, LV_ALIGN_CENTER, 0, 0);

    y_offset += 70;

    // Keypad configuration
    int btn_size = UI_KEYPAD_BUTTON_SIZE;
    int btn_spacing = UI_KEYPAD_BUTTON_SPACING;

    if (config->type == IP_TYPE_IPV4) {
        // IPv4 Keypad: 0-9, dot, backspace, clear
        int grid_width = btn_size * 3 + btn_spacing * 2;

        lv_obj_t *numpad_container = lv_obj_create(ip_container);
        lv_obj_set_size(numpad_container, grid_width, btn_size * 5 + btn_spacing * 4);
        lv_obj_align(numpad_container, LV_ALIGN_TOP_MID, 0, y_offset);
        lv_obj_set_style_bg_opa(numpad_container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(numpad_container, 0, 0);
        lv_obj_set_style_pad_all(numpad_container, 0, 0);

        // Create number buttons 1-9
        for (int i = 1; i <= 9; i++) {
            int row = (i - 1) / 3;
            int col = (i - 1) % 3;

            lv_obj_t *btn = lv_btn_create(numpad_container);
            lv_obj_set_size(btn, btn_size, btn_size);
            lv_obj_set_pos(btn, col * (btn_size + btn_spacing), row * (btn_size + btn_spacing));
            apply_button_style(btn, 0);

            lv_obj_t *label = lv_label_create(btn);
            char num_str[2] = {i + '0', '\0'};
            lv_label_set_text(label, num_str);
            apply_label_style(label);
            lv_obj_center(label);

            lv_obj_add_event_cb(btn, number_btn_callback, LV_EVENT_CLICKED, (void *)(intptr_t)(i + '0'));
        }

        // Bottom row: Clear, 0, Dot
        lv_obj_t *btn_clear = lv_btn_create(numpad_container);
        lv_obj_set_size(btn_clear, btn_size, btn_size);
        lv_obj_set_pos(btn_clear, 0, 3 * (btn_size + btn_spacing));
        apply_button_style(btn_clear, 0);
        lv_obj_t *label_clear = lv_label_create(btn_clear);
        lv_label_set_text(label_clear, get_label("network_screen.clear_button"));
        apply_label_style(label_clear);
        lv_obj_center(label_clear);
        lv_obj_add_event_cb(btn_clear, clear_all_callback, LV_EVENT_CLICKED, NULL);

        lv_obj_t *btn_0 = lv_btn_create(numpad_container);
        lv_obj_set_size(btn_0, btn_size, btn_size);
        lv_obj_set_pos(btn_0, btn_size + btn_spacing, 3 * (btn_size + btn_spacing));
        apply_button_style(btn_0, 0);
        lv_obj_t *label_0 = lv_label_create(btn_0);
        lv_label_set_text(label_0, "0");
        apply_label_style(label_0);
        lv_obj_center(label_0);
        lv_obj_add_event_cb(btn_0, number_btn_callback, LV_EVENT_CLICKED, (void *)(intptr_t)'0');

        lv_obj_t *btn_dot = lv_btn_create(numpad_container);
        lv_obj_set_size(btn_dot, btn_size, btn_size);
        lv_obj_set_pos(btn_dot, 2 * (btn_size + btn_spacing), 3 * (btn_size + btn_spacing));
        apply_button_style(btn_dot, 0);
        lv_obj_t *label_dot = lv_label_create(btn_dot);
        lv_label_set_text(label_dot, ".");
        apply_label_style(label_dot);
        lv_obj_center(label_dot);
        lv_obj_add_event_cb(btn_dot, dot_colon_callback, LV_EVENT_CLICKED, NULL);

        // Last row: Backspace (full width)
        lv_obj_t *btn_backspace = lv_btn_create(numpad_container);
        lv_obj_set_size(btn_backspace, grid_width, btn_size);
        lv_obj_set_pos(btn_backspace, 0, 4 * (btn_size + btn_spacing));
        apply_button_style(btn_backspace, 0);
        lv_obj_t *label_backspace = lv_label_create(btn_backspace);
        lv_label_set_text(label_backspace, get_label("network_screen.backspace_button"));
        apply_label_style(label_backspace);
        lv_obj_center(label_backspace);
        lv_obj_add_event_cb(btn_backspace, backspace_callback, LV_EVENT_CLICKED, NULL);

        y_offset += btn_size * 5 + btn_spacing * 4 + 10;
    } else {
        // IPv6 Keypad: 0-9, A-F, colon, backspace, clear
        int grid_width = btn_size * 4 + btn_spacing * 3;

        lv_obj_t *numpad_container = lv_obj_create(ip_container);
        lv_obj_set_size(numpad_container, grid_width, btn_size * 5 + btn_spacing * 4);
        lv_obj_align(numpad_container, LV_ALIGN_TOP_MID, 0, y_offset);
        lv_obj_set_style_bg_opa(numpad_container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(numpad_container, 0, 0);
        lv_obj_set_style_pad_all(numpad_container, 0, 0);

        // Row 0-2: Numbers 1-9, A-C (3x4 grid)
        const char *hex_chars[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C"};
        const char hex_values[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c'};

        for (int i = 0; i < 12; i++) {
            int row = i / 4;
            int col = i % 4;

            lv_obj_t *btn = lv_btn_create(numpad_container);
            lv_obj_set_size(btn, btn_size, btn_size);
            lv_obj_set_pos(btn, col * (btn_size + btn_spacing), row * (btn_size + btn_spacing));
            apply_button_style(btn, 0);

            lv_obj_t *label = lv_label_create(btn);
            lv_label_set_text(label, hex_chars[i]);
            apply_label_style(label);
            lv_obj_center(label);

            lv_obj_add_event_cb(btn, number_btn_callback, LV_EVENT_CLICKED, (void *)(intptr_t)hex_values[i]);
        }

        // Row 3: D, E, F, 0
        const char *row3_chars[] = {"D", "E", "F", "0"};
        const char row3_values[] = {'d', 'e', 'f', '0'};

        for (int i = 0; i < 4; i++) {
            lv_obj_t *btn = lv_btn_create(numpad_container);
            lv_obj_set_size(btn, btn_size, btn_size);
            lv_obj_set_pos(btn, i * (btn_size + btn_spacing), 3 * (btn_size + btn_spacing));
            apply_button_style(btn, 0);

            lv_obj_t *label = lv_label_create(btn);
            lv_label_set_text(label, row3_chars[i]);
            apply_label_style(label);
            lv_obj_center(label);

            lv_obj_add_event_cb(btn, number_btn_callback, LV_EVENT_CLICKED, (void *)(intptr_t)row3_values[i]);
        }

        // Row 4: Clear, Colon, Backspace
        int bottom_row_y = 4 * (btn_size + btn_spacing);

        lv_obj_t *btn_clear = lv_btn_create(numpad_container);
        lv_obj_set_size(btn_clear, btn_size, btn_size);
        lv_obj_set_pos(btn_clear, 0, bottom_row_y);
        apply_button_style(btn_clear, 0);
        lv_obj_t *label_clear = lv_label_create(btn_clear);
        lv_label_set_text(label_clear, get_label("network_screen.clear_button"));
        apply_label_style(label_clear);
        lv_obj_center(label_clear);
        lv_obj_add_event_cb(btn_clear, clear_all_callback, LV_EVENT_CLICKED, NULL);

        lv_obj_t *btn_colon = lv_btn_create(numpad_container);
        lv_obj_set_size(btn_colon, btn_size, btn_size);
        lv_obj_set_pos(btn_colon, btn_size + btn_spacing, bottom_row_y);
        apply_button_style(btn_colon, 0);
        lv_obj_t *label_colon = lv_label_create(btn_colon);
        lv_label_set_text(label_colon, ":");
        apply_label_style(label_colon);
        lv_obj_center(label_colon);
        lv_obj_add_event_cb(btn_colon, dot_colon_callback, LV_EVENT_CLICKED, NULL);

        lv_obj_t *btn_backspace = lv_btn_create(numpad_container);
        lv_obj_set_size(btn_backspace, btn_size * 2 + btn_spacing, btn_size);
        lv_obj_set_pos(btn_backspace, 2 * (btn_size + btn_spacing), bottom_row_y);
        apply_button_style(btn_backspace, 0);
        lv_obj_t *label_backspace = lv_label_create(btn_backspace);
        lv_label_set_text(label_backspace, get_label("network_screen.back_button"));
        apply_label_style(label_backspace);
        lv_obj_center(label_backspace);
        lv_obj_add_event_cb(btn_backspace, backspace_callback, LV_EVENT_CLICKED, NULL);

        y_offset += btn_size * 5 + btn_spacing * 4 + 10;
    }

    // Control buttons (Save, Cancel)
    int btn_width = 115;
    int btn_gap = 10;
    int total_width = btn_width * 2 + btn_gap;

    lv_obj_t *ctrl_container = lv_obj_create(ip_container);
    lv_obj_set_size(ctrl_container, total_width, 40);
    lv_obj_align(ctrl_container, LV_ALIGN_TOP_MID, 0, y_offset);
    lv_obj_set_style_bg_opa(ctrl_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ctrl_container, 0, 0);
    lv_obj_set_style_pad_all(ctrl_container, 0, 0);

    lv_obj_t *save_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(save_btn, btn_width, 40);
    lv_obj_set_pos(save_btn, 0, 0);
    apply_button_style(save_btn, 0);
    lv_obj_set_style_bg_color(save_btn, lv_color_hex(UI_COLOR_BTN_SUCCESS), 0);

    lv_obj_t *save_label = lv_label_create(save_btn);
    lv_label_set_text(save_label, get_label("network_screen.save_button"));
    apply_label_style(save_label);
    lv_obj_center(save_label);
    lv_obj_add_event_cb(save_btn, save_ip_callback, LV_EVENT_CLICKED, NULL);

    lv_obj_t *cancel_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(cancel_btn, btn_width, 40);
    lv_obj_set_pos(cancel_btn, btn_width + btn_gap, 0);
    apply_button_style(cancel_btn, 0);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(UI_COLOR_BTN_DANGER), 0);

    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, get_label("network_screen.cancel_button"));
    apply_label_style(cancel_label);
    lv_obj_center(cancel_label);
    lv_obj_add_event_cb(cancel_btn, cancel_btn_callback, LV_EVENT_CLICKED, NULL);

    // Initialize input state with current IP addresses
    init_input_state();

    // Initialize cursor state and start blinking animation
    cursor_state_init(&cursor_state, update_popup_ip_display);
    cursor_start_blinking(&cursor_state);

    // Update display to show current IP address
    update_popup_ip_display();
}

void show_ip_popup(void) {
    if (ip_popup) {
        lv_obj_del(ip_popup);
        ip_popup = NULL;
    }
    create_ip_popup_content();
}

void hide_ip_popup(void) {
    cursor_stop_blinking(&cursor_state);
    if (ip_popup) {
        lv_obj_del(ip_popup);
        ip_popup = NULL;
    }
}

// ============================================================================
// NETWORK SCREEN COMPONENTS
// ============================================================================

static lv_obj_t *create_network_content(lv_obj_t *parent) {
    lv_obj_t *content = create_standard_content(parent);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_AUTO);

    int y_pos = CONTENT_PADDING;

    // IP Address section
    lv_obj_t *ip_section_label = lv_label_create(content);
    lv_label_set_text(ip_section_label, get_label("network_screen.ip_address_title"));
    apply_label_style(ip_section_label);
    if (app_state_get_font_24_bold()) {
        lv_obj_set_style_text_font(ip_section_label, app_state_get_font_24_bold(), 0);
    }
    lv_obj_align(ip_section_label, LV_ALIGN_TOP_LEFT, CONTENT_PADDING, y_pos);
    y_pos += 40;

    // IP display with edit button
    lv_obj_t *ip_display_container = lv_obj_create(content);
    lv_obj_set_size(ip_display_container, SCREEN_WIDTH - CONTENT_WIDTH_LARGE_PADDING, 60);
    lv_obj_align(ip_display_container, LV_ALIGN_TOP_LEFT, CONTENT_PADDING, y_pos);
    apply_button_style(ip_display_container, 0);
    lv_obj_set_style_pad_all(ip_display_container, 10, 0);
    lv_obj_clear_flag(ip_display_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(ip_display_container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ip_display_container, ip_edit_btn_callback, LV_EVENT_CLICKED, NULL);

    ip_display_label = lv_label_create(ip_display_container);
    lv_label_set_long_mode(ip_display_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(ip_display_label, SCREEN_WIDTH - CONTENT_WIDTH_LARGE_PADDING - 20);
    apply_label_style(ip_display_label);
    lv_obj_align(ip_display_label, LV_ALIGN_LEFT_MID, 0, 0);

    y_pos += 80;

    // Info text
    lv_obj_t *info_label = lv_label_create(content);
    lv_label_set_text(info_label, get_label("network_screen.ip_config_instruction"));
    apply_label_style(info_label);
    lv_obj_set_style_text_color(info_label, lv_color_hex(UI_COLOR_TEXT_SECONDARY), 0);
    lv_label_set_long_mode(info_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(info_label, SCREEN_WIDTH - CONTENT_WIDTH_PADDING);
    lv_obj_align(info_label, LV_ALIGN_TOP_LEFT, CONTENT_PADDING, y_pos);
    y_pos += 60;

    // Additional network info (placeholder)
    lv_obj_t *network_label = lv_label_create(content);
    lv_label_set_long_mode(network_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(network_label, SCREEN_WIDTH - CONTENT_WIDTH_PADDING);
    apply_label_style(network_label);
    lv_obj_set_style_pad_all(network_label, CONTENT_PADDING, 0);
    lv_obj_align(network_label, LV_ALIGN_TOP_LEFT, CONTENT_PADDING, y_pos);

    char network_text[512];
    snprintf(network_text, sizeof(network_text),
        "%s\n\n"
        "%s\n"
        "- %s\n"
        "- %s\n\n"
        "%s\n"
        "- %s",
        get_label("network_screen.title"),
        get_label("network_screen.wifi_settings"),
        get_label("network_screen.wifi_ssid"),
        get_label("network_screen.wifi_status"),
        get_label("network_screen.vpn_settings"),
        get_label("network_screen.vpn_status")
    );
    lv_label_set_text(network_label, network_text);

    // Load saved configuration and update display
    load_ip_config();
    update_ip_display_label();

    return content;
}

// ============================================================================
// NETWORK SCREEN CREATION
// ============================================================================

/**
 * Creates the network configuration screen with title bar, content area, and status bar.
 * Uses the standard screen creation pattern.
 */
void create_network_screen(void) {
    lv_obj_t *network_screen = create_screen_base(SCREEN_NETWORK);

    create_standard_title_bar(network_screen, SCREEN_NETWORK);
    create_network_content(network_screen);
    create_standard_status_bar(network_screen);

    finalize_screen(network_screen, SCREEN_NETWORK);
}
