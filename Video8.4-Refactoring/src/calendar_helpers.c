#include "../include/calendar_helpers.h"
#include "../include/calendar.h"
#include "../include/label.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// CALENDAR MANAGEMENT HELPERS
// ============================================================================

void update_calendar_state_displays(calendar_state_t* state) {
    if (!state || !state->display_label) return;
    
    char date_text[64];
    char main_display_text[128];
    
    calendar_format_date_string(&state->date, date_text, sizeof(date_text));
    
    // Get day of week name
    int day_of_week = calendar_get_day_of_week(&state->date);
    const char* day_name = calendar_get_day_name(day_of_week);
    
    // Format main display with day of week
    snprintf(main_display_text, sizeof(main_display_text), "%s (%s)", date_text, day_name);
    lv_label_set_text(state->display_label, main_display_text);
    
    // Update individual labels if they exist
    if (state->month_label) {
        lv_label_set_text(state->month_label, calendar_get_month_abbr(state->date.month));
    }
    
    if (state->day_label) {
        char day_text[8];
        snprintf(day_text, sizeof(day_text), "%d", state->date.day);
        lv_label_set_text(state->day_label, day_text);
    }
    
    if (state->year_label) {
        char year_text[8];
        snprintf(year_text, sizeof(year_text), "%d", state->date.year);
        lv_label_set_text(state->year_label, year_text);
    }
    
    // Update button colors
    update_calendar_button_colors(state);
}

void update_calendar_button_colors(calendar_state_t* state) {
    if (!state) return;
    
    lv_color_t default_color = lv_color_hex(0xFF9800); // Orange
    lv_color_t selected_color = lv_color_hex(0xBF360C); // Darker orange
    
    // Update button colors based on current mode
    if (state->month_button) {
        lv_obj_set_style_bg_color(state->month_button, 
            (state->current_mode == 0) ? selected_color : default_color, 0);
    }
    if (state->day_button) {
        lv_obj_set_style_bg_color(state->day_button, 
            (state->current_mode == 1) ? selected_color : default_color, 0);
    }
    if (state->year_button) {
        lv_obj_set_style_bg_color(state->year_button, 
            (state->current_mode == 2) ? selected_color : default_color, 0);
    }
}

void calendar_handle_prev(calendar_state_t* state) {
    if (!state) return;
    
    switch (state->current_mode) {
        case 0: // Month mode
            calendar_prev_month(&state->date);
            break;
        case 1: // Day mode
            calendar_prev_day(&state->date);
            break;
        case 2: // Year mode
            calendar_prev_year(&state->date);
            break;
    }
    update_calendar_state_displays(state);
}

void calendar_handle_next(calendar_state_t* state) {
    if (!state) return;
    
    switch (state->current_mode) {
        case 0: // Month mode
            calendar_next_month(&state->date);
            break;
        case 1: // Day mode
            calendar_next_day(&state->date);
            break;
        case 2: // Year mode
            calendar_next_year(&state->date);
            break;
    }
    update_calendar_state_displays(state);
}

void calendar_set_mode(calendar_state_t* state, int mode) {
    if (!state || mode < 0 || mode > 2) return;
    
    state->current_mode = mode;
    update_calendar_state_displays(state);
}

void calendar_state_init(calendar_state_t* state) {
    if (!state) return;
    
    // Initialize with current date
    calendar_init(&state->date);
    state->current_mode = 0; // Start with month mode
    
    // Initialize all pointers to NULL
    state->display_label = NULL;
    state->month_label = NULL;
    state->day_label = NULL;
    state->year_label = NULL;
    state->month_button = NULL;
    state->day_button = NULL;
    state->year_button = NULL;
}

void calendar_state_reset(calendar_state_t* state) {
    if (!state) return;
    
    // Reset all pointers to avoid dangling references
    state->display_label = NULL;
    state->month_label = NULL;
    state->day_label = NULL;
    state->year_label = NULL;
    state->month_button = NULL;
    state->day_button = NULL;
    state->year_button = NULL;
}