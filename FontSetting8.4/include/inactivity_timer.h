#ifndef INACTIVITY_TIMER_H
#define INACTIVITY_TIMER_H

#include <lvgl/lvgl.h>
#include <stdbool.h>

// ============================================================================
// INACTIVITY TIMER API
// ============================================================================

/**
 * Timer context for managing different inactivity behaviors
 */
typedef enum {
    INACTIVITY_CONTEXT_HOME,      // Home screen: shows video after timeout
    INACTIVITY_CONTEXT_NON_HOME   // Non-home screens: returns to home after timeout
} InactivityContext;

/**
 * Initializes the inactivity timer system
 */
void inactivity_timer_init(void);

/**
 * Starts the inactivity timer for a specific context
 * @param context The context to start the timer for
 */
void inactivity_timer_start(InactivityContext context);

/**
 * Stops the inactivity timer for a specific context
 * @param context The context to stop the timer for
 */
void inactivity_timer_stop(InactivityContext context);

/**
 * Pauses the inactivity timer for a specific context
 * @param context The context to pause the timer for
 */
void inactivity_timer_pause(InactivityContext context);

/**
 * Resumes the inactivity timer for a specific context
 * @param context The context to resume the timer for
 */
void inactivity_timer_resume(InactivityContext context);

/**
 * Resets the inactivity timer for a specific context (updates last activity time)
 * @param context The context to reset the timer for
 */
void inactivity_timer_reset(InactivityContext context);

/**
 * Activity event callback for home screen
 * Resets home timer and stops video if playing
 * @param e The LVGL event
 */
void inactivity_home_activity_cb(lv_event_t *e);

/**
 * Activity event callback for non-home screens
 * Resets non-home timer or returns to previous screen if on home
 * @param e The LVGL event
 */
void inactivity_non_home_activity_cb(lv_event_t *e);

/**
 * Sets the last active screen ID (for returning from home screen)
 * @param screen_id The screen ID to remember
 */
void inactivity_timer_set_last_screen(int screen_id);

/**
 * Gets the last active screen ID
 * @return The last active screen ID
 */
int inactivity_timer_get_last_screen(void);

#endif
