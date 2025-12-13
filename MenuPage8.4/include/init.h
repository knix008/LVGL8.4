#ifndef INIT_H
#define INIT_H

#include "lvgl/lvgl.h"

// ============================================================================
// INITIALIZATION API
// ============================================================================

int init_sdl(void);
int init_lvgl(void);
int init_fonts(void);

// ============================================================================
// FONT RELOADING API
// ============================================================================

int reload_title_font(void);
int reload_status_bar_font(void);
int reload_button_font(void);
int reload_label_font(void);
int reload_home_contents_font(void);

// ============================================================================
// UI UPDATE API
// ============================================================================

void update_title_bar_fonts(void);
void update_status_bar_fonts(void);
void update_button_fonts(void);
void update_label_fonts(void);
void update_home_contents_fonts(void);

#endif
