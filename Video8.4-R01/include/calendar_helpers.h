#ifndef CALENDAR_HELPERS_H
#define CALENDAR_HELPERS_H

#include "types.h"
#include "calendar.h"

// ============================================================================
// CALENDAR MANAGEMENT HELPERS
// ============================================================================

/**
 * Calendar state structure for managing calendar UI components
 */
typedef struct {
    calendar_date_t date;
    lv_obj_t* display_label;
    lv_obj_t* month_label;
    lv_obj_t* day_label;
    lv_obj_t* year_label;
    lv_obj_t* month_button;
    lv_obj_t* day_button;
    lv_obj_t* year_button;
    int current_mode;  // 0=month, 1=day, 2=year
} calendar_state_t;

/**
 * Update all calendar display labels and button colors
 * @param state Calendar state structure
 */
void update_calendar_state_displays(calendar_state_t* state);

/**
 * Update button colors based on current selection mode
 * @param state Calendar state structure
 */
void update_calendar_button_colors(calendar_state_t* state);

/**
 * Handle previous navigation for calendar
 * @param state Calendar state structure
 */
void calendar_handle_prev(calendar_state_t* state);

/**
 * Handle next navigation for calendar
 * @param state Calendar state structure
 */
void calendar_handle_next(calendar_state_t* state);

/**
 * Set calendar mode (0=month, 1=day, 2=year)
 * @param state Calendar state structure
 * @param mode New mode
 */
void calendar_set_mode(calendar_state_t* state, int mode);

/**
 * Initialize calendar state with current date
 * @param state Calendar state structure to initialize
 */
void calendar_state_init(calendar_state_t* state);

/**
 * Reset calendar state pointers (for safe cleanup)
 * @param state Calendar state structure to reset
 */
void calendar_state_reset(calendar_state_t* state);

#endif