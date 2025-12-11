#ifndef SCREEN_H
#define SCREEN_H

#include "types.h"

// ============================================================================
// SCREEN MANAGEMENT API
// ============================================================================

extern ScreenState screen_stack[];
extern int screen_stack_top;

void show_screen(int screen_id);
void create_menu_screen(void);
void create_info_screen(void);
void create_admin_screen(void);
void create_network_screen(void);
void create_number_input_screen(void);
void update_title_bar_location(int screen_id);

// ============================================================================
// REUSABLE SCREEN COMPONENT CREATION
// ============================================================================

// Create a standard title bar with back button
lv_obj_t *create_standard_title_bar(lv_obj_t *parent, int screen_id);

// Create a standard status bar with 4 navigation buttons
lv_obj_t *create_standard_status_bar(lv_obj_t *parent);

// Create a standard content area
lv_obj_t *create_standard_content(lv_obj_t *parent);

// Create base screen object with common settings
lv_obj_t *create_screen_base(int screen_id);

// Apply standard screen initialization (stack management, load)
void finalize_screen(lv_obj_t *screen, int screen_id);

// ============================================================================
// STATUS BAR ICON MANAGEMENT
// ============================================================================

// Add an icon to the status bar
void add_status_bar_icon(int menu_index, const char *icon_path);

// Remove an icon from the status bar
void remove_status_bar_icon(int menu_index);

// Update all status bar icons positions
void update_status_bar_icons(void);

// Move status bar to a specific screen (for navigation)
void move_status_bar_to_screen(lv_obj_t *screen, int screen_id);

#endif
