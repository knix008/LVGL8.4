#include "../include/network.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"
#include "../include/label.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ============================================================================
// MODULE STATE
// ============================================================================

static ip_config_t ip_config = {
    .type = IP_TYPE_IPV4,
    .ipv4 = "192.168.1.100",
    .ipv6 = "2001:0db8:85a3:0000:0000:8a2e:0370:7334"
};

static lv_obj_t *ip_popup = NULL;
static lv_obj_t *ip_display_label = NULL;
static lv_obj_t *ip_input_display = NULL;
static lv_obj_t *ipv4_toggle_btn = NULL;
static lv_obj_t *ipv6_toggle_btn = NULL;

// Temporary input buffers
static char temp_ipv4[16] = "";
static char temp_ipv6[40] = "";

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void show_ip_popup(void);
static void hide_ip_popup(void);
static void update_ip_display_label(void);

// ============================================================================
// IP VALIDATION FUNCTIONS
// ============================================================================

/**
 * Validate IPv4 address format (xxx.xxx.xxx.xxx)
 */
static bool is_valid_ipv4(const char *ip) {
    if (!ip) return false;

    int segments = 0;
    int value = -1;
    bool has_digit = false;

    for (const char *p = ip; *p != '\0'; p++) {
        if (isdigit(*p)) {
            if (value == -1) value = 0;
            value = value * 10 + (*p - '0');
            has_digit = true;
            if (value > 255) return false;
        } else if (*p == '.') {
            if (!has_digit || value < 0) return false;
            segments++;
            value = -1;
            has_digit = false;
        } else {
            return false;
        }
    }

    if (has_digit && value >= 0) segments++;
    return segments == 4;
}

/**
 * Validate IPv6 address format (simplified - checks basic structure)
 */
static bool is_valid_ipv6(const char *ip) {
    if (!ip) return false;

    int segments = 0;
    int hex_digits = 0;
    bool has_double_colon = false;
    const char *p = ip;

    while (*p != '\0') {
        if (isxdigit(*p)) {
            hex_digits++;
            if (hex_digits > 4) return false;
        } else if (*p == ':') {
            if (hex_digits > 0) {
                segments++;
                hex_digits = 0;
            }
            if (*(p + 1) == ':') {
                if (has_double_colon) return false;  // Only one :: allowed
                has_double_colon = true;
                p++;
            }
        } else {
            return false;
        }
        p++;
    }

    if (hex_digits > 0) segments++;

    // IPv6 should have 8 segments, or less if :: is used
    if (has_double_colon) {
        return segments <= 7;
    } else {
        return segments == 8;
    }
}

// ============================================================================
// IP DISPLAY UPDATE
// ============================================================================

static void update_ip_display_label(void) {
    if (!ip_display_label) return;

    char display_text[128];
    if (ip_config.type == IP_TYPE_IPV4) {
        snprintf(display_text, sizeof(display_text), "IP (IPv4): %s", ip_config.ipv4);
    } else {
        snprintf(display_text, sizeof(display_text), "IP (IPv6): %s", ip_config.ipv6);
    }
    lv_label_set_text(ip_display_label, display_text);
    lv_obj_invalidate(ip_display_label);
}

static void update_popup_ip_display(void) {
    if (!ip_input_display) return;

    if (ip_config.type == IP_TYPE_IPV4) {
        if (strlen(temp_ipv4) > 0) {
            lv_label_set_text(ip_input_display, temp_ipv4);
        } else {
            lv_label_set_text(ip_input_display, "e.g. 192.168.1.100");
        }
    } else {
        if (strlen(temp_ipv6) > 0) {
            lv_label_set_text(ip_input_display, temp_ipv6);
        } else {
            lv_label_set_text(ip_input_display, "e.g. 2001:db8::1");
        }
    }
}

// ============================================================================
// EVENT CALLBACKS
// ============================================================================

static void ip_edit_btn_callback(lv_event_t *e) {
    (void)e;
    show_ip_popup();
}

static void ip_type_toggle_callback(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    ip_type_t old_type = ip_config.type;

    if (btn == ipv4_toggle_btn) {
        ip_config.type = IP_TYPE_IPV4;
    } else if (btn == ipv6_toggle_btn) {
        ip_config.type = IP_TYPE_IPV6;
    }

    // Only recreate if type actually changed
    if (old_type != ip_config.type) {
        // Recreate keypad for the new mode
        hide_ip_popup();
        show_ip_popup();
    }
}

static void number_btn_callback(lv_event_t *e) {
    char ch = (char)(intptr_t)lv_event_get_user_data(e);

    if (ip_config.type == IP_TYPE_IPV4) {
        // IPv4 input - add character directly
        size_t len = strlen(temp_ipv4);
        if (len < 15) {  // xxx.xxx.xxx.xxx = 15 chars max
            temp_ipv4[len] = ch;
            temp_ipv4[len + 1] = '\0';
            update_popup_ip_display();
        }
    } else {
        // IPv6 input - add character directly
        size_t len = strlen(temp_ipv6);
        if (len < 39) {  // xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx = 39 chars max
            temp_ipv6[len] = ch;
            temp_ipv6[len + 1] = '\0';
            update_popup_ip_display();
        }
    }
}

static void dot_colon_callback(lv_event_t *e) {
    (void)e;

    if (ip_config.type == IP_TYPE_IPV4) {
        // Add dot for IPv4
        size_t len = strlen(temp_ipv4);
        if (len > 0 && len < 15 && temp_ipv4[len - 1] != '.') {
            temp_ipv4[len] = '.';
            temp_ipv4[len + 1] = '\0';
            update_popup_ip_display();
        }
    } else {
        // Add colon for IPv6
        size_t len = strlen(temp_ipv6);
        if (len > 0 && len < 39 && temp_ipv6[len - 1] != ':') {
            temp_ipv6[len] = ':';
            temp_ipv6[len + 1] = '\0';
            update_popup_ip_display();
        }
    }
}

static void backspace_callback(lv_event_t *e) {
    (void)e;

    if (ip_config.type == IP_TYPE_IPV4) {
        size_t len = strlen(temp_ipv4);
        if (len > 0) {
            temp_ipv4[len - 1] = '\0';
            update_popup_ip_display();
        }
    } else {
        size_t len = strlen(temp_ipv6);
        if (len > 0) {
            temp_ipv6[len - 1] = '\0';
            update_popup_ip_display();
        }
    }
}

static void clear_all_callback(lv_event_t *e) {
    (void)e;

    if (ip_config.type == IP_TYPE_IPV4) {
        temp_ipv4[0] = '\0';
    } else {
        temp_ipv6[0] = '\0';
    }
    update_popup_ip_display();
}

static void cancel_btn_callback(lv_event_t *e) {
    (void)e;
    hide_ip_popup();
}

// Callback to close error message box
static void error_msgbox_event_cb(lv_event_t *e) {
    lv_obj_t *mbox = lv_event_get_current_target(e);
    lv_obj_del(mbox);
}

static void save_ip_callback(lv_event_t *e) {
    (void)e;

    bool valid = false;
    const char *error_msg = NULL;

    if (ip_config.type == IP_TYPE_IPV4) {
        // Validate IPv4
        if (strlen(temp_ipv4) == 0) {
            error_msg = get_label("network_screen.error_empty");
        } else if (!is_valid_ipv4(temp_ipv4)) {
            error_msg = get_label("network_screen.error_invalid_ipv4");
        } else {
            strcpy(ip_config.ipv4, temp_ipv4);
            valid = true;
        }
    } else {
        // Validate IPv6
        if (strlen(temp_ipv6) == 0) {
            error_msg = get_label("network_screen.error_empty");
        } else if (!is_valid_ipv6(temp_ipv6)) {
            error_msg = get_label("network_screen.error_invalid_ipv6");
        } else {
            strcpy(ip_config.ipv6, temp_ipv6);
            valid = true;
        }
    }

    if (valid) {
        save_ip_config();
        update_ip_display_label();
        hide_ip_popup();
    } else {
        // Show error message with localized text
        // Create message box on the active screen (which is the IP popup)
        lv_obj_t *scr = lv_scr_act();

        const char *btns_text[] = {get_label("network_screen.ok_button"), ""};
        static const char *btns[2];
        btns[0] = btns_text[0];
        btns[1] = "";

        lv_obj_t *mbox = lv_msgbox_create(scr, get_label("network_screen.error_title"),
                                          error_msg, btns, false);

        if (mbox) {
            lv_obj_center(mbox);
            lv_obj_move_foreground(mbox);

            // Apply styling
            lv_obj_set_width(mbox, 265);
            lv_obj_set_style_bg_color(mbox, lv_color_hex(0x000000), 0);
            lv_obj_set_style_bg_opa(mbox, LV_OPA_70, 0);
            lv_obj_set_style_border_color(mbox, lv_color_hex(0xFF0000), 0);
            lv_obj_set_style_border_width(mbox, 2, 0);

            // Style the text
            lv_obj_t *text = lv_msgbox_get_text(mbox);
            if (text) {
                lv_obj_set_style_text_color(text, lv_color_hex(0xFFFFFF), 0);
            }

            // Style the title
            lv_obj_t *title = lv_msgbox_get_title(mbox);
            if (title) {
                lv_obj_set_style_text_color(title, lv_color_hex(0xFF6666), 0);
            }

            // Add event callback to close only the message box when OK is clicked
            lv_obj_add_event_cb(mbox, error_msgbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
        }
        // Don't hide IP popup - keep it open so user can correct their input
    }
}

// ============================================================================
// IP POPUP FUNCTIONS
// ============================================================================

static void create_ip_popup_content(void) {
    lv_obj_t *scr = lv_scr_act();
    ip_popup = lv_obj_create(scr);
    lv_obj_set_size(ip_popup, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_pos(ip_popup, 0, 0);
    lv_obj_set_style_bg_color(ip_popup, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(ip_popup, LV_OPA_50, 0);
    lv_obj_set_style_border_width(ip_popup, 0, 0);
    lv_obj_clear_flag(ip_popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(ip_popup);

    // Create IP input container
    lv_obj_t *ip_container = lv_obj_create(ip_popup);
    lv_obj_set_size(ip_container, 280, 520);
    lv_obj_align(ip_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(ip_container, lv_color_hex(0x000000), 0);
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
    lv_obj_t *toggle_container = lv_obj_create(ip_container);
    lv_obj_set_size(toggle_container, 240, 40);
    lv_obj_align(toggle_container, LV_ALIGN_TOP_MID, 0, y_offset);
    lv_obj_set_style_bg_opa(toggle_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(toggle_container, 0, 0);
    lv_obj_set_style_pad_all(toggle_container, 0, 0);

    ipv4_toggle_btn = lv_btn_create(toggle_container);
    lv_obj_set_size(ipv4_toggle_btn, 115, 40);
    lv_obj_set_pos(ipv4_toggle_btn, 0, 0);
    apply_button_style(ipv4_toggle_btn, 0);

    lv_obj_t *ipv4_label = lv_label_create(ipv4_toggle_btn);
    lv_label_set_text(ipv4_label, get_label("network_screen.ipv4_button"));
    apply_label_style(ipv4_label);
    lv_obj_center(ipv4_label);
    lv_obj_add_event_cb(ipv4_toggle_btn, ip_type_toggle_callback, LV_EVENT_CLICKED, NULL);

    ipv6_toggle_btn = lv_btn_create(toggle_container);
    lv_obj_set_size(ipv6_toggle_btn, 115, 40);
    lv_obj_set_pos(ipv6_toggle_btn, 125, 0);
    apply_button_style(ipv6_toggle_btn, 0);

    lv_obj_t *ipv6_label = lv_label_create(ipv6_toggle_btn);
    lv_label_set_text(ipv6_label, get_label("network_screen.ipv6_button"));
    apply_label_style(ipv6_label);
    lv_obj_center(ipv6_label);
    lv_obj_add_event_cb(ipv6_toggle_btn, ip_type_toggle_callback, LV_EVENT_CLICKED, NULL);

    // Set initial button colors based on current type
    if (ip_config.type == IP_TYPE_IPV4) {
        lv_obj_set_style_bg_color(ipv4_toggle_btn, lv_color_hex(0x00AA00), 0);
        lv_obj_set_style_bg_color(ipv6_toggle_btn, lv_color_hex(get_button_color()), 0);
    } else {
        lv_obj_set_style_bg_color(ipv4_toggle_btn, lv_color_hex(get_button_color()), 0);
        lv_obj_set_style_bg_color(ipv6_toggle_btn, lv_color_hex(0x00AA00), 0);
    }

    y_offset += 50;

    // IP display area
    lv_obj_t *ip_display_container = lv_obj_create(ip_container);
    lv_obj_set_size(ip_display_container, 260, 60);
    lv_obj_align(ip_display_container, LV_ALIGN_TOP_MID, 0, y_offset);
    apply_button_style(ip_display_container, 0);
    lv_obj_set_style_pad_all(ip_display_container, 10, 0);
    lv_obj_clear_flag(ip_display_container, LV_OBJ_FLAG_SCROLLABLE);

    ip_input_display = lv_label_create(ip_display_container);
    lv_label_set_long_mode(ip_input_display, LV_LABEL_LONG_DOT);
    lv_obj_set_width(ip_input_display, 240);
    apply_label_style(ip_input_display);
    lv_obj_align(ip_input_display, LV_ALIGN_CENTER, 0, 0);

    y_offset += 70;

    // Keypad configuration
    int btn_size = 50;
    int btn_spacing = 8;

    if (ip_config.type == IP_TYPE_IPV4) {
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

        y_offset += btn_size * 5 + btn_spacing * 4 + 20;
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

        y_offset += btn_size * 5 + btn_spacing * 4 + 20;
    }

    // Control buttons (Save, Cancel)
    lv_obj_t *ctrl_container = lv_obj_create(ip_container);
    lv_obj_set_size(ctrl_container, 240, 40);
    lv_obj_align(ctrl_container, LV_ALIGN_TOP_MID, 0, y_offset);
    lv_obj_set_style_bg_opa(ctrl_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ctrl_container, 0, 0);
    lv_obj_set_style_pad_all(ctrl_container, 0, 0);

    lv_obj_t *save_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(save_btn, 115, 40);
    lv_obj_set_pos(save_btn, 0, 0);
    apply_button_style(save_btn, 0);
    lv_obj_set_style_bg_color(save_btn, lv_color_hex(0x00AA00), 0);

    lv_obj_t *save_label = lv_label_create(save_btn);
    lv_label_set_text(save_label, get_label("network_screen.save_button"));
    apply_label_style(save_label);
    lv_obj_center(save_label);
    lv_obj_add_event_cb(save_btn, save_ip_callback, LV_EVENT_CLICKED, NULL);

    lv_obj_t *cancel_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(cancel_btn, 115, 40);
    lv_obj_set_pos(cancel_btn, 125, 0);
    apply_button_style(cancel_btn, 0);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0xAA0000), 0);

    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, get_label("network_screen.cancel_button"));
    apply_label_style(cancel_label);
    lv_obj_center(cancel_label);
    lv_obj_add_event_cb(cancel_btn, cancel_btn_callback, LV_EVENT_CLICKED, NULL);

    // Initialize display
    strcpy(temp_ipv4, ip_config.ipv4);
    strcpy(temp_ipv6, ip_config.ipv6);

    update_popup_ip_display();
}

static void show_ip_popup(void) {
    if (ip_popup) {
        lv_obj_del(ip_popup);
        ip_popup = NULL;
    }
    create_ip_popup_content();
}

static void hide_ip_popup(void) {
    if (ip_popup) {
        lv_obj_del(ip_popup);
        ip_popup = NULL;
    }
}

// ============================================================================
// CONFIGURATION PERSISTENCE
// ============================================================================

// Helper function to extract a JSON section
static int extract_json_section(const char* json, const char* section_name, char* output, size_t output_size) {
    if (!json || !section_name || !output) return -1;

    char search[256];
    snprintf(search, sizeof(search), "\"%s\"", section_name);

    const char* section_start = strstr(json, search);
    if (!section_start) return -1;

    const char* brace = strchr(section_start, '{');
    if (!brace) return -1;

    int depth = 1;
    const char* p = brace + 1;
    while (*p && depth > 0) {
        if (*p == '{') depth++;
        else if (*p == '}') depth--;
        p++;
    }

    if (depth == 0) {
        size_t len = p - section_start;
        if (len < output_size - 1) {
            strncpy(output, section_start, len);
            output[len] = '\0';
            return 0;
        }
    }

    return -1;
}

// Helper function to read entire file
static char* read_config_file(void) {
    FILE* fp = fopen(STATUS_BAR_CONFIG_FILE, "r");
    if (!fp) return NULL;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size > MAX_CONFIG_JSON_SIZE - 1) {
        fclose(fp);
        return NULL;
    }

    static char content[MAX_CONFIG_JSON_SIZE];
    size_t bytes_read = fread(content, 1, size, fp);
    content[bytes_read] = '\0';
    fclose(fp);

    return content;
}

int save_ip_config(void) {
    // Read existing config to preserve other sections
    char* existing_config = read_config_file();

    // Extract sections to preserve
    char status_bar_section[1024] = "";
    char border_section[2048] = "";
    char theme_section[1024] = "";
    char fonts_section[2048] = "";

    if (existing_config) {
        extract_json_section(existing_config, "status_bar", status_bar_section, sizeof(status_bar_section));
        extract_json_section(existing_config, "border", border_section, sizeof(border_section));
        extract_json_section(existing_config, "theme", theme_section, sizeof(theme_section));
        extract_json_section(existing_config, "fonts", fonts_section, sizeof(fonts_section));
    }

    FILE *fp = fopen(STATUS_BAR_CONFIG_FILE, "w");
    if (!fp) return -1;

    fprintf(fp, "{\n");

    // Write status_bar section if exists
    if (status_bar_section[0] != '\0') {
        fprintf(fp, "  %s,\n", status_bar_section);
    }

    // Write border section if exists
    if (border_section[0] != '\0') {
        fprintf(fp, "  %s,\n", border_section);
    }

    // Write ip_config section
    fprintf(fp, "  \"ip_config\": {\n");
    fprintf(fp, "    \"type\": \"%s\",\n", ip_config.type == IP_TYPE_IPV4 ? "ipv4" : "ipv6");
    fprintf(fp, "    \"ipv4\": \"%s\",\n", ip_config.ipv4);
    fprintf(fp, "    \"ipv6\": \"%s\"\n", ip_config.ipv6);
    fprintf(fp, "  }");

    // Write theme section if exists
    if (theme_section[0] != '\0') {
        fprintf(fp, ",\n  %s", theme_section);
    }

    // Write fonts section if exists
    if (fonts_section[0] != '\0') {
        fprintf(fp, ",\n  %s", fonts_section);
    }

    fprintf(fp, "\n}\n");

    fclose(fp);
    return 0;
}

int load_ip_config(void) {
    // Read config file
    char* content = read_config_file();
    if (!content) {
        // Use defaults
        ip_config.type = IP_TYPE_IPV4;
        strcpy(ip_config.ipv4, "192.168.1.100");
        strcpy(ip_config.ipv6, "2001:0db8:85a3:0000:0000:8a2e:0370:7334");
        return 0;
    }

    // Extract ip_config section
    char ip_section[512];
    if (extract_json_section(content, "ip_config", ip_section, sizeof(ip_section)) != 0) {
        // IP config section doesn't exist, use defaults
        return 0;
    }

    // Parse type
    if (strstr(ip_section, "\"type\"")) {
        if (strstr(ip_section, "ipv4")) {
            ip_config.type = IP_TYPE_IPV4;
        } else if (strstr(ip_section, "ipv6")) {
            ip_config.type = IP_TYPE_IPV6;
        }
    }

    // Parse ipv4 address
    const char* ipv4_start = strstr(ip_section, "\"ipv4\"");
    if (ipv4_start) {
        const char* colon = strchr(ipv4_start, ':');
        if (colon) {
            const char* quote_start = strchr(colon, '"');
            if (quote_start) {
                quote_start++;
                const char* quote_end = strchr(quote_start, '"');
                if (quote_end) {
                    int len = quote_end - quote_start;
                    if (len < 16) {
                        strncpy(ip_config.ipv4, quote_start, len);
                        ip_config.ipv4[len] = '\0';
                    }
                }
            }
        }
    }

    // Parse ipv6 address
    const char* ipv6_start = strstr(ip_section, "\"ipv6\"");
    if (ipv6_start) {
        const char* colon = strchr(ipv6_start, ':');
        if (colon) {
            const char* quote_start = strchr(colon, '"');
            if (quote_start) {
                quote_start++;
                const char* quote_end = strchr(quote_start, '"');
                if (quote_end) {
                    int len = quote_end - quote_start;
                    if (len < 40) {
                        strncpy(ip_config.ipv6, quote_start, len);
                        ip_config.ipv6[len] = '\0';
                    }
                }
            }
        }
    }

    return 0;
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
    extern AppState app_state;
    if (app_state.font_24_bold) {
        lv_obj_set_style_text_font(ip_section_label, app_state.font_24_bold, 0);
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
    lv_label_set_long_mode(ip_display_label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(ip_display_label, SCREEN_WIDTH - CONTENT_WIDTH_LARGE_PADDING - 20);
    apply_label_style(ip_display_label);
    lv_obj_align(ip_display_label, LV_ALIGN_LEFT_MID, 0, 0);

    y_pos += 80;

    // Info text
    lv_obj_t *info_label = lv_label_create(content);
    lv_label_set_text(info_label, get_label("network_screen.ip_config_instruction"));
    apply_label_style(info_label);
    lv_obj_set_style_text_color(info_label, lv_color_hex(0x888888), 0);
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
