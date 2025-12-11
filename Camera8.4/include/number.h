/**
 * @file number_screen.h
 * @brief Number input screen with numeric keypad
 *
 * Provides a dedicated screen for number input with a large,
 * centered numeric keypad and save/cancel functionality.
 */

#ifndef NUMBER_SCREEN_H
#define NUMBER_SCREEN_H

#include "types.h"

// ============================================================================
// NUMBER SCREEN API
// ============================================================================

/**
 * @brief Create the number input screen
 *
 * Creates a screen with a text input display and a numeric keypad popup.
 * The keypad includes numbers 0-9, clear, backspace, and save/cancel buttons.
 */
void create_number_screen(void);

/**
 * @brief Show the number input popup
 *
 * Displays the numeric keypad popup overlay on the current screen.
 * User can enter numbers and save or cancel the input.
 */
void show_number_popup(void);

/**
 * @brief Close the number input popup
 *
 * Closes and cleans up the numeric keypad popup.
 */
void close_number_popup(void);

/**
 * @brief Get the current number input value
 *
 * @return Pointer to the current number string
 */
const char* get_number_input(void);

/**
 * @brief Set the number input value
 *
 * @param value The number string to set
 */
void set_number_input(const char *value);

#endif // NUMBER_SCREEN_H
