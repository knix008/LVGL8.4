#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include "lvgl/lvgl.h"

// ============================================================================
// SLIDESHOW API
// ============================================================================

/**
 * Initialize the slideshow with images from a directory
 * @param parent_screen The parent LVGL screen object
 * @return 0 on success, -1 on failure
 */
int slideshow_init(lv_obj_t *parent_screen);

/**
 * Get the slideshow image object
 * @return Pointer to the slideshow image object
 */
lv_obj_t *slideshow_get_image(void);

/**
 * Pause the slideshow timer
 */
void slideshow_pause(void);

/**
 * Resume the slideshow timer
 */
void slideshow_resume(void);

#endif
