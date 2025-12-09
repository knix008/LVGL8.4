#include "../include/camera.h"
#include "../include/camera_stream.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"
#include "../include/label.h"
#include "../include/socket.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// CAMERA STATE
// ============================================================================

static SocketClient *camera_socket = NULL;
static lv_obj_t *status_label = NULL;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Ensure socket connection is established
 * @return SocketClient pointer or NULL on error
 */
static SocketClient* ensure_socket_connection(void) {
    if (!camera_socket) {
        camera_socket = socket_client_create_unix("/tmp/face_recognition.sock");
    }
    return camera_socket;
}

/**
 * @brief Execute socket command and display response
 * @param cmd_func Socket command function pointer
 * @param ... Additional arguments for the command
 */
typedef int (*SocketCmdFunc)(SocketClient*, Response*);
typedef int (*SocketCmdWithStr)(SocketClient*, const char*, Response*);
typedef int (*SocketCmdWithCapture)(SocketClient*, const char*, uint64_t, Response*);

static void execute_socket_command(SocketCmdFunc cmd_func) {
    SocketClient *socket = ensure_socket_connection();
    if (!socket || !status_label) {
        return;
    }

    Response response;
    int result = cmd_func(socket, &response);
    
    if (result == 0) {
        lv_label_set_text(status_label, response.message);
    } else {
        lv_label_set_text(status_label, response.message);
    }
}

static void execute_socket_command_with_str(SocketCmdWithStr cmd_func, const char *arg) {
    SocketClient *socket = ensure_socket_connection();
    if (!socket || !status_label) {
        return;
    }

    Response response;
    cmd_func(socket, arg, &response);
    
    lv_label_set_text(status_label, response.message);
}

static void execute_socket_command_with_capture(SocketCmdWithCapture cmd_func, 
                                                 const char *initial, uint64_t id) {
    SocketClient *socket = ensure_socket_connection();
    if (!socket || !status_label) {
        return;
    }

    Response response;
    cmd_func(socket, initial, id, &response);
    
    lv_label_set_text(status_label, response.message);
}

// ============================================================================
// BUTTON CALLBACKS
// ============================================================================

static void camera_on_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        execute_socket_command(socket_client_camera_on);
        
        // Start streaming when camera turns on
        SocketClient *socket = ensure_socket_connection();
        if (socket) {
            camera_stream_start(socket);
        }
    }
}

static void camera_off_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        execute_socket_command(socket_client_camera_off);
        
        // Stop streaming when camera turns off
        camera_stream_stop();
    }
}

static void capture_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // Capture with default name "Person" and ID 1
        execute_socket_command_with_capture(socket_client_capture, "Person", 1);
    }
}

static void train_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        execute_socket_command(socket_client_train);
    }
}

static void status_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        execute_socket_command(socket_client_status);
    }
}

static void list_persons_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        execute_socket_command(socket_client_list_persons);
    }
}

static void delete_person_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // Delete with default person name "Person0" for demo
        // In a real implementation, this should prompt for a name
        execute_socket_command_with_str(socket_client_delete_person, "Person0");
    }
}

static void fas_on_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        execute_socket_command(socket_client_fas_on);
    }
}

static void fas_off_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        execute_socket_command(socket_client_fas_off);
    }
}

// ============================================================================
// UI COMPONENTS
// ============================================================================

typedef struct {
    const char *label_key;
    lv_event_cb_t callback;
} CameraButton;

static const CameraButton camera_buttons[] = {
    {"camera_screen.camera_on", camera_on_callback},
    {"camera_screen.camera_off", camera_off_callback},
    {"camera_screen.capture", capture_callback},
    {"camera_screen.training", train_callback},
    {"camera_screen.status", status_callback},
    {"camera_screen.list_persons", list_persons_callback},
    {"camera_screen.delete_person", delete_person_callback},
    {"camera_screen.fas_on", fas_on_callback},
    {"camera_screen.fas_off", fas_off_callback}
};

static void create_button_grid(lv_obj_t *parent, int start_y) {
    const int btn_width = 140;
    const int btn_height = 35;
    const int col_spacing = 10;
    const int row_spacing = 10;
    const int num_buttons = sizeof(camera_buttons) / sizeof(camera_buttons[0]);

    for (int i = 0; i < num_buttons; i++) {
        int row = i / 2;
        int col = i % 2;

        lv_obj_t *btn = lv_btn_create(parent);
        lv_obj_set_size(btn, btn_width, btn_height);

        int x_pos = col * (btn_width + col_spacing) + 10;
        int y_pos = start_y + row * (btn_height + row_spacing);

        lv_obj_set_pos(btn, x_pos, y_pos);
        apply_button_style(btn, 0);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, get_label(camera_buttons[i].label_key));
        apply_label_style(label);
        lv_obj_center(label);

        lv_obj_add_event_cb(btn, camera_buttons[i].callback, LV_EVENT_CLICKED, NULL);
    }
}

static lv_obj_t* create_status_container(lv_obj_t *parent, int y_pos, const char *title) {
    const int container_width = 280;
    
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_size(container, container_width, 100);
    lv_obj_set_pos(container, 10, y_pos);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_border_width(container, 2, 0);
    lv_obj_set_style_border_color(container, lv_color_hex(get_button_border_color()), 0);
    lv_obj_set_scroll_dir(container, LV_DIR_VER);

    lv_obj_t *label = lv_label_create(container);
    lv_label_set_text(label, title);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, container_width - 20);
    apply_label_style(label);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 10);

    return label;
}

static lv_obj_t* create_stream_container(lv_obj_t *parent, int y_pos) {
    const int container_width = 280;
    
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_size(container, container_width, 120);
    lv_obj_set_pos(container, 10, y_pos);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_border_width(container, 2, 0);
    lv_obj_set_style_border_color(container, lv_color_hex(0x00AA00), 0);
    lv_obj_set_scroll_dir(container, LV_DIR_VER);

    lv_obj_t *label = lv_label_create(container);
    lv_label_set_text(label, get_label("camera_screen.stream_not_connected"));
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, container_width - 20);
    apply_label_style(label);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 10);

    return label;
}

static lv_obj_t* create_camera_content(lv_obj_t *parent) {
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, SCREEN_WIDTH, SCREEN_HEIGHT - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, TITLE_BAR_HEIGHT);
    lv_obj_set_style_bg_color(content, lv_color_hex(get_background_color()), 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);

    const int start_y = 10;
    const int btn_height = 35;
    const int row_spacing = 10;
    const int num_buttons = sizeof(camera_buttons) / sizeof(camera_buttons[0]);

    // Create button grid
    create_button_grid(content, start_y);

    // Create status display area
    int status_y = start_y + ((num_buttons + 1) / 2) * (btn_height + row_spacing) + 10;
    status_label = create_status_container(content, status_y, get_label("camera_screen.ready"));

    // Create stream display area
    int stream_y = status_y + 110;
    lv_obj_t *stream_label_obj = create_stream_container(content, stream_y);
    camera_stream_init(stream_label_obj);

    return content;
}

// ============================================================================
// PUBLIC API
// ============================================================================

void create_camera_screen(void) {
    lv_obj_t *camera_screen = create_screen_base(SCREEN_CAMERA);

    create_standard_title_bar(camera_screen, SCREEN_CAMERA);
    create_camera_content(camera_screen);
    create_standard_status_bar(camera_screen);

    finalize_screen(camera_screen, SCREEN_CAMERA);
}

void cleanup_camera_screen(void) {
    camera_stream_cleanup();
    
    if (camera_socket) {
        socket_client_destroy(camera_socket);
        camera_socket = NULL;
    }
    status_label = NULL;
}
