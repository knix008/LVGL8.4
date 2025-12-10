#include "../include/state.h"
#include "../include/config.h"
#include <string.h>

// ============================================================================
// INTERNAL STATE
// ============================================================================

// The actual application state - encapsulated and not exported
static AppState app_state = {0};

// ============================================================================
// INITIALIZATION
// ============================================================================

int app_state_init(void) {
    // Initialize default values
    app_state.bg_color = COLOR_BG_DARK;
    app_state.title_bar_color = COLOR_BG_TITLE;
    app_state.status_bar_color = COLOR_BG_TITLE;
    app_state.button_color = COLOR_BUTTON_BG;
    app_state.button_border_color = COLOR_BORDER;

    // Initialize default language
    strncpy(app_state.current_language, "ko", sizeof(app_state.current_language) - 1);
    app_state.current_language[sizeof(app_state.current_language) - 1] = '\0';

    // Initialize font sizes (will be loaded from config later)
    app_state.font_size_title_bar = 20;
    app_state.font_size_label = 20;
    app_state.font_size_button_label = 20;
    app_state.font_size_bold = 24;

    // Initialize font names
    strncpy(app_state.font_name_title, "NotoSansKR-Bold.ttf", sizeof(app_state.font_name_title) - 1);
    app_state.font_name_title[sizeof(app_state.font_name_title) - 1] = '\0';

    strncpy(app_state.font_name_status_bar, "NotoSansKR-Regular.ttf", sizeof(app_state.font_name_status_bar) - 1);
    app_state.font_name_status_bar[sizeof(app_state.font_name_status_bar) - 1] = '\0';

    strncpy(app_state.font_name_button_label, "NotoSansKR-Medium.ttf", sizeof(app_state.font_name_button_label) - 1);
    app_state.font_name_button_label[sizeof(app_state.font_name_button_label) - 1] = '\0';

    // Initialize calendar date to current date (will be set properly later)
    app_state.calendar_date.year = 2024;
    app_state.calendar_date.month = 1;
    app_state.calendar_date.day = 1;

    return 0;
}

void app_state_cleanup(void) {
    // Currently no dynamic memory to free, but placeholder for future use
}

// ============================================================================
// SCREEN MANAGEMENT
// ============================================================================

lv_obj_t* app_state_get_screen(void) {
    return app_state.screen;
}

void app_state_set_screen(lv_obj_t *screen) {
    app_state.screen = screen;
}

// ============================================================================
// UI ELEMENTS
// ============================================================================

lv_obj_t* app_state_get_title_bar(void) {
    return app_state.title_bar;
}

void app_state_set_title_bar(lv_obj_t *title_bar) {
    app_state.title_bar = title_bar;
}

lv_obj_t* app_state_get_status_bar(void) {
    return app_state.status_bar;
}

void app_state_set_status_bar(lv_obj_t *status_bar) {
    app_state.status_bar = status_bar;
}

lv_obj_t* app_state_get_title_label(void) {
    return app_state.title_label;
}

void app_state_set_title_label(lv_obj_t *label) {
    app_state.title_label = label;
}

lv_obj_t* app_state_get_current_title_label(void) {
    return app_state.current_title_label;
}

void app_state_set_current_title_label(lv_obj_t *label) {
    app_state.current_title_label = label;
}

lv_obj_t* app_state_get_welcome_label(void) {
    return app_state.welcome_message_label;
}

void app_state_set_welcome_label(lv_obj_t *label) {
    app_state.welcome_message_label = label;
}

lv_obj_t* app_state_get_menu_button_label(void) {
    return app_state.menu_button_label;
}

void app_state_set_menu_button_label(lv_obj_t *label) {
    app_state.menu_button_label = label;
}

lv_obj_t* app_state_get_exit_button_label(void) {
    return app_state.exit_button_label;
}

void app_state_set_exit_button_label(lv_obj_t *label) {
    app_state.exit_button_label = label;
}

// ============================================================================
// FONT MANAGEMENT
// ============================================================================

lv_font_t* app_state_get_font_20(void) {
    return app_state.font_20;
}

void app_state_set_font_20(lv_font_t *font) {
    app_state.font_20 = font;
}

lv_font_t* app_state_get_font_button(void) {
    return app_state.font_button;
}

void app_state_set_font_button(lv_font_t *font) {
    app_state.font_button = font;
}

lv_font_t* app_state_get_font_24_bold(void) {
    return app_state.font_24_bold;
}

void app_state_set_font_24_bold(lv_font_t *font) {
    app_state.font_24_bold = font;
}

// ============================================================================
// COLOR MANAGEMENT
// ============================================================================

uint32_t app_state_get_bg_color(void) {
    return app_state.bg_color;
}

void app_state_set_bg_color(uint32_t color) {
    app_state.bg_color = color;
}

uint32_t app_state_get_title_bar_color(void) {
    return app_state.title_bar_color;
}

void app_state_set_title_bar_color(uint32_t color) {
    app_state.title_bar_color = color;
}

uint32_t app_state_get_status_bar_color(void) {
    return app_state.status_bar_color;
}

void app_state_set_status_bar_color(uint32_t color) {
    app_state.status_bar_color = color;
}

uint32_t app_state_get_button_color(void) {
    return app_state.button_color;
}

void app_state_set_button_color(uint32_t color) {
    app_state.button_color = color;
}

uint32_t app_state_get_button_border_color(void) {
    return app_state.button_border_color;
}

void app_state_set_button_border_color(uint32_t color) {
    app_state.button_border_color = color;
}

// ============================================================================
// LANGUAGE MANAGEMENT
// ============================================================================

const char* app_state_get_language(void) {
    return app_state.current_language;
}

void app_state_set_language(const char *lang) {
    if (lang) {
        strncpy(app_state.current_language, lang, sizeof(app_state.current_language) - 1);
        app_state.current_language[sizeof(app_state.current_language) - 1] = '\0';
    }
}

// ============================================================================
// FONT CONFIGURATION
// ============================================================================

int app_state_get_font_size_title_bar(void) {
    return app_state.font_size_title_bar;
}

void app_state_set_font_size_title_bar(int size) {
    app_state.font_size_title_bar = size;
}

int app_state_get_font_size_label(void) {
    return app_state.font_size_label;
}

void app_state_set_font_size_label(int size) {
    app_state.font_size_label = size;
}

int app_state_get_font_size_button_label(void) {
    return app_state.font_size_button_label;
}

void app_state_set_font_size_button_label(int size) {
    app_state.font_size_button_label = size;
}

int app_state_get_font_size_bold(void) {
    return app_state.font_size_bold;
}

void app_state_set_font_size_bold(int size) {
    app_state.font_size_bold = size;
}

const char* app_state_get_font_name_title(void) {
    return app_state.font_name_title;
}

void app_state_set_font_name_title(const char *name) {
    if (name) {
        strncpy(app_state.font_name_title, name, sizeof(app_state.font_name_title) - 1);
        app_state.font_name_title[sizeof(app_state.font_name_title) - 1] = '\0';
    }
}

const char* app_state_get_font_name_status_bar(void) {
    return app_state.font_name_status_bar;
}

void app_state_set_font_name_status_bar(const char *name) {
    if (name) {
        strncpy(app_state.font_name_status_bar, name, sizeof(app_state.font_name_status_bar) - 1);
        app_state.font_name_status_bar[sizeof(app_state.font_name_status_bar) - 1] = '\0';
    }
}

const char* app_state_get_font_name_button_label(void) {
    return app_state.font_name_button_label;
}

void app_state_set_font_name_button_label(const char *name) {
    if (name) {
        strncpy(app_state.font_name_button_label, name, sizeof(app_state.font_name_button_label) - 1);
        app_state.font_name_button_label[sizeof(app_state.font_name_button_label) - 1] = '\0';
    }
}

// ============================================================================
// MENU ITEM SELECTION
// ============================================================================

bool app_state_is_menu_item_selected(int index) {
    if (index < 0 || index >= MAX_STATUS_ICONS) {
        return false;
    }
    return app_state.menu_item_selected[index];
}

void app_state_set_menu_item_selected(int index, bool selected) {
    if (index >= 0 && index < MAX_STATUS_ICONS) {
        app_state.menu_item_selected[index] = selected;
    }
}

int app_state_get_menu_item_order(int index) {
    if (index >= 0 && index < MAX_STATUS_ICONS) {
        return app_state.menu_item_order[index];
    }
    return -1;
}

void app_state_set_menu_item_order(int index, int order) {
    if (index >= 0 && index < MAX_STATUS_ICONS) {
        app_state.menu_item_order[index] = order;
    }
}

lv_obj_t* app_state_get_status_icon(int index) {
    if (index < 0 || index >= MAX_STATUS_ICONS) {
        return NULL;
    }
    return app_state.status_icons[index];
}

void app_state_set_status_icon(int index, lv_obj_t *icon) {
    if (index >= 0 && index < MAX_STATUS_ICONS) {
        app_state.status_icons[index] = icon;
    }
}

// ============================================================================
// CALENDAR DATE
// ============================================================================

calendar_date_t app_state_get_calendar_date(void) {
    return app_state.calendar_date;
}

void app_state_set_calendar_date(calendar_date_t date) {
    app_state.calendar_date = date;
}

// ============================================================================
// DIRECT STATE ACCESS (for init.c only)
// ============================================================================

AppState* app_state_get_internal(void) {
    return &app_state;
}
