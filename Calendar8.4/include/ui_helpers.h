#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#include "types.h"
#include "style.h"

// ============================================================================
// BUTTON CREATION HELPERS
// ============================================================================

/**
 * Create a standard button with label
 * @param parent Parent container
 * @param text Button label text
 * @param width Button width
 * @param height Button height
 * @param bg_color Button background color (0 for default)
 * @param callback Event callback function
 * @param user_data User data for callback
 * @return Created button object
 */
lv_obj_t* create_button_with_label(lv_obj_t* parent, const char* text, 
                                   int width, int height, uint32_t bg_color,
                                   lv_event_cb_t callback, void* user_data);

/**
 * Create a close button with cancel image (Korean input style)
 * @param parent Parent container
 * @param callback Event callback function
 * @param user_data User data for callback
 * @return Created close button object
 */
lv_obj_t* create_close_button(lv_obj_t* parent, lv_event_cb_t callback, void* user_data);

/**
 * Create a navigation button (< or >)
 * @param parent Parent container
 * @param text Button text ("&lt;" or "&gt;")
 * @param width Button width
 * @param height Button height
 * @param bg_color Button background color
 * @param callback Event callback function
 * @param user_data User data for callback
 * @return Created navigation button object
 */
lv_obj_t* create_nav_button(lv_obj_t* parent, const char* text, 
                            int width, int height, uint32_t bg_color,
                            lv_event_cb_t callback, void* user_data);

// ============================================================================
// POPUP CREATION HELPERS
// ============================================================================

/**
 * Create a full-screen overlay popup container
 * @param parent Parent container (usually lv_scr_act())
 * @return Created overlay container
 */
lv_obj_t* create_popup_overlay(lv_obj_t* parent);

/**
 * Create a centered popup container within an overlay
 * @param overlay_parent The overlay container
 * @param width Container width
 * @param height Container height
 * @return Created popup container
 */
lv_obj_t* create_popup_container(lv_obj_t* overlay_parent, int width, int height);

// ============================================================================
// CALENDAR HELPERS
// ============================================================================

/**
 * Calendar button configuration structure
 */
typedef struct {
    int width;
    int height;
    int x_offset;
    int y_offset;
    uint32_t bg_color;
    lv_event_cb_t callback;
    void* user_data;
} calendar_button_config_t;

/**
 * Create a calendar navigation row with prev, month, day, year, next buttons
 * @param parent Parent container
 * @param config Array of 5 calendar_button_config_t for each button
 * @param labels Array to store label objects (can be NULL)
 * @param buttons Array to store button objects (can be NULL)
 */
void create_calendar_nav_row(lv_obj_t* parent, const calendar_button_config_t config[5],
                             lv_obj_t* labels[5], lv_obj_t* buttons[5]);

// ============================================================================
// LABEL CREATION HELPERS  
// ============================================================================

/**
 * Create a styled label with text
 * @param parent Parent container
 * @param text Label text
 * @param use_font Whether to apply app_state font
 * @return Created label object
 */
lv_obj_t* create_styled_label(lv_obj_t* parent, const char* text, bool use_font);

/**
 * Create a title label with standard styling
 * @param parent Parent container
 * @param text Title text
 * @return Created title label object
 */
lv_obj_t* create_title_label(lv_obj_t* parent, const char* text);

#endif