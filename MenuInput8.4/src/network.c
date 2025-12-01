#include "../include/network.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/screen_components.h"
#include "../include/navigation.h"

// ============================================================================
// NETWORK SCREEN COMPONENTS
// ============================================================================

static lv_obj_t *create_network_content(lv_obj_t *parent) {
    lv_obj_t *content = create_standard_content(parent);

    // Create network settings text
    lv_obj_t *network_label = lv_label_create(content);
    lv_label_set_long_mode(network_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(network_label, SCREEN_WIDTH - 20);
    apply_label_style(network_label);
    lv_obj_set_style_pad_all(network_label, 10, 0);
    lv_obj_align(network_label, LV_ALIGN_TOP_LEFT, 10, 10);

    lv_label_set_text(network_label,
        "네트워크 설정\n\n"
        "Wi-Fi 설정\n"
        "- SSID: MyNetwork\n"
        "- 상태: 연결됨\n\n"
        "이더넷 설정\n"
        "- IP: 192.168.1.100\n"
        "- 상태: 비활성\n\n"
        "VPN 설정\n"
        "- 상태: 비활성"
    );

    return content;
}

// ============================================================================
// NETWORK SCREEN CREATION
// ============================================================================

void create_network_screen(void) {
    lv_obj_t *network_screen = create_screen_base(SCREEN_NETWORK);

    create_standard_title_bar(network_screen, SCREEN_NETWORK);
    create_network_content(network_screen);
    create_standard_status_bar(network_screen);

    finalize_screen(network_screen, SCREEN_NETWORK);
}
