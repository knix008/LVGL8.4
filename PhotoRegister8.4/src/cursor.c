/**
 * @file cursor.c
 * @brief Implementation of reusable cursor blinking utilities
 */

#include "../include/cursor.h"

// Cursor blink interval in milliseconds
#define CURSOR_BLINK_INTERVAL_MS 500

/**
 * @brief Internal timer callback for cursor blinking
 */
static void cursor_blink_timer_callback(lv_timer_t *timer) {
    cursor_state_t *state = (cursor_state_t *)timer->user_data;
    if (!state) return;

    // Toggle cursor visibility
    state->visible = !state->visible;

    // Call the update callback to refresh display
    if (state->update_cb) {
        state->update_cb();
    }
}

void cursor_state_init(cursor_state_t *state, cursor_update_callback_t update_callback) {
    if (!state) return;

    state->visible = true;
    state->timer = NULL;
    state->update_cb = update_callback;
}

void cursor_start_blinking(cursor_state_t *state) {
    if (!state) return;

    // Stop existing timer if any
    if (state->timer) {
        lv_timer_del(state->timer);
        state->timer = NULL;
    }

    // Reset cursor to visible
    state->visible = true;

    // Create new blink timer
    state->timer = lv_timer_create(cursor_blink_timer_callback,
                                   CURSOR_BLINK_INTERVAL_MS,
                                   state);
}

void cursor_stop_blinking(cursor_state_t *state) {
    if (!state) return;

    // Delete timer if it exists
    if (state->timer) {
        lv_timer_del(state->timer);
        state->timer = NULL;
    }

    // Reset cursor to visible
    state->visible = true;
}

bool cursor_is_visible(const cursor_state_t *state) {
    return state ? state->visible : true;
}

void cursor_state_cleanup(cursor_state_t *state) {
    if (!state) return;

    // Stop blinking and cleanup timer
    cursor_stop_blinking(state);
}
