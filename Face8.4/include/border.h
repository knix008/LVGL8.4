#ifndef BORDER_H
#define BORDER_H

#include "lvgl/lvgl.h"

// ============================================================================
// GREEN BORDER RECTANGLE API
// ============================================================================

/**
 * Shows a green rectangle border on the active screen.
 * Creates a full-screen overlay with a transparent background and green border.
 */
void show_green_border(void);

/**
 * Removes the green rectangle border from the screen.
 */
void remove_green_border(void);

#endif
