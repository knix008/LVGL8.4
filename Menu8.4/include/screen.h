#ifndef SCREEN_H
#define SCREEN_H

#include "types.h"

// ============================================================================
// SCREEN MANAGEMENT API
// ============================================================================

extern AppState app_state;
extern ScreenState screen_stack[];
extern int screen_stack_top;

void show_screen(int screen_id);
void create_menu_screen(void);
void create_info_screen(void);
void create_admin_screen(void);
void create_network_screen(void);
void update_title_bar_location(int screen_id);

#endif
