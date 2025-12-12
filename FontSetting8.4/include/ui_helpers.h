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

// ============================================================================
// WARNING MESSAGE BOX - YELLOW BORDER AROUND SCREEN EDGES
// ============================================================================

/**
 * Structure to hold warning border components
 */
typedef struct {
    lv_obj_t* top;
    lv_obj_t* bottom;
    lv_obj_t* left;
    lv_obj_t* right;
} warning_border_t;

/**
 * Create a yellow warning border around the application window edges
 * @param parent Parent container (usually lv_scr_act())
 * @param message_key Language key for the warning message (optional, can be NULL)
 *                    If provided, a message label will be displayed at the top center.
 *                    The function will use get_label() to retrieve the localized message.
 * @param border_width Width of the yellow border in pixels (0 for default 8px)
 * @param auto_close_ms Auto-close timer in milliseconds (0 for no auto-close)
 * @return Pointer to warning_border_t structure containing the border objects, or NULL on failure
 */
warning_border_t* create_warning_box(lv_obj_t* parent, const char* message_key, 
                                     int border_width, uint32_t auto_close_ms);

/**
 * Structure to hold warning message box components (border + message box)
 */
typedef struct {
    warning_border_t* border;
    lv_obj_t* msgbox;
} warning_msgbox_t;

/**
 * Create a warning message box with orange border around screen edges
 * This function creates both a standard LVGL message box and an orange border around the screen.
 * Uses the existing border system (show_orange_border/remove_border) to reuse the same mechanism
 * as green and red borders.
 * @param parent Parent container (usually lv_scr_act() or NULL for screen)
 * @param title_key Language key for the message box title (e.g., "common.warning")
 * @param message_key Language key for the message box content (e.g., "menu_screen.status_bar_full")
 * @param button_texts Array of button texts (e.g., {"OK", ""} or {get_label("common.ok"), ""})
 * @param add_close_btn Whether to add a close button
 * @param border_width Width of the yellow border in pixels (ignored - uses existing border system with default width)
 * @return Pointer to warning_msgbox_t structure, or NULL on failure
 */
warning_msgbox_t* create_warning_msgbox_with_border(lv_obj_t* parent, 
                                                     const char* title_key,
                                                     const char* message_key,
                                                     const char* button_texts[],
                                                     bool add_close_btn,
                                                     int border_width);

#endif