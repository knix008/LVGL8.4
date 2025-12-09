#include "../include/camera_stream.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

// ============================================================================
// STREAM STATE
// ============================================================================

static lv_obj_t *stream_label = NULL;
static lv_timer_t *stream_timer = NULL;
static int stream_fd = -1;

// ============================================================================
// STREAM TIMER CALLBACK
// ============================================================================

static void stream_timer_callback(lv_timer_t *timer) {
    (void)timer;
    
    if (stream_fd < 0 || !stream_label) {
        return;
    }

    // Read stream data (non-blocking)
    char buffer[512];
    ssize_t bytes_read = read(stream_fd, buffer, sizeof(buffer) - 1);
    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        
        // Update stream label with new data
        const char *current_text = lv_label_get_text(stream_label);
        char new_text[1024];
        snprintf(new_text, sizeof(new_text), "%s\n%s", current_text, buffer);
        
        // Keep only last 10 lines
        int line_count = 0;
        char *line = new_text;
        char *last_lines_start = new_text;
        while (*line) {
            if (*line == '\n') {
                line_count++;
                if (line_count > 10) {
                    last_lines_start = line + 1;
                }
            }
            line++;
        }
        
        lv_label_set_text(stream_label, last_lines_start);
    } else if (bytes_read == 0) {
        // Connection closed
        lv_label_set_text(stream_label, "Stream: Connection closed");
        camera_stream_stop();
    } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
        // Error occurred
        lv_label_set_text(stream_label, "Stream: Error reading");
        camera_stream_stop();
    }
}

// ============================================================================
// PUBLIC API
// ============================================================================

void camera_stream_init(lv_obj_t *label) {
    stream_label = label;
}

int camera_stream_start(SocketClient *socket) {
    if (stream_fd >= 0) {
        // Already streaming
        return 0;
    }

    if (!socket) {
        return -1;
    }

    stream_fd = socket_client_stream_recognition(socket);
    if (stream_fd >= 0) {
        // Set non-blocking mode
        int flags = fcntl(stream_fd, F_GETFL, 0);
        fcntl(stream_fd, F_SETFL, flags | O_NONBLOCK);
        
        if (stream_label) {
            lv_label_set_text(stream_label, "Stream: Connected");
        }
        
        // Start timer to read stream (100ms interval)
        if (!stream_timer) {
            stream_timer = lv_timer_create(stream_timer_callback, 100, NULL);
        }
        return 0;
    } else {
        if (stream_label) {
            lv_label_set_text(stream_label, "Stream: Failed to connect");
        }
        return -1;
    }
}

void camera_stream_stop(void) {
    if (stream_timer) {
        lv_timer_del(stream_timer);
        stream_timer = NULL;
    }
    
    if (stream_fd >= 0) {
        close(stream_fd);
        stream_fd = -1;
    }
    
    if (stream_label) {
        lv_label_set_text(stream_label, "Stream: Stopped");
    }
}

int camera_stream_is_active(void) {
    return stream_fd >= 0 ? 1 : 0;
}

void camera_stream_cleanup(void) {
    camera_stream_stop();
    stream_label = NULL;
}
