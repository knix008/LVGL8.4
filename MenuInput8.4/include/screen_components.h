#ifndef SCREEN_COMPONENTS_H
#define SCREEN_COMPONENTS_H

#include "types.h"

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

#endif
