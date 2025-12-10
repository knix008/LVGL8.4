#ifndef HOME_H
#define HOME_H

// ============================================================================
// HOME SCREEN API
// ============================================================================

void create_gui(void);

/**
 * Update home screen button labels when language changes
 */
void update_home_screen_labels(void);

/**
 * Inactivity timer control functions
 */
void start_inactivity_timer(void);
void stop_inactivity_timer(void);
void pause_inactivity_timer(void);
void resume_inactivity_timer(void);

#endif
