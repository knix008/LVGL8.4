/**
 * @file cursor_utils.h
 * @brief Reusable cursor blinking utilities for text input fields
 *
 * Provides a unified API for managing blinking cursors in text input displays.
 * Eliminates code duplication between network.c and korean.c.
 */

#ifndef CURSOR_UTILS_H
#define CURSOR_UTILS_H

#include <stdbool.h>
#include "lvgl.h"

/**
 * @brief Cursor blink callback function type
 *
 * This function is called every time the cursor blink state changes.
 * It should update the display to show/hide the cursor.
 */
typedef void (*cursor_update_callback_t)(void);

/**
 * @brief Cursor state management structure
 */
typedef struct {
    bool visible;                           // Current cursor visibility state
    lv_timer_t *timer;                      // LVGL timer for cursor blinking
    cursor_update_callback_t update_cb;     // Callback to update display
} cursor_state_t;

/**
 * @brief Initialize a cursor state structure
 *
 * @param state Pointer to cursor state structure
 * @param update_callback Function to call when cursor state changes
 */
void cursor_state_init(cursor_state_t *state, cursor_update_callback_t update_callback);

/**
 * @brief Start the cursor blinking timer
 *
 * @param state Pointer to cursor state structure
 */
void cursor_start_blinking(cursor_state_t *state);

/**
 * @brief Stop the cursor blinking timer
 *
 * @param state Pointer to cursor state structure
 */
void cursor_stop_blinking(cursor_state_t *state);

/**
 * @brief Check if cursor is currently visible
 *
 * @param state Pointer to cursor state structure
 * @return true if cursor should be visible, false otherwise
 */
bool cursor_is_visible(const cursor_state_t *state);

/**
 * @brief Cleanup cursor state resources
 *
 * @param state Pointer to cursor state structure
 */
void cursor_state_cleanup(cursor_state_t *state);

#endif // CURSOR_UTILS_H
