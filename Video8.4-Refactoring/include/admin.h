#ifndef ADMIN_H
#define ADMIN_H

#include "types.h"
#include "calendar.h"

// ============================================================================
// ADMIN SETTINGS SCREEN API
// ============================================================================

/**
 * Creates the admin settings screen with title bar, content area, and status bar.
 * This is the main coordinator function that delegates to specialized modules:
 * - admin_colors: Color picker functionality
 * - admin_calendar: Calendar date selection
 * - admin_language: Language switching
 * - admin_fonts: Font configuration (future)
 */
void create_admin_screen(void);

/**
 * Shows the calendar popup for date selection
 * @param e Event object (can be NULL)
 */
void show_calendar_popup(lv_event_t *e);

#endif
