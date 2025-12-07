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

/**
 * Get current video information
 * @param index Pointer to store current video index (can be NULL)
 * @param total Pointer to store total video count (can be NULL)
 * @param path Pointer to store current video path (can be NULL)
 * @param path_size Size of path buffer
 */
void video_get_info(int *index, int *total, char *path, size_t path_size);

/**
 * Skip to next video manually
 */
void video_next(void);

/**
 * Skip to previous video manually
 */
void video_previous(void);

/**
 * Get total number of videos loaded
 * @return Number of video files loaded
 */
int video_get_count(void);

/**
 * Manually trigger transition to next video (for testing/debugging)
 */
void video_trigger_next(void);

#endif
