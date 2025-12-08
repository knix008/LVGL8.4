#include "../include/admin_calendar.h"
#include "../include/config.h"
#include "../include/state.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/label.h"
#include "../include/ui_helpers.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// CALENDAR FUNCTIONALITY IMPLEMENTATION
// ============================================================================

// Static display label for main admin screen
static lv_obj_t *calendar_display_label = NULL;

// Static variables for popup calendar
static calendar_date_t popup_calendar_date;
static lv_obj_t *popup_calendar_display_label = NULL;
static lv_obj_t *popup_month_label = NULL;
static lv_obj_t *popup_day_label = NULL;
static lv_obj_t *popup_year_label = NULL;
static lv_obj_t *popup_month_button = NULL;
static lv_obj_t *popup_day_button = NULL;
static lv_obj_t *popup_year_button = NULL;

// Calendar selection mode for popup
typedef enum {
    POPUP_CALENDAR_MODE_MONTH,
    POPUP_CALENDAR_MODE_DAY,
    POPUP_CALENDAR_MODE_YEAR
} popup_calendar_mode_t;

static popup_calendar_mode_t popup_current_mode = POPUP_CALENDAR_MODE_MONTH;

// Forward declarations
static void popup_update_button_colors(void);
static void popup_update_calendar_displays(void);
static void popup_calendar_prev_cb(lv_event_t *e);
static void popup_calendar_next_cb(lv_event_t *e);
static void popup_calendar_select_month_cb(lv_event_t *e);
static void popup_calendar_select_day_cb(lv_event_t *e);
static void popup_calendar_select_year_cb(lv_event_t *e);
static void popup_calendar_enter_cb(lv_event_t *e);
static void calendar_popup_close_cb(lv_event_t *e);

// Helper to update calendar display
void update_calendar_display(void) {
    if (!calendar_display_label) return;

    char date_text[64];
    char display_text[128];

    calendar_date_t calendar_date = app_state_get_calendar_date();
    calendar_format_date_string(&calendar_date, date_text, sizeof(date_text));

    // Get day of week name
    int day_of_week = calendar_get_day_of_week(&calendar_date);
    const char* day_name = calendar_get_day_name(day_of_week);

    // Format display with day of week
    snprintf(display_text, sizeof(display_text), "%s (%s)", date_text, day_name);
    lv_label_set_text(calendar_display_label, display_text);

    // Save configuration
    save_theme_config();
}

// Helper function to update popup button colors based on selection
static void popup_update_button_colors(void) {
    lv_color_t default_color = lv_color_hex(0xFF9800); // Orange
    lv_color_t selected_color = lv_color_hex(0xBF360C); // Darker orange

    // Update button colors based on current mode
    if (popup_month_button) {
        lv_obj_set_style_bg_color(popup_month_button,
            (popup_current_mode == POPUP_CALENDAR_MODE_MONTH) ? selected_color : default_color, 0);
    }
    if (popup_day_button) {
        lv_obj_set_style_bg_color(popup_day_button,
            (popup_current_mode == POPUP_CALENDAR_MODE_DAY) ? selected_color : default_color, 0);
    }
    if (popup_year_button) {
        lv_obj_set_style_bg_color(popup_year_button,
            (popup_current_mode == POPUP_CALENDAR_MODE_YEAR) ? selected_color : default_color, 0);
    }
}

// Helper function to update popup displays
static void popup_update_calendar_displays(void) {
    if (!popup_calendar_display_label) return;

    char date_text[64];
    char main_display_text[128];

    calendar_format_date_string(&popup_calendar_date, date_text, sizeof(date_text));

    // Get day of week name
    int day_of_week = calendar_get_day_of_week(&popup_calendar_date);
    const char* day_name = calendar_get_day_name(day_of_week);

    // Format main display with day of week
    snprintf(main_display_text, sizeof(main_display_text), "%s (%s)", date_text, day_name);
    lv_label_set_text(popup_calendar_display_label, main_display_text);

    // Update individual labels
    if (popup_month_label) {
        lv_label_set_text(popup_month_label, calendar_get_month_abbr(popup_calendar_date.month));
    }
    if (popup_day_label) {
        char day_text[8];
        snprintf(day_text, sizeof(day_text), "%d", popup_calendar_date.day);
        lv_label_set_text(popup_day_label, day_text);
    }
    if (popup_year_label) {
        char year_text[8];
        snprintf(year_text, sizeof(year_text), "%d", popup_calendar_date.year);
        lv_label_set_text(popup_year_label, year_text);
    }

    // Update button colors
    popup_update_button_colors();
}

// Popup navigation callbacks
static void popup_calendar_prev_cb(lv_event_t *e) {
    (void)e;
    switch (popup_current_mode) {
        case POPUP_CALENDAR_MODE_MONTH:
            calendar_prev_month(&popup_calendar_date);
            break;
        case POPUP_CALENDAR_MODE_DAY:
            calendar_prev_day(&popup_calendar_date);
            break;
        case POPUP_CALENDAR_MODE_YEAR:
            calendar_prev_year(&popup_calendar_date);
            break;
    }
    popup_update_calendar_displays();
}

static void popup_calendar_next_cb(lv_event_t *e) {
    (void)e;
    switch (popup_current_mode) {
        case POPUP_CALENDAR_MODE_MONTH:
            calendar_next_month(&popup_calendar_date);
            break;
        case POPUP_CALENDAR_MODE_DAY:
            calendar_next_day(&popup_calendar_date);
            break;
        case POPUP_CALENDAR_MODE_YEAR:
            calendar_next_year(&popup_calendar_date);
            break;
    }
    popup_update_calendar_displays();
}

static void popup_calendar_select_month_cb(lv_event_t *e) {
    (void)e;
    popup_current_mode = POPUP_CALENDAR_MODE_MONTH;
    popup_update_calendar_displays();
}

static void popup_calendar_select_day_cb(lv_event_t *e) {
    (void)e;
    popup_current_mode = POPUP_CALENDAR_MODE_DAY;
    popup_update_calendar_displays();
}

static void popup_calendar_select_year_cb(lv_event_t *e) {
    (void)e;
    popup_current_mode = POPUP_CALENDAR_MODE_YEAR;
    popup_update_calendar_displays();
}

static void popup_calendar_enter_cb(lv_event_t *e) {
    (void)e;

    // Update app state with popup calendar date
    app_state_set_calendar_date(popup_calendar_date);

    // Update the main display
    update_calendar_display();

    // Save configuration
    save_theme_config();

    // Close the popup safely
    lv_obj_t *btn = lv_event_get_target(e);
    if (!btn) return;

    lv_obj_t *popup = btn;
    // Find the root popup container
    while (popup && lv_obj_get_parent(popup) && lv_obj_get_parent(popup) != lv_scr_act()) {
        popup = lv_obj_get_parent(popup);
    }
    if (popup && lv_obj_get_parent(popup) == lv_scr_act()) {
        // Reset static variables to avoid dangling pointers
        popup_calendar_display_label = NULL;
        popup_month_label = NULL;
        popup_day_label = NULL;
        popup_year_label = NULL;
        popup_month_button = NULL;
        popup_day_button = NULL;
        popup_year_button = NULL;

        lv_obj_del(popup);
    }
}

// Calendar popup close callback
static void calendar_popup_close_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        if (!btn) return;

        lv_obj_t *popup = btn;
        // Find the root popup container
        while (popup && lv_obj_get_parent(popup) && lv_obj_get_parent(popup) != lv_scr_act()) {
            popup = lv_obj_get_parent(popup);
        }
        if (popup && lv_obj_get_parent(popup) == lv_scr_act()) {
            // Reset static variables to avoid dangling pointers
            popup_calendar_display_label = NULL;
            popup_month_label = NULL;
            popup_day_label = NULL;
            popup_year_label = NULL;
            popup_month_button = NULL;
            popup_day_button = NULL;
            popup_year_button = NULL;

            lv_obj_del(popup);
        }
    }
}

// Show calendar popup when display is clicked
void show_calendar_popup(lv_event_t *e) {
    (void)e; // Unused parameter

    lv_obj_t *parent = lv_scr_act(); // Get the active screen
    if (!parent) return;

    // Initialize popup calendar with current app state date
    popup_calendar_date = app_state_get_calendar_date();
    popup_current_mode = POPUP_CALENDAR_MODE_MONTH;

    // Create popup overlay and container using helpers
    lv_obj_t *popup = create_popup_overlay(parent);
    lv_obj_t *calendar_container = create_popup_container(popup, 300, 280);

    // Title
    lv_obj_t *title_label = lv_label_create(calendar_container);
    lv_label_set_text(title_label, get_label("admin_screen.calendar_setting"));
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 10);
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(title_label, app_state_get_font_20(), 0);
    }

    // Main display area for selected date (like Korean input text area)
    lv_obj_t *calendar_display = lv_label_create(calendar_container);
    lv_obj_set_style_bg_color(calendar_display, lv_color_hex(0x333333), 0); // Dark gray like Korean input
    lv_obj_set_style_bg_opa(calendar_display, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(calendar_display, lv_color_hex(get_button_border_color()), 0);
    lv_obj_set_style_border_width(calendar_display, 2, 0);
    lv_obj_set_style_text_color(calendar_display, lv_color_white(), 0);
    lv_obj_set_style_pad_all(calendar_display, 8, 0);
    lv_obj_set_style_text_align(calendar_display, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(calendar_display, 280, 50);
    lv_obj_align(calendar_display, LV_ALIGN_TOP_MID, 0, 60);
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(calendar_display, app_state_get_font_20(), 0);
    }
    popup_calendar_display_label = calendar_display;

    // Navigation row - centered layout
    int label_width = 50;
    int label_height = 32;
    int nav_row_y_offset = 45;

    // Previous button
    lv_obj_t *prev_btn = create_nav_button(calendar_container, "<", 45, label_height, 0,
                                           popup_calendar_prev_cb, NULL);
    lv_obj_align(prev_btn, LV_ALIGN_CENTER, -110, nav_row_y_offset);

    // Month button
    lv_obj_t *month_btn = lv_btn_create(calendar_container);
    lv_obj_set_size(month_btn, label_width, label_height);
    lv_obj_align(month_btn, LV_ALIGN_CENTER, -55, nav_row_y_offset);
    apply_button_style(month_btn, app_state_get_button_color());
    lv_obj_t *month_btn_label = lv_label_create(month_btn);
    lv_obj_set_style_text_color(month_btn_label, lv_color_white(), 0);
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(month_btn_label, app_state_get_font_20(), 0);
    }
    lv_obj_center(month_btn_label);
    lv_obj_add_event_cb(month_btn, popup_calendar_select_month_cb, LV_EVENT_CLICKED, NULL);
    popup_month_label = month_btn_label;
    popup_month_button = month_btn;

    // Day button
    lv_obj_t *day_btn = lv_btn_create(calendar_container);
    lv_obj_set_size(day_btn, label_width, label_height);
    lv_obj_align(day_btn, LV_ALIGN_CENTER, 0, nav_row_y_offset);
    apply_button_style(day_btn, app_state_get_button_color());
    lv_obj_t *day_btn_label = lv_label_create(day_btn);
    lv_obj_set_style_text_color(day_btn_label, lv_color_white(), 0);
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(day_btn_label, app_state_get_font_20(), 0);
    }
    lv_obj_center(day_btn_label);
    lv_obj_add_event_cb(day_btn, popup_calendar_select_day_cb, LV_EVENT_CLICKED, NULL);
    popup_day_label = day_btn_label;
    popup_day_button = day_btn;

    // Year button
    lv_obj_t *year_btn = lv_btn_create(calendar_container);
    lv_obj_set_size(year_btn, label_width, label_height);
    lv_obj_align(year_btn, LV_ALIGN_CENTER, 55, nav_row_y_offset);
    apply_button_style(year_btn, app_state_get_button_color());
    lv_obj_t *year_btn_label = lv_label_create(year_btn);
    lv_obj_set_style_text_color(year_btn_label, lv_color_white(), 0);
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(year_btn_label, app_state_get_font_20(), 0);
    }
    lv_obj_center(year_btn_label);
    lv_obj_add_event_cb(year_btn, popup_calendar_select_year_cb, LV_EVENT_CLICKED, NULL);
    popup_year_label = year_btn_label;
    popup_year_button = year_btn;

    // Next button
    lv_obj_t *next_btn = create_nav_button(calendar_container, ">", 45, label_height, 0,
                                           popup_calendar_next_cb, NULL);
    lv_obj_align(next_btn, LV_ALIGN_CENTER, 110, nav_row_y_offset);

    // Enter button
    lv_obj_t *enter_btn = create_button_with_label(calendar_container, get_label("admin_screen.select"),
                                                   90, 32, 0, popup_calendar_enter_cb, NULL);
    lv_obj_align(enter_btn, LV_ALIGN_CENTER, 0, 105);

    // Create close button using helper
    create_close_button(calendar_container, calendar_popup_close_cb, NULL);

    // Initialize displays
    popup_update_calendar_displays();
}

// Creates the calendar display button in admin content
lv_obj_t *create_calendar_section(lv_obj_t *parent, int y_pos) {
    if (!parent) return NULL;

    // Calendar title - left aligned at 5px
    lv_obj_t *calendar_title = lv_label_create(parent);
    lv_label_set_text(calendar_title, get_label("admin_screen.calendar_setting"));
    apply_label_style(calendar_title);
    lv_obj_set_pos(calendar_title, 5, y_pos);

    // Calendar date display button - left aligned at 5px
    int calendar_btn_width = 290;
    lv_obj_t *calendar_btn = lv_btn_create(parent);
    lv_obj_set_size(calendar_btn, calendar_btn_width, 50);

    // Position at left edge
    lv_obj_set_pos(calendar_btn, 5, y_pos + 25);
    apply_button_style(calendar_btn, app_state_get_button_color());

    // Create label inside the button for the calendar text
    calendar_display_label = lv_label_create(calendar_btn);
    lv_obj_set_style_text_color(calendar_display_label, lv_color_white(), 0);
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(calendar_display_label, app_state_get_font_20(), 0);
    }
    lv_obj_center(calendar_display_label);

    // Add click event to button
    lv_obj_add_event_cb(calendar_btn, show_calendar_popup, LV_EVENT_CLICKED, NULL);

    // Initialize calendar with current date or system date
    calendar_date_t calendar_date = app_state_get_calendar_date();
    if (calendar_date.year == 0) {
        calendar_init(&calendar_date);
        app_state_set_calendar_date(calendar_date);
    }
    update_calendar_display();

    return calendar_display_label;
}
