#ifndef CAMERA_STREAM_H
#define CAMERA_STREAM_H

#include "lvgl/lvgl.h"
#include "socket.h"

// ============================================================================
// CAMERA STREAM API
// ============================================================================

/**
 * @brief Initialize stream module
 * @param label LVGL label object to display stream messages
 */
void camera_stream_init(lv_obj_t *label);

/**
 * @brief Start streaming from camera server
 * @param socket Socket client to use for streaming
 * @return 0 on success, -1 on error
 */
int camera_stream_start(SocketClient *socket);

/**
 * @brief Stop streaming
 */
void camera_stream_stop(void);

/**
 * @brief Check if stream is active
 * @return 1 if streaming, 0 otherwise
 */
int camera_stream_is_active(void);

/**
 * @brief Cleanup stream resources
 */
void camera_stream_cleanup(void);

#endif /* CAMERA_STREAM_H */
