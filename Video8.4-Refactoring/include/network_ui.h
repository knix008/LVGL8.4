#ifndef NETWORK_UI_H
#define NETWORK_UI_H

#include "lvgl/lvgl.h"
#include "cursor.h"

/**
 * Create the main network screen with all UI elements
 */
void create_network_screen(void);

/**
 * Update the IP display label on the main screen
 */
void update_ip_display_label(void);

/**
 * Update the IP input display in the popup with cursor
 */
void update_popup_ip_display(void);

/**
 * Show the IP configuration popup
 */
void show_ip_popup(void);

/**
 * Hide the IP configuration popup
 */
void hide_ip_popup(void);

/**
 * Get the cursor state object
 * @return Pointer to cursor_state_t structure
 */
cursor_state_t* get_cursor_state(void);

#endif // NETWORK_UI_H
