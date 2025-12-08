#ifndef ADMIN_CALENDAR_H
#define ADMIN_CALENDAR_H

#include "types.h"
#include "calendar.h"

// ============================================================================
// CALENDAR POPUP API
// ============================================================================

/**
 * Creates and displays a calendar popup for date selection
 * @param e Event object (can be NULL)
 */
void show_calendar_popup(lv_event_t *e);

/**
 * Creates the calendar display button in admin content
 * @param parent Parent container
 * @param y_pos Vertical position
 * @return The calendar display label for updates
 */
lv_obj_t *create_calendar_section(lv_obj_t *parent, int y_pos);

/**
 * Updates the calendar display with current date
 */
void update_calendar_display(void);

#endif
