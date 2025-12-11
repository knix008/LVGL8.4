#include "../include/network.h"
#include "../include/config.h"
#include "../include/state.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"
#include "../include/label.h"
#include "../include/border.h"
#include "../include/cursor.h"
#include "../include/colors.h"
#include "../include/layout.h"
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
static lv_obj_t *ip_type_switch = NULL;

// Temporary input buffers
static char temp_ipv4[16] = "";
static char temp_ipv6[40] = "";

// Cursor state
static int cursor_pos = 0;
static cursor_state_t cursor_state;

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

    char display_text[64];

    if (ip_config.type == IP_TYPE_IPV4) {
        if (strlen(temp_ipv4) > 0) {
            // Insert cursor at cursor_pos
            int len = strlen(temp_ipv4);
            if (cursor_pos > len) cursor_pos = len;

            // Build display string with cursor
            if (cursor_is_visible(&cursor_state)) {
                strncpy(display_text, temp_ipv4, cursor_pos);
                display_text[cursor_pos] = '|';
                strcpy(display_text + cursor_pos + 1, temp_ipv4 + cursor_pos);
            } else {
                strcpy(display_text, temp_ipv4);
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
        if (strlen(temp_ipv6) > 0) {
            // Insert cursor at cursor_pos
            int len = strlen(temp_ipv6);
            if (cursor_pos > len) cursor_pos = len;

            // Build display string with cursor
            if (cursor_is_visible(&cursor_state)) {
                strncpy(display_text, temp_ipv6, cursor_pos);
                display_text[cursor_pos] = '|';
                strcpy(display_text + cursor_pos + 1, temp_ipv6 + cursor_pos);
            } else {
                strcpy(display_text, temp_ipv6);
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
// CURSOR ANIMATION
// ============================================================================
// EVENT CALLBACKS
// ============================================================================

static void ip_edit_btn_callback(lv_event_t *e) {
    (void)e;
    show_ip_popup();
}

static void ip_type_toggle_callback(lv_event_t *e) {
    lv_obj_t *sw = lv_event_get_target(e);
    ip_type_t old_type = ip_config.type;

    // Switch OFF = IPv4, Switch ON = IPv6
    if (lv_obj_has_state(sw, LV_STATE_CHECKED)) {
        ip_config.type = IP_TYPE_IPV6;
    } else {
        ip_config.type = IP_TYPE_IPV4;
    }

    // Only recreate if type actually changed
    if (old_type != ip_config.type) {
        // Reset cursor position when switching types
        if (ip_config.type == IP_TYPE_IPV4) {
            cursor_pos = strlen(temp_ipv4);
        } else {
            cursor_pos = strlen(temp_ipv6);
        }
        // Recreate keypad for the new mode
        hide_ip_popup();
        show_ip_popup();
    }
}

static void number_btn_callback(lv_event_t *e) {
    char ch = (char)(intptr_t)lv_event_get_user_data(e);

    if (ip_config.type == IP_TYPE_IPV4) {
        // IPv4 input - insert character at cursor position
        size_t len = strlen(temp_ipv4);

        // Safety checks: validate buffer capacity and cursor position
        if (len >= IPV4_MAX_LENGTH || cursor_pos > (int)len || cursor_pos < 0) {
            return;  // Prevent buffer overflow
        }

        // Shift characters to the right from cursor position
        for (int i = (int)len; i > cursor_pos; i--) {
            temp_ipv4[i] = temp_ipv4[i - 1];
        }
        temp_ipv4[cursor_pos] = ch;
        temp_ipv4[len + 1] = '\0';
        cursor_pos++;
        update_popup_ip_display();
    } else {
        // IPv6 input - insert character at cursor position
        size_t len = strlen(temp_ipv6);

        // Safety checks: validate buffer capacity and cursor position
        if (len >= IPV6_MAX_LENGTH || cursor_pos > (int)len || cursor_pos < 0) {
            return;  // Prevent buffer overflow
        }

        // Shift characters to the right from cursor position
        for (int i = (int)len; i > cursor_pos; i--) {
            temp_ipv6[i] = temp_ipv6[i - 1];
        }
        temp_ipv6[cursor_pos] = ch;
        temp_ipv6[len + 1] = '\0';
        cursor_pos++;
        update_popup_ip_display();
    }
}

static void dot_colon_callback(lv_event_t *e) {
    (void)e;

    if (ip_config.type == IP_TYPE_IPV4) {
        // Add dot for IPv4 at cursor position
        size_t len = strlen(temp_ipv4);

        // Safety checks: validate buffer, cursor position, and prevent duplicate dots
        if (len == 0 || len >= IPV4_MAX_LENGTH || cursor_pos > (int)len || cursor_pos < 0) {
            return;
        }
        if (cursor_pos > 0 && temp_ipv4[cursor_pos - 1] == '.') {
            return;  // Prevent consecutive dots
        }

        // Shift characters to the right from cursor position
        for (int i = (int)len; i > cursor_pos; i--) {
            temp_ipv4[i] = temp_ipv4[i - 1];
        }
        temp_ipv4[cursor_pos] = '.';
        temp_ipv4[len + 1] = '\0';
        cursor_pos++;
        update_popup_ip_display();
    } else {
        // Add colon for IPv6 at cursor position
        size_t len = strlen(temp_ipv6);

        // Safety checks: validate buffer, cursor position, and prevent duplicate colons
        if (len == 0 || len >= IPV6_MAX_LENGTH || cursor_pos > (int)len || cursor_pos < 0) {
            return;
        }
        if (cursor_pos > 0 && temp_ipv6[cursor_pos - 1] == ':') {
            return;  // Prevent consecutive colons (except for ::)
        }

        // Shift characters to the right from cursor position
        for (int i = (int)len; i > cursor_pos; i--) {
            temp_ipv6[i] = temp_ipv6[i - 1];
        }
        temp_ipv6[cursor_pos] = ':';
        temp_ipv6[len + 1] = '\0';
        cursor_pos++;
        update_popup_ip_display();
    }
}

static void backspace_callback(lv_event_t *e) {
    (void)e;

    if (ip_config.type == IP_TYPE_IPV4) {
        int len = strlen(temp_ipv4);
        if (len > 0 && cursor_pos > 0) {
            // Shift characters to the left from cursor position
            for (int i = cursor_pos - 1; i < len; i++) {
                temp_ipv4[i] = temp_ipv4[i + 1];
            }
            cursor_pos--;
            update_popup_ip_display();
        }
    } else {
        int len = strlen(temp_ipv6);
        if (len > 0 && cursor_pos > 0) {
            // Shift characters to the left from cursor position
            for (int i = cursor_pos - 1; i < len; i++) {
                temp_ipv6[i] = temp_ipv6[i + 1];
            }
            cursor_pos--;
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
    cursor_pos = 0;
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
    // Remove the red border when error message is closed
    remove_border();
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
            // Use strncpy with proper null termination for safety
            strncpy(ip_config.ipv4, temp_ipv4, sizeof(ip_config.ipv4) - 1);
            ip_config.ipv4[sizeof(ip_config.ipv4) - 1] = '\0';
            valid = true;
        }
    } else {
        // Validate IPv6
        if (strlen(temp_ipv6) == 0) {
            error_msg = get_label("network_screen.error_empty");
        } else if (!is_valid_ipv6(temp_ipv6)) {
            error_msg = get_label("network_screen.error_invalid_ipv6");
        } else {
            // Use strncpy with proper null termination for safety
            strncpy(ip_config.ipv6, temp_ipv6, sizeof(ip_config.ipv6) - 1);
            ip_config.ipv6[sizeof(ip_config.ipv6) - 1] = '\0';
            valid = true;
        }
    }

    if (valid) {
        save_ip_config();
        update_ip_display_label();
        hide_ip_popup();
    } else {
        // Show red border to indicate error
        show_red_border();
        
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
            lv_obj_set_width(mbox, UI_POPUP_MESSAGE_BOX_WIDTH);
            lv_obj_set_style_bg_color(mbox, lv_color_hex(UI_COLOR_BG_POPUP), 0);
            lv_obj_set_style_bg_opa(mbox, LV_OPA_70, 0);
            lv_obj_set_style_border_color(mbox, lv_color_hex(UI_COLOR_BORDER_ERROR), 0);
            lv_obj_set_style_border_width(mbox, 2, 0);

            // Style the text
            lv_obj_t *text = lv_msgbox_get_text(mbox);
            if (text) {
                lv_obj_set_style_text_color(text, lv_color_hex(UI_COLOR_TEXT_PRIMARY), 0);
                if (app_state_get_font_20()) {
                    lv_obj_set_style_text_font(text, app_state_get_font_20(), 0);
                }
            }

            // Style the title
            lv_obj_t *title = lv_msgbox_get_title(mbox);
            if (title) {
                lv_obj_set_style_text_color(title, lv_color_hex(UI_COLOR_TEXT_ERROR), 0);
                if (app_state_get_font_24_bold()) {
                    lv_obj_set_style_text_font(title, app_state_get_font_24_bold(), 0);
                }
            }

            // Style the buttons
            lv_obj_t *btns_obj = lv_msgbox_get_btns(mbox);
            if (btns_obj) {
                if (app_state_get_font_20()) {
                    lv_obj_set_style_text_font(btns_obj, app_state_get_font_20(), 0);
                }
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
    lv_obj_set_style_pad_all(ip_container, 0, 0);
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
    if (ip_config.type == IP_TYPE_IPV6) {
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
    int btn_width = UI_CONTAINER_CONTROL_BUTTON_WIDTH;
    int btn_height = UI_CONTAINER_CONTROL_BUTTON_HEIGHT;
    int btn_gap = 10;
    int total_width = btn_width * 2 + btn_gap;

    lv_obj_t *ctrl_container = lv_obj_create(ip_container);
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
    lv_label_set_text(save_label, get_label("network_screen.save_button"));
    apply_label_style(save_label);
    lv_obj_center(save_label);
    lv_obj_add_event_cb(save_btn, save_ip_callback, LV_EVENT_CLICKED, NULL);

    lv_obj_t *cancel_btn = lv_btn_create(ctrl_container);
    lv_obj_set_size(cancel_btn, btn_width, btn_height);
    lv_obj_set_pos(cancel_btn, btn_width + btn_gap, 0);
    apply_button_style(cancel_btn, 0);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(UI_COLOR_BTN_DANGER), 0);

    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, get_label("network_screen.cancel_button"));
    apply_label_style(cancel_label);
    lv_obj_center(cancel_label);
    lv_obj_add_event_cb(cancel_btn, cancel_btn_callback, LV_EVENT_CLICKED, NULL);

    // Initialize temp buffers with current IP addresses BEFORE updating display
    // Use strncpy with proper null termination for safety
    strncpy(temp_ipv4, ip_config.ipv4, sizeof(temp_ipv4) - 1);
    temp_ipv4[sizeof(temp_ipv4) - 1] = '\0';
    strncpy(temp_ipv6, ip_config.ipv6, sizeof(temp_ipv6) - 1);
    temp_ipv6[sizeof(temp_ipv6) - 1] = '\0';

    // Initialize cursor position to end of current IP address
    if (ip_config.type == IP_TYPE_IPV4) {
        cursor_pos = strlen(temp_ipv4);
    } else {
        cursor_pos = strlen(temp_ipv6);
    }

    // Initialize cursor state and start blinking animation
    cursor_state_init(&cursor_state, update_popup_ip_display);
    cursor_start_blinking(&cursor_state);

    // Update display to show current IP address
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
    cursor_stop_blinking(&cursor_state);
    if (ip_popup) {
        lv_obj_del(ip_popup);
        ip_popup = NULL;
    }
}

// ============================================================================
// CONFIGURATION PERSISTENCE
// ============================================================================

int save_ip_config(void) {
    FILE *fp = fopen(IP_CONFIG_FILE, "w");
    if (!fp) return -1;

    fprintf(fp, "{\n");
    fprintf(fp, "  \"type\": \"%s\",\n", ip_config.type == IP_TYPE_IPV4 ? "ipv4" : "ipv6");
    fprintf(fp, "  \"ipv4\": \"%s\",\n", ip_config.ipv4);
    fprintf(fp, "  \"ipv6\": \"%s\"\n", ip_config.ipv6);
    fprintf(fp, "}\n");

    fclose(fp);
    return 0;
}

int load_ip_config(void) {
    // Read IP config file
    FILE *fp = fopen(IP_CONFIG_FILE, "r");
    if (!fp) {
        // Use defaults
        ip_config.type = IP_TYPE_IPV4;
        strncpy(ip_config.ipv4, "192.168.1.100", sizeof(ip_config.ipv4) - 1);
        ip_config.ipv4[sizeof(ip_config.ipv4) - 1] = '\0';
        strncpy(ip_config.ipv6, "2001:0db8:85a3:0000:0000:8a2e:0370:7334", sizeof(ip_config.ipv6) - 1);
        ip_config.ipv6[sizeof(ip_config.ipv6) - 1] = '\0';
        return 0;
    }

    // Use static buffer instead of malloc for memory safety
    static char content[512];  // IP config file is small (type + 2 IP addresses)

    // Read file content
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Ensure file size doesn't exceed buffer
    if (file_size >= (long)sizeof(content)) {
        fclose(fp);
        // Use defaults if file is too large
        ip_config.type = IP_TYPE_IPV4;
        strncpy(ip_config.ipv4, "192.168.1.100", sizeof(ip_config.ipv4) - 1);
        ip_config.ipv4[sizeof(ip_config.ipv4) - 1] = '\0';
        strncpy(ip_config.ipv6, "2001:0db8:85a3:0000:0000:8a2e:0370:7334", sizeof(ip_config.ipv6) - 1);
        ip_config.ipv6[sizeof(ip_config.ipv6) - 1] = '\0';
        return -1;
    }

    size_t bytes_read = fread(content, 1, file_size, fp);
    content[bytes_read] = '\0';
    fclose(fp);

    // Parse type
    if (strstr(content, "\"type\"")) {
        if (strstr(content, "ipv4")) {
            ip_config.type = IP_TYPE_IPV4;
        } else if (strstr(content, "ipv6")) {
            ip_config.type = IP_TYPE_IPV6;
        }
    }

    // Parse ipv4 address
    const char* ipv4_start = strstr(content, "\"ipv4\":");
    if (ipv4_start) {
        // Skip past "ipv4":
        const char* value_start = ipv4_start + 7; // length of "ipv4":
        const char* quote_start = strchr(value_start, '"');
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

    // Parse ipv6 address
    const char* ipv6_start = strstr(content, "\"ipv6\":");
    if (ipv6_start) {
        // Skip past "ipv6":
        const char* value_start = ipv6_start + 7; // length of "ipv6":
        const char* quote_start = strchr(value_start, '"');
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
