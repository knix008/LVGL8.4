#ifndef BORDER_H
#define BORDER_H

#include "lvgl/lvgl.h"

// ============================================================================
// BORDER RECTANGLE API
// ============================================================================

/**
 * Shows a colored rectangle border on the active screen.
 * Creates a full-screen overlay with a transparent background and colored border.
 * @param color The color of the border (hex value, e.g., 0x00FF00 for green)
 * @param width The width of the border in pixels (default: 8)
 */
void show_border(uint32_t color, uint8_t width);

/**
 * Shows a green rectangle border (convenience function).
 */
void show_green_border(void);

/**
 * Shows a red rectangle border (convenience function).
 */
void show_red_border(void);

/**
 * Shows a blue rectangle border (convenience function).
 */
void show_blue_border(void);

/**
 * Shows a yellow rectangle border (convenience function).
 */
void show_yellow_border(void);

/**
 * Shows an orange rectangle border (convenience function).
 */
void show_orange_border(void);

/**
 * Shows a purple rectangle border (convenience function).
 */
void show_purple_border(void);

/**
 * Shows a white rectangle border (convenience function).
 */
void show_white_border(void);

/**
 * Shows a border with thin width (convenience function).
 */
void show_thin_border(uint32_t color);

/**
 * Shows a border with thick width (convenience function).
 */
void show_thick_border(uint32_t color);

/**
 * Shows a border with extra thick width (convenience function).
 */
void show_extra_thick_border(uint32_t color);

/**
 * Removes the rectangle border from the screen.
 */
void remove_border(void);

/**
 * Removes the green rectangle border (alias for backward compatibility).
 */
void remove_green_border(void);

#endif
