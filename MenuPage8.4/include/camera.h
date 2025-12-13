#ifndef CAMERA_H
#define CAMERA_H

#include "lvgl/lvgl.h"

// ============================================================================
// CAMERA SCREEN API
// ============================================================================

/**
 * @brief Create camera control screen with UI buttons
 *
 * Creates a screen with buttons for camera operations:
 * - Camera On/Off
 * - Capture
 * - Training
 * - Status
 * - List Persons
 * - Delete Person
 * - FAS On/Off
 */
void create_camera_screen(void);

/**
 * @brief Cleanup camera screen resources
 *
 * Destroys socket connections and clears references
 */
void cleanup_camera_screen(void);

#endif /* CAMERA_H */
