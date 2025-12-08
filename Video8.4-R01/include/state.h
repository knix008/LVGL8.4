#ifndef APP_STATE_H
#define APP_STATE_H

#include "types.h"
#include "calendar.h"
#include "lvgl/lvgl.h"
#include <stdbool.h>

// ============================================================================
// STATE MANAGEMENT API
// ============================================================================
// This module encapsulates the global application state and provides
// controlled access through accessor functions. This eliminates the need
// for 'extern AppState app_state' declarations throughout the codebase.

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * @brief Initialize the application state with default values
 * @return 0 on success, -1 on failure
 */
int app_state_init(void);

/**
 * @brief Cleanup and free application state resources
 */
void app_state_cleanup(void);

// ============================================================================
// SCREEN MANAGEMENT
// ============================================================================

lv_obj_t* app_state_get_screen(void);
void app_state_set_screen(lv_obj_t *screen);

// ============================================================================
// UI ELEMENTS
// ============================================================================

lv_obj_t* app_state_get_title_bar(void);
void app_state_set_title_bar(lv_obj_t *title_bar);

lv_obj_t* app_state_get_status_bar(void);
void app_state_set_status_bar(lv_obj_t *status_bar);

lv_obj_t* app_state_get_title_label(void);
void app_state_set_title_label(lv_obj_t *label);

lv_obj_t* app_state_get_current_title_label(void);
void app_state_set_current_title_label(lv_obj_t *label);

lv_obj_t* app_state_get_welcome_label(void);
void app_state_set_welcome_label(lv_obj_t *label);

lv_obj_t* app_state_get_menu_button_label(void);
void app_state_set_menu_button_label(lv_obj_t *label);

lv_obj_t* app_state_get_exit_button_label(void);
void app_state_set_exit_button_label(lv_obj_t *label);

// ============================================================================
// FONT MANAGEMENT
// ============================================================================

lv_font_t* app_state_get_font_20(void);
void app_state_set_font_20(lv_font_t *font);

lv_font_t* app_state_get_font_button(void);
void app_state_set_font_button(lv_font_t *font);

lv_font_t* app_state_get_font_24_bold(void);
void app_state_set_font_24_bold(lv_font_t *font);

// ============================================================================
// COLOR MANAGEMENT
// ============================================================================

uint32_t app_state_get_bg_color(void);
void app_state_set_bg_color(uint32_t color);

uint32_t app_state_get_title_bar_color(void);
void app_state_set_title_bar_color(uint32_t color);

uint32_t app_state_get_status_bar_color(void);
void app_state_set_status_bar_color(uint32_t color);

uint32_t app_state_get_button_color(void);
void app_state_set_button_color(uint32_t color);

uint32_t app_state_get_button_border_color(void);
void app_state_set_button_border_color(uint32_t color);

// ============================================================================
// LANGUAGE MANAGEMENT
// ============================================================================

const char* app_state_get_language(void);
void app_state_set_language(const char *lang);

// ============================================================================
// FONT CONFIGURATION
// ============================================================================

int app_state_get_font_size_title_bar(void);
void app_state_set_font_size_title_bar(int size);

int app_state_get_font_size_label(void);
void app_state_set_font_size_label(int size);

int app_state_get_font_size_button_label(void);
void app_state_set_font_size_button_label(int size);

int app_state_get_font_size_bold(void);
void app_state_set_font_size_bold(int size);

const char* app_state_get_font_name_title(void);
void app_state_set_font_name_title(const char *name);

const char* app_state_get_font_name_status_bar(void);
void app_state_set_font_name_status_bar(const char *name);

const char* app_state_get_font_name_button_label(void);
void app_state_set_font_name_button_label(const char *name);

// ============================================================================
// MENU ITEM SELECTION
// ============================================================================

bool app_state_is_menu_item_selected(int index);
void app_state_set_menu_item_selected(int index, bool selected);
int app_state_get_menu_item_order(int index);
void app_state_set_menu_item_order(int index, int order);

lv_obj_t* app_state_get_status_icon(int index);
void app_state_set_status_icon(int index, lv_obj_t *icon);

// ============================================================================
// CALENDAR DATE
// ============================================================================

calendar_date_t app_state_get_calendar_date(void);
void app_state_set_calendar_date(calendar_date_t date);

// ============================================================================
// DIRECT STATE ACCESS (for init.c only)
// ============================================================================

/**
 * @brief Get direct pointer to app state (use sparingly, only in init.c)
 * @return Pointer to internal app state
 * @warning This function should only be used in init.c for initialization.
 *          All other modules should use the accessor functions above.
 */
AppState* app_state_get_internal(void);

#endif // APP_STATE_H
