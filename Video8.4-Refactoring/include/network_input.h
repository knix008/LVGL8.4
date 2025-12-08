#ifndef NETWORK_INPUT_H
#define NETWORK_INPUT_H

#include "lvgl/lvgl.h"
#include "network_ip_config.h"

/**
 * Input state management structure
 */
typedef struct {
    char temp_ipv4[16];      // Temporary buffer for IPv4 input
    char temp_ipv6[40];      // Temporary buffer for IPv6 input
    int cursor_pos;          // Current cursor position in input
} input_state_t;

/**
 * Get pointer to the input state
 * @return Pointer to input_state_t structure
 */
input_state_t* get_input_state(void);

/**
 * Initialize input state with current IP configuration
 */
void init_input_state(void);

/**
 * Reset cursor position based on current IP type
 */
void reset_cursor_position(void);

/**
 * Get current cursor position
 * @return Current cursor position
 */
int get_cursor_position(void);

/**
 * Set cursor position (with bounds checking)
 * @param pos New cursor position
 */
void set_cursor_position(int pos);

/**
 * Event callback for number/hex button clicks
 * @param e Event data
 */
void number_btn_callback(lv_event_t *e);

/**
 * Event callback for dot (IPv4) or colon (IPv6) button
 * @param e Event data
 */
void dot_colon_callback(lv_event_t *e);

/**
 * Event callback for backspace button
 * @param e Event data
 */
void backspace_callback(lv_event_t *e);

/**
 * Event callback for clear all button
 * @param e Event data
 */
void clear_all_callback(lv_event_t *e);

/**
 * Event callback for IP type toggle switch (IPv4/IPv6)
 * @param e Event data
 */
void ip_type_toggle_callback(lv_event_t *e);

/**
 * Event callback for save button
 * @param e Event data
 */
void save_ip_callback(lv_event_t *e);

/**
 * Event callback for cancel button
 * @param e Event data
 */
void cancel_btn_callback(lv_event_t *e);

/**
 * Event callback for IP edit button
 * @param e Event data
 */
void ip_edit_btn_callback(lv_event_t *e);

#endif // NETWORK_INPUT_H
