#include "../include/network.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"
#include "../include/label.h"
#include <stdio.h>

// ============================================================================
// NETWORK SCREEN COMPONENTS
// ============================================================================

static lv_obj_t *create_network_content(lv_obj_t *parent) {
    lv_obj_t *content = create_standard_content(parent);

    // Create network settings text
    lv_obj_t *network_label = lv_label_create(content);
    lv_label_set_long_mode(network_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(network_label, SCREEN_WIDTH - CONTENT_WIDTH_PADDING);
    apply_label_style(network_label);
    lv_obj_set_style_pad_all(network_label, CONTENT_PADDING, 0);
    lv_obj_align(network_label, LV_ALIGN_TOP_LEFT, CONTENT_PADDING, CONTENT_PADDING);

    char network_text[512];
    snprintf(network_text, sizeof(network_text),
        "%s\n\n"
        "%s\n"
        "- %s\n"
        "- %s\n\n"
        "%s\n"
        "- %s\n"
        "- %s\n\n"
        "%s\n"
        "- %s",
        get_label("network_screen.title"),
        get_label("network_screen.wifi_settings"),
        get_label("network_screen.wifi_ssid"),
        get_label("network_screen.wifi_status"),
        get_label("network_screen.ethernet_settings"),
        get_label("network_screen.ethernet_ip"),
        get_label("network_screen.ethernet_status"),
        get_label("network_screen.vpn_settings"),
        get_label("network_screen.vpn_status")
    );
    lv_label_set_text(network_label, network_text);

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
