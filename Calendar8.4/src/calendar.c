#include "../include/calendar.h"
#include "lvgl/lvgl.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Global variables for calendar GUI
static lv_obj_t * calendar_display_label = NULL;
static lv_obj_t * month_label = NULL;
static lv_obj_t * day_label = NULL;
static lv_obj_t * year_label = NULL;
static lv_obj_t * prev_button = NULL;
static lv_obj_t * next_button = NULL;
static lv_obj_t * month_button = NULL;
static lv_obj_t * day_button = NULL;
static lv_obj_t * year_button = NULL;
static calendar_date_t current_date;

// Forward declarations for helper functions
static int calendar_is_leap_year(int year);
static int calendar_get_days_in_month(int month, int year);

// Calendar selection mode
typedef enum {
    CALENDAR_MODE_MONTH,
    CALENDAR_MODE_DAY,
    CALENDAR_MODE_YEAR
} calendar_mode_t;

static calendar_mode_t current_mode = CALENDAR_MODE_MONTH;

// Helper function to update button colors based on selection
static void update_button_colors() {
    // Default color for unselected month/day/year buttons
    lv_color_t default_color = lv_color_hex(0xFF9800); // Orange
    // Selected color
    lv_color_t selected_color = lv_color_hex(0xBF360C); // Much darker orange
    // Update month button color
    lv_obj_set_style_bg_color(month_button, 
        (current_mode == CALENDAR_MODE_MONTH) ? selected_color : default_color, 0);
    // Update day button color
    lv_obj_set_style_bg_color(day_button, 
        (current_mode == CALENDAR_MODE_DAY) ? selected_color : default_color, 0);
    // Update year button color
    lv_obj_set_style_bg_color(year_button, 
        (current_mode == CALENDAR_MODE_YEAR) ? selected_color : default_color, 0);
}

// Helper function to update all displays
static void update_calendar_displays() {
    char date_text[64];
    char main_display_text[128];
    char month_text[16];
    char day_text[8];
    char year_text[8];
    
    calendar_format_date_string(&current_date, date_text, sizeof(date_text));
    
    // Get day of week name
    int day_of_week = calendar_get_day_of_week(&current_date);
    const char* day_name = calendar_get_day_name(day_of_week);
    
    // Format main display with day of week
    snprintf(main_display_text, sizeof(main_display_text), "%s (%s)", date_text, day_name);
    lv_label_set_text(calendar_display_label, main_display_text);
    
    // Update individual labels
    snprintf(month_text, sizeof(month_text), "%s", calendar_get_month_name(current_date.month));
    snprintf(day_text, sizeof(day_text), "%d", current_date.day);
    snprintf(year_text, sizeof(year_text), "%d", current_date.year);
    
    lv_label_set_text(month_label, month_text);
    lv_label_set_text(day_label, day_text);
    lv_label_set_text(year_label, year_text);
    
    // Update button colors
    update_button_colors();
}

// Unified prev/next button callbacks
void calendar_prev_cb(lv_event_t * e) {
    (void)e; // Unused parameter
    switch (current_mode) {
        case CALENDAR_MODE_MONTH:
            calendar_prev_month(&current_date);
            break;
        case CALENDAR_MODE_DAY:
            calendar_prev_day(&current_date);
            break;
        case CALENDAR_MODE_YEAR:
            calendar_prev_year(&current_date);
            break;
    }
    update_calendar_displays();
}

void calendar_next_cb(lv_event_t * e) {
    (void)e; // Unused parameter
    switch (current_mode) {
        case CALENDAR_MODE_MONTH:
            calendar_next_month(&current_date);
            break;
        case CALENDAR_MODE_DAY:
            calendar_next_day(&current_date);
            break;
        case CALENDAR_MODE_YEAR:
            calendar_next_year(&current_date);
            break;
    }
    update_calendar_displays();
}

// Mode selection callbacks
void calendar_select_month_cb(lv_event_t * e) {
    (void)e; // Unused parameter
    current_mode = CALENDAR_MODE_MONTH;
    update_calendar_displays();
}

void calendar_select_day_cb(lv_event_t * e) {
    (void)e; // Unused parameter
    current_mode = CALENDAR_MODE_DAY;
    update_calendar_displays();
}

void calendar_select_year_cb(lv_event_t * e) {
    (void)e; // Unused parameter
    current_mode = CALENDAR_MODE_YEAR;
    update_calendar_displays();
}

// Popup dialog close callback
void calendar_close_dialog_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // Close the dialog
        lv_obj_t * btn = lv_event_get_target(e);
        lv_obj_t * dialog = lv_obj_get_parent(btn);
        lv_obj_del(dialog);
    }
}

void calendar_enter_cb(lv_event_t * e) {
    // Get current display text
    const char* current_text = lv_label_get_text(calendar_display_label);
    
    // Create popup dialog to show the result
    lv_obj_t * parent = lv_obj_get_parent(lv_event_get_target(e));
    lv_obj_t * dialog = lv_obj_create(parent);
    lv_obj_set_size(dialog, 250, 150);
    lv_obj_align(dialog, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(dialog, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(dialog, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(dialog, 2, 0);
    lv_obj_set_style_border_color(dialog, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(dialog, 10, 0);
    
    // Title label
    lv_obj_t * title_label = lv_label_create(dialog);
    lv_label_set_text(title_label, "Calendar Selection Result");
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 0);
    
    // Result text label
    lv_obj_t * popup_result_label = lv_label_create(dialog);
    lv_obj_set_style_text_font(popup_result_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(popup_result_label, lv_color_hex(0x00AA00), 0); // Green text
    lv_obj_align(popup_result_label, LV_ALIGN_CENTER, 0, -5);
    
    char popup_text[300];
    snprintf(popup_text, sizeof(popup_text), "Selected Date:\n%s", current_text);
    lv_label_set_text(popup_result_label, popup_text);
    
    // Close button
    lv_obj_t * close_btn = lv_btn_create(dialog);
    lv_obj_set_size(close_btn, 80, 40);
    lv_obj_align(close_btn, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_t * close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, "Close");
    lv_obj_center(close_label);
    
    // Close button callback
    lv_obj_add_event_cb(close_btn, calendar_close_dialog_cb, LV_EVENT_CLICKED, NULL);
    
    printf("Calendar: Enter 버튼 클릭 - Selected date: %s\n", current_text);
}

// Create Calendar tab
void create_calendar_tab(lv_obj_t * parent) {
    // Initialize calendar with current date
    calendar_init(&current_date);
    
    // Tab: Calendar Input
    lv_obj_t * calendar_label = lv_label_create(parent);
    lv_label_set_text(calendar_label, "Calendar Input");
    lv_obj_align(calendar_label, LV_ALIGN_TOP_MID, 0, 10);

    // Main display area for selected date
    lv_obj_t * calendar_display = lv_label_create(parent);
    lv_obj_set_style_bg_color(calendar_display, lv_color_hex(0x00FF00), 0); // Green background like other tabs
    lv_obj_set_style_bg_opa(calendar_display, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_main_stop(calendar_display, 0, 0);
    lv_obj_set_style_bg_grad_color(calendar_display, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_bg_grad_dir(calendar_display, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_border_color(calendar_display, lv_color_make(128, 128, 128), 0);
    lv_obj_set_style_border_width(calendar_display, 3, 0);
    lv_obj_set_style_pad_all(calendar_display, 15, 0);
    lv_obj_set_size(calendar_display, 400, 60);
    lv_obj_align(calendar_display, LV_ALIGN_TOP_MID, 0, 40);
    calendar_display_label = calendar_display;

    // First row: prev button + month + day + year + next button
    int row_y = -100;
    int label_width = 60;
    int label_height = 40;
    int spacing = 5;  // Reduced spacing from 10 to 5
    
    // Calculate total width and starting position
    int total_width = 60 + label_width + spacing + label_width + spacing + label_width + 60; // prev + month + day + year + next
    int start_x = -total_width / 2 + 30;
    
    // Prev button
    lv_obj_t * prev_btn = lv_btn_create(parent);
    lv_obj_set_size(prev_btn, 60, label_height);
    lv_obj_align(prev_btn, LV_ALIGN_CENTER, start_x, row_y);
    lv_obj_set_style_bg_color(prev_btn, lv_color_hex(0x2196F3), 0); // Blue
    lv_obj_set_style_bg_opa(prev_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(prev_btn, lv_color_white(), 0);
    lv_obj_t * prev_label = lv_label_create(prev_btn);
    lv_label_set_text(prev_label, "<");
    lv_obj_center(prev_label);
    lv_obj_add_event_cb(prev_btn, calendar_prev_cb, LV_EVENT_CLICKED, NULL);
    prev_button = prev_btn;
    // Month label (clickable) - Orange
    lv_obj_t * month_btn = lv_btn_create(parent);
    lv_obj_set_size(month_btn, label_width, label_height);
    lv_obj_align(month_btn, LV_ALIGN_CENTER, start_x + 60 + spacing, row_y);
    lv_obj_set_style_bg_color(month_btn, lv_color_hex(0xFF9800), 0); // Orange
    lv_obj_set_style_bg_opa(month_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(month_btn, lv_color_white(), 0); // White text
    lv_obj_t * month_btn_label = lv_label_create(month_btn);
    lv_label_set_text(month_btn_label, calendar_get_month_name(current_date.month));
    lv_obj_center(month_btn_label);
    lv_obj_add_event_cb(month_btn, calendar_select_month_cb, LV_EVENT_CLICKED, NULL);
    month_label = month_btn_label;
    month_button = month_btn;
    // Day label (clickable) - Orange
    lv_obj_t * day_btn = lv_btn_create(parent);
    lv_obj_set_size(day_btn, label_width, label_height);
    lv_obj_align(day_btn, LV_ALIGN_CENTER, start_x + 60 + spacing + label_width + spacing, row_y);
    lv_obj_set_style_bg_color(day_btn, lv_color_hex(0xFF9800), 0); // Orange
    lv_obj_set_style_bg_opa(day_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(day_btn, lv_color_white(), 0); // White text
    lv_obj_t * day_btn_label = lv_label_create(day_btn);
    char day_text[8];
    snprintf(day_text, sizeof(day_text), "%d", current_date.day);
    lv_label_set_text(day_btn_label, day_text);
    lv_obj_center(day_btn_label);
    lv_obj_add_event_cb(day_btn, calendar_select_day_cb, LV_EVENT_CLICKED, NULL);
    day_label = day_btn_label;
    day_button = day_btn;
    // Year label (clickable) - Orange
    lv_obj_t * year_btn = lv_btn_create(parent);
    lv_obj_set_size(year_btn, label_width, label_height);
    lv_obj_align(year_btn, LV_ALIGN_CENTER, start_x + 60 + spacing + (label_width + spacing) * 2, row_y);
    lv_obj_set_style_bg_color(year_btn, lv_color_hex(0xFF9800), 0); // Orange
    lv_obj_set_style_bg_opa(year_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(year_btn, lv_color_white(), 0); // White text
    lv_obj_t * year_btn_label = lv_label_create(year_btn);
    char year_text[8];
    snprintf(year_text, sizeof(year_text), "%d", current_date.year);
    lv_label_set_text(year_btn_label, year_text);
    lv_obj_center(year_btn_label);
    lv_obj_add_event_cb(year_btn, calendar_select_year_cb, LV_EVENT_CLICKED, NULL);
    year_label = year_btn_label;
    year_button = year_btn;
    // Next button
    lv_obj_t * next_btn = lv_btn_create(parent);
    lv_obj_set_size(next_btn, 60, label_height);
    lv_obj_align(next_btn, LV_ALIGN_CENTER, start_x + 60 + spacing + (label_width + spacing) * 3, row_y);
    lv_obj_set_style_bg_color(next_btn, lv_color_hex(0x2196F3), 0); // Blue
    lv_obj_set_style_bg_opa(next_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(next_btn, lv_color_white(), 0);
    lv_obj_t * next_label = lv_label_create(next_btn);
    lv_label_set_text(next_label, ">");
    lv_obj_center(next_label);
    lv_obj_add_event_cb(next_btn, calendar_next_cb, LV_EVENT_CLICKED, NULL);
    next_button = next_btn;
    // Second row: Enter button
    lv_obj_t * enter_btn = lv_btn_create(parent);
    lv_obj_set_size(enter_btn, 120, 40);
    lv_obj_align(enter_btn, LV_ALIGN_CENTER, 5, row_y + label_height + 10);  // Center aligned vertically
    lv_obj_set_style_bg_color(enter_btn, lv_color_hex(0x2196F3), 0); // Blue
    lv_obj_set_style_bg_opa(enter_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(enter_btn, lv_color_white(), 0);
    lv_obj_t * enter_label = lv_label_create(enter_btn);
    lv_label_set_text(enter_label, "Enter");
    lv_obj_center(enter_label);
    lv_obj_add_event_cb(enter_btn, calendar_enter_cb, LV_EVENT_CLICKED, NULL);

    // Initialize displays
    update_calendar_displays();
}

// ============================================================================
// CALENDAR LOGIC FUNCTIONS IMPLEMENTATION
// ============================================================================

// Initialize calendar with current system date
void calendar_init(calendar_date_t* date) {
    if (!date) return;
    
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    date->year = tm->tm_year + 1900;
    date->month = tm->tm_mon + 1;
    date->day = tm->tm_mday;
}

// Set specific date
void calendar_set_date(calendar_date_t* date, int year, int month, int day) {
    if (!date) return;
    date->year = year;
    date->month = month;
    date->day = day;
}

// Navigation functions
void calendar_prev_month(calendar_date_t* date) {
    if (!date) return;
    
    date->month--;
    if (date->month < 1) {
        date->month = 12;
        date->year--;
    }
    
    // Adjust day if it's invalid for new month
    int days_in_month = calendar_get_days_in_month(date->month, date->year);
    if (date->day > days_in_month) {
        date->day = days_in_month;
    }
}

void calendar_next_month(calendar_date_t* date) {
    if (!date) return;
    
    date->month++;
    if (date->month > 12) {
        date->month = 1;
        date->year++;
    }
    
    // Adjust day if it's invalid for new month
    int days_in_month = calendar_get_days_in_month(date->month, date->year);
    if (date->day > days_in_month) {
        date->day = days_in_month;
    }
}

void calendar_prev_year(calendar_date_t* date) {
    if (!date) return;
    
    date->year--;
    
    // Handle leap year adjustment for Feb 29
    if (date->month == 2 && date->day == 29 && !calendar_is_leap_year(date->year)) {
        date->day = 28;
    }
}

void calendar_next_year(calendar_date_t* date) {
    if (!date) return;
    
    date->year++;
    
    // Handle leap year adjustment for Feb 29
    if (date->month == 2 && date->day == 29 && !calendar_is_leap_year(date->year)) {
        date->day = 28;
    }
}

void calendar_prev_day(calendar_date_t* date) {
    if (!date) return;
    
    date->day--;
    if (date->day < 1) {
        // Go to previous month
        calendar_prev_month(date);
        date->day = calendar_get_days_in_month(date->month, date->year);
    }
}

void calendar_next_day(calendar_date_t* date) {
    if (!date) return;
    
    int days_in_month = calendar_get_days_in_month(date->month, date->year);
    date->day++;
    if (date->day > days_in_month) {
        // Go to next month
        date->day = 1;
        calendar_next_month(date);
    }
}

// Helper functions
static int calendar_is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int calendar_get_days_in_month(int month, int year) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    if (month < 1 || month > 12) return 30;
    
    if (month == 2 && calendar_is_leap_year(year)) {
        return 29;
    }
    
    return days[month - 1];
}

// Get day of week (0 = Sunday, 1 = Monday, ..., 6 = Saturday)
int calendar_get_day_of_week(const calendar_date_t* date) {
    if (!date) return 0;
    
    int year = date->year;
    int month = date->month;
    int day = date->day;
    
    // Zeller's congruence algorithm
    if (month < 3) {
        month += 12;
        year--;
    }
    
    int k = year % 100;
    int j = year / 100;
    
    int h = (day + ((13 * (month + 1)) / 5) + k + (k / 4) + (j / 4) - 2 * j) % 7;
    
    // Convert to standard format (0 = Sunday)
    return (h + 5) % 7;
}

// Get month name
const char* calendar_get_month_name(int month) {
    static const char* months[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    
    if (month < 1 || month > 12) return "Unknown";
    return months[month - 1];
}

// Get abbreviated month name (JAN, FEB, etc.)
const char* calendar_get_month_abbr(int month) {
    static const char* months_abbr[] = {
        "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
        "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
    };
    
    if (month < 1 || month > 12) return "???";
    return months_abbr[month - 1];
}

// Get day name
const char* calendar_get_day_name(int day_of_week) {
    static const char* days[] = {
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
    };
    
    if (day_of_week < 0 || day_of_week > 6) return "Unknown";
    return days[day_of_week];
}

// Format date string
void calendar_format_date_string(const calendar_date_t* date, char* buffer, size_t buffer_size) {
    if (!date || !buffer || buffer_size == 0) return;
    
    snprintf(buffer, buffer_size, "%04d-%02d-%02d", date->year, date->month, date->day);
} 