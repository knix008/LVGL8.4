#include "../include/camera_stream.h"
#include "../include/state.h"
#include "../include/label.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

// ============================================================================
// STREAM STATE
// ============================================================================

static lv_obj_t *stream_label = NULL;
static lv_obj_t *parent_screen = NULL;
static lv_obj_t *popup_container = NULL;
static lv_timer_t *stream_timer = NULL;
static lv_timer_t *popup_timer = NULL;
static int stream_fd = -1;

// ============================================================================
// POPUP MANAGEMENT
// ============================================================================

static void popup_timer_callback(lv_timer_t *timer) {
    (void)timer;
    if (popup_container) {
        lv_obj_del(popup_container);
        popup_container = NULL;
    }
    if (popup_timer) {
        lv_timer_del(popup_timer);
        popup_timer = NULL;
    }
}

static void close_button_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (popup_container) {
            lv_obj_del(popup_container);
            popup_container = NULL;
        }
        if (popup_timer) {
            lv_timer_del(popup_timer);
            popup_timer = NULL;
        }
    }
}

static void show_recognition_popup(const char *name, float confidence) {
    if (!parent_screen) {
        return;
    }

    // Delete existing popup if any
    if (popup_container) {
        lv_obj_del(popup_container);
        popup_container = NULL;
    }
    if (popup_timer) {
        lv_timer_del(popup_timer);
        popup_timer = NULL;
    }

    // Create popup container (bottom aligned overlay)
    popup_container = lv_obj_create(parent_screen);
    lv_obj_set_size(popup_container, 400, 200);
    lv_obj_align(popup_container, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(popup_container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(popup_container, LV_OPA_50, 0);
    lv_obj_set_style_border_width(popup_container, 0, 0);
    lv_obj_set_style_radius(popup_container, 0, 0);
    lv_obj_set_style_shadow_width(popup_container, 0, 0);

    // Title label
    lv_obj_t *title_label = lv_label_create(popup_container);
    lv_label_set_text(title_label, get_label("camera_screen.recognized"));
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(title_label, app_state_get_font_20(), 0);
    }
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 20);

    // User ID label
    lv_obj_t *user_id_label = lv_label_create(popup_container);
    char user_id_text[150];
    snprintf(user_id_text, sizeof(user_id_text), "%s : %s", get_label("camera_screen.user_id"), name);
    lv_label_set_text(user_id_label, user_id_text);
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(user_id_label, app_state_get_font_20(), 0);
    }
    lv_obj_set_style_text_color(user_id_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(user_id_label, LV_ALIGN_CENTER, 0, -10);

    // Confidence level label
    lv_obj_t *conf_label = lv_label_create(popup_container);
    char conf_text[64];
    snprintf(conf_text, sizeof(conf_text), "%s : %.0f%%", get_label("camera_screen.confidence_level"), confidence);
    lv_label_set_text(conf_label, conf_text);
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(conf_label, app_state_get_font_20(), 0);
    }
    lv_obj_set_style_text_color(conf_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(conf_label, LV_ALIGN_CENTER, 0, 20);

    // Close button with cancel image
    lv_obj_t *close_btn = lv_btn_create(popup_container);
    lv_obj_set_size(close_btn, 40, 40);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_set_style_bg_opa(close_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(close_btn, 0, 0);
    lv_obj_set_style_shadow_width(close_btn, 0, 0);
    
    // Add cancel button image
    lv_obj_t *close_img = lv_img_create(close_btn);
    lv_img_set_src(close_img, "A:assets/images/cancel_button_40x40.png");
    lv_obj_center(close_img);
    
    // Add click event to close popup
    lv_obj_add_event_cb(close_btn, close_button_callback, LV_EVENT_CLICKED, NULL);

    // Auto-close after 3 seconds
    popup_timer = lv_timer_create(popup_timer_callback, 3000, NULL);
    lv_timer_set_repeat_count(popup_timer, 1);
}

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
        
        // Parse stream data for recognized persons
        // Format: "FACE:name:confidence:timestamp" or "NO_FACE:timestamp"
        char name[128] = {0};
        int confidence_int = 0;
        uint64_t timestamp = 0;
        
        if (sscanf(buffer, "FACE:%127[^:]:%d:%lu", name, &confidence_int, &timestamp) == 3) {
            float confidence = (float)confidence_int;
            
            // Only show popup for recognized persons (not "Unknown" or "Too far")
            if (strcmp(name, "Unknown") != 0 && strcmp(name, "Too far") != 0 && confidence >= 70.0f) {
                show_recognition_popup(name, confidence);
            }
        }
        
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

void camera_stream_init(lv_obj_t *label, lv_obj_t *parent) {
    stream_label = label;
    parent_screen = parent;
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
    
    if (popup_timer) {
        lv_timer_del(popup_timer);
        popup_timer = NULL;
    }
    
    if (popup_container) {
        lv_obj_del(popup_container);
        popup_container = NULL;
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
    parent_screen = NULL;
}
