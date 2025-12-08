#ifndef ADMIN_LANGUAGE_H
#define ADMIN_LANGUAGE_H

#include "types.h"

// ============================================================================
// LANGUAGE SWITCHING API
// ============================================================================

/**
 * Creates language selection buttons
 * @param parent Parent container
 * @param y_pos Vertical position for the language section
 */
void create_language_section(lv_obj_t *parent, int y_pos);

#endif
