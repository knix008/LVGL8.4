#ifndef ADMIN_COLORS_H
#define ADMIN_COLORS_H

#include "types.h"

// ============================================================================
// COLOR CONFIGURATION API
// ============================================================================

// Color target enum
typedef enum {
    COLOR_TARGET_BACKGROUND,
    COLOR_TARGET_TITLE_BAR,
    COLOR_TARGET_STATUS_BAR,
    COLOR_TARGET_BUTTON,
    COLOR_TARGET_BUTTON_BORDER
} ColorTarget;

// Predefined color options
typedef struct {
    const char *name;
    uint32_t color;
    ColorTarget target;  // Which color this button sets
} ColorOption;

/**
 * Creates a color picker section with buttons for color selection
 * @param parent Parent container for the color section
 * @param title Title text for the section
 * @param y_pos Vertical position of the section
 * @param target Which color target this section controls
 */
void create_color_section(lv_obj_t *parent, const char *title, int y_pos, ColorTarget target);

/**
 * Updates button colors recursively in the UI hierarchy
 * @param obj Root object to update
 */
void update_buttons_recursively(lv_obj_t *obj);

#endif
