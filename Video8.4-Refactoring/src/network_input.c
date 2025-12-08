#include "../include/network_input.h"
#include "../include/network_ip_config.h"
#include "../include/network_ui.h"
#include "../include/label.h"
#include "../include/border.h"
#include "../include/state.h"
#include "../include/colors.h"
#include "../include/layout.h"
#include "../include/style.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// ============================================================================
// MODULE STATE
// ============================================================================

static input_state_t input_state = {
    .temp_ipv4 = "",
    .temp_ipv6 = "",
    .cursor_pos = 0
};

// ============================================================================
// PUBLIC API - INPUT STATE ACCESS
// ============================================================================

input_state_t* get_input_state(void) {
    return &input_state;
}

void init_input_state(void) {
    ip_config_t *config = get_ip_config();
    if (!config) return;

    // Initialize temp buffers with current IP addresses
    strncpy(input_state.temp_ipv4, config->ipv4, sizeof(input_state.temp_ipv4) - 1);
    input_state.temp_ipv4[sizeof(input_state.temp_ipv4) - 1] = '\0';
    strncpy(input_state.temp_ipv6, config->ipv6, sizeof(input_state.temp_ipv6) - 1);
    input_state.temp_ipv6[sizeof(input_state.temp_ipv6) - 1] = '\0';

    // Initialize cursor position to end of current IP address
    reset_cursor_position();
}

void reset_cursor_position(void) {
    ip_config_t *config = get_ip_config();
    if (!config) return;

    if (config->type == IP_TYPE_IPV4) {
        input_state.cursor_pos = strlen(input_state.temp_ipv4);
    } else {
        input_state.cursor_pos = strlen(input_state.temp_ipv6);
    }
}

int get_cursor_position(void) {
    return input_state.cursor_pos;
}

void set_cursor_position(int pos) {
    ip_config_t *config = get_ip_config();
    if (!config) return;

    if (config->type == IP_TYPE_IPV4) {
        int len = strlen(input_state.temp_ipv4);
        input_state.cursor_pos = (pos < 0) ? 0 : ((pos > len) ? len : pos);
    } else {
        int len = strlen(input_state.temp_ipv6);
        input_state.cursor_pos = (pos < 0) ? 0 : ((pos > len) ? len : pos);
    }
}

// ============================================================================
// EVENT CALLBACKS
// ============================================================================

void ip_edit_btn_callback(lv_event_t *e) {
    (void)e;
    show_ip_popup();
}

void ip_type_toggle_callback(lv_event_t *e) {
    lv_obj_t *sw = lv_event_get_target(e);
    ip_config_t *config = get_ip_config();
    if (!config) return;

    ip_type_t old_type = config->type;

    // Switch OFF = IPv4, Switch ON = IPv6
    if (lv_obj_has_state(sw, LV_STATE_CHECKED)) {
        config->type = IP_TYPE_IPV6;
    } else {
        config->type = IP_TYPE_IPV4;
    }

    // Only recreate if type actually changed
    if (old_type != config->type) {
        // Reset cursor position when switching types
        reset_cursor_position();
        // Recreate keypad for the new mode
        hide_ip_popup();
        show_ip_popup();
    }
}

void number_btn_callback(lv_event_t *e) {
    char ch = (char)(intptr_t)lv_event_get_user_data(e);
    ip_config_t *config = get_ip_config();
    if (!config) return;

    if (config->type == IP_TYPE_IPV4) {
        // IPv4 input - insert character at cursor position
        size_t len = strlen(input_state.temp_ipv4);

        // Safety checks: validate buffer capacity and cursor position
        if (len >= IPV4_MAX_LENGTH || input_state.cursor_pos > (int)len || input_state.cursor_pos < 0) {
            return;  // Prevent buffer overflow
        }

        // Shift characters to the right from cursor position
        for (int i = (int)len; i > input_state.cursor_pos; i--) {
            input_state.temp_ipv4[i] = input_state.temp_ipv4[i - 1];
        }
        input_state.temp_ipv4[input_state.cursor_pos] = ch;
        input_state.temp_ipv4[len + 1] = '\0';
        input_state.cursor_pos++;
        update_popup_ip_display();
    } else {
        // IPv6 input - insert character at cursor position
        size_t len = strlen(input_state.temp_ipv6);

        // Safety checks: validate buffer capacity and cursor position
        if (len >= IPV6_MAX_LENGTH || input_state.cursor_pos > (int)len || input_state.cursor_pos < 0) {
            return;  // Prevent buffer overflow
        }

        // Shift characters to the right from cursor position
        for (int i = (int)len; i > input_state.cursor_pos; i--) {
            input_state.temp_ipv6[i] = input_state.temp_ipv6[i - 1];
        }
        input_state.temp_ipv6[input_state.cursor_pos] = ch;
        input_state.temp_ipv6[len + 1] = '\0';
        input_state.cursor_pos++;
        update_popup_ip_display();
    }
}

void dot_colon_callback(lv_event_t *e) {
    (void)e;
    ip_config_t *config = get_ip_config();
    if (!config) return;

    if (config->type == IP_TYPE_IPV4) {
        // Add dot for IPv4 at cursor position
        size_t len = strlen(input_state.temp_ipv4);

        // Safety checks: validate buffer, cursor position, and prevent duplicate dots
        if (len == 0 || len >= IPV4_MAX_LENGTH || input_state.cursor_pos > (int)len || input_state.cursor_pos < 0) {
            return;
        }
        if (input_state.cursor_pos > 0 && input_state.temp_ipv4[input_state.cursor_pos - 1] == '.') {
            return;  // Prevent consecutive dots
        }

        // Shift characters to the right from cursor position
        for (int i = (int)len; i > input_state.cursor_pos; i--) {
            input_state.temp_ipv4[i] = input_state.temp_ipv4[i - 1];
        }
        input_state.temp_ipv4[input_state.cursor_pos] = '.';
        input_state.temp_ipv4[len + 1] = '\0';
        input_state.cursor_pos++;
        update_popup_ip_display();
    } else {
        // Add colon for IPv6 at cursor position
        size_t len = strlen(input_state.temp_ipv6);

        // Safety checks: validate buffer, cursor position, and prevent duplicate colons
        if (len == 0 || len >= IPV6_MAX_LENGTH || input_state.cursor_pos > (int)len || input_state.cursor_pos < 0) {
            return;
        }
        if (input_state.cursor_pos > 0 && input_state.temp_ipv6[input_state.cursor_pos - 1] == ':') {
            return;  // Prevent consecutive colons (except for ::)
        }

        // Shift characters to the right from cursor position
        for (int i = (int)len; i > input_state.cursor_pos; i--) {
            input_state.temp_ipv6[i] = input_state.temp_ipv6[i - 1];
        }
        input_state.temp_ipv6[input_state.cursor_pos] = ':';
        input_state.temp_ipv6[len + 1] = '\0';
        input_state.cursor_pos++;
        update_popup_ip_display();
    }
}

void backspace_callback(lv_event_t *e) {
    (void)e;
    ip_config_t *config = get_ip_config();
    if (!config) return;

    if (config->type == IP_TYPE_IPV4) {
        int len = strlen(input_state.temp_ipv4);
        if (len > 0 && input_state.cursor_pos > 0) {
            // Shift characters to the left from cursor position
            for (int i = input_state.cursor_pos - 1; i < len; i++) {
                input_state.temp_ipv4[i] = input_state.temp_ipv4[i + 1];
            }
            input_state.cursor_pos--;
            update_popup_ip_display();
        }
    } else {
        int len = strlen(input_state.temp_ipv6);
        if (len > 0 && input_state.cursor_pos > 0) {
            // Shift characters to the left from cursor position
            for (int i = input_state.cursor_pos - 1; i < len; i++) {
                input_state.temp_ipv6[i] = input_state.temp_ipv6[i + 1];
            }
            input_state.cursor_pos--;
            update_popup_ip_display();
        }
    }
}

void clear_all_callback(lv_event_t *e) {
    (void)e;
    ip_config_t *config = get_ip_config();
    if (!config) return;

    if (config->type == IP_TYPE_IPV4) {
        input_state.temp_ipv4[0] = '\0';
    } else {
        input_state.temp_ipv6[0] = '\0';
    }
    input_state.cursor_pos = 0;
    update_popup_ip_display();
}

// Callback to close error message box
static void error_msgbox_event_cb(lv_event_t *e) {
    lv_obj_t *mbox = lv_event_get_current_target(e);
    lv_obj_del(mbox);
    // Remove the red border when error message is closed
    remove_border();

    // Resume cursor blinking when message box is closed
    cursor_state_t *cursor = get_cursor_state();
    if (cursor) {
        cursor_start_blinking(cursor);
    }
}

void save_ip_callback(lv_event_t *e) {
    (void)e;
    ip_config_t *config = get_ip_config();
    if (!config) return;

    bool valid = false;
    const char *error_msg = NULL;

    if (config->type == IP_TYPE_IPV4) {
        // Validate IPv4
        if (strlen(input_state.temp_ipv4) == 0) {
            error_msg = get_label("network_screen.error_empty");
        } else if (!is_valid_ipv4(input_state.temp_ipv4)) {
            error_msg = get_label("network_screen.error_invalid_ipv4");
        } else {
            // Use strncpy with proper null termination for safety
            strncpy(config->ipv4, input_state.temp_ipv4, sizeof(config->ipv4) - 1);
            config->ipv4[sizeof(config->ipv4) - 1] = '\0';
            valid = true;
        }
    } else {
        // Validate IPv6
        if (strlen(input_state.temp_ipv6) == 0) {
            error_msg = get_label("network_screen.error_empty");
        } else if (!is_valid_ipv6(input_state.temp_ipv6)) {
            error_msg = get_label("network_screen.error_invalid_ipv6");
        } else {
            // Use strncpy with proper null termination for safety
            strncpy(config->ipv6, input_state.temp_ipv6, sizeof(config->ipv6) - 1);
            config->ipv6[sizeof(config->ipv6) - 1] = '\0';
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

        // Stop cursor blinking while error message box is shown
        cursor_state_t *cursor = get_cursor_state();
        if (cursor) {
            cursor_stop_blinking(cursor);
        }

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

void cancel_btn_callback(lv_event_t *e) {
    (void)e;
    hide_ip_popup();
}
