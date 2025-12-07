#ifndef VIDEO_H
#define VIDEO_H

#include "lvgl/lvgl.h"

// ============================================================================
// VIDEO PLAYBACK API
// ============================================================================

/**
 * Initialize the video player with a video file from the videos directory
 * @param parent_screen The parent LVGL screen object
 * @return 0 on success, -1 on failure
 */
int video_init(lv_obj_t *parent_screen);

/**
 * Start video playback
 */
void video_start(void);

/**
 * Stop video playback
 */
void video_stop(void);

/**
 * Hide video player
 */
void video_hide(void);

/**
 * Show video player
 */
void video_show(void);

/**
 * Get the video player object
 * @return Pointer to the video player object
 */
lv_obj_t *video_get_player(void);

/**
 * Check if video is currently playing
 * @return true if playing, false otherwise
 */
bool video_is_playing(void);

#endif
