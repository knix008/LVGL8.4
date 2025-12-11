#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "types.h"

// ============================================================================
// NAVIGATION CALLBACK FUNCTIONS
// ============================================================================

// Common navigation callbacks used across multiple screens
void back_btn_callback(lv_event_t *e);
void info_btn_callback(lv_event_t *e);
void admin_btn_callback(lv_event_t *e);
void network_btn_callback(lv_event_t *e);
void korean_input_btn_callback(lv_event_t *e);
void settings_btn_callback(lv_event_t *e);
void number_input_btn_callback(lv_event_t *e);

#endif
