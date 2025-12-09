#include "../include/camera.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"
#include "../include/label.h"
#include "../include/socket.h"
#include <stdio.h>

// ============================================================================
// CAMERA CONTROL STATE
// ============================================================================

static SocketClient *camera_socket = NULL;
static lv_obj_t *status_label = NULL;

// ============================================================================
// CAMERA BUTTON CALLBACKS
// ============================================================================

static void camera_on_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (!camera_socket) {
            camera_socket = socket_client_create_unix("/tmp/face_recognition.sock");
        }

        if (camera_socket) {
            Response response;
            int result = socket_client_camera_on(camera_socket, &response);

            if (status_label) {
                if (result == 0 && response.success) {
                    lv_label_set_text(status_label, response.message);
                } else {
                    lv_label_set_text(status_label, response.message);
                }
            }
        }
    }
}

static void camera_off_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (!camera_socket) {
            camera_socket = socket_client_create_unix("/tmp/face_recognition.sock");
        }

        if (camera_socket) {
            Response response;
            int result = socket_client_camera_off(camera_socket, &response);

            if (status_label) {
                if (result == 0 && response.success) {
                    lv_label_set_text(status_label, response.message);
                } else {
                    lv_label_set_text(status_label, response.message);
                }
            }
        }
    }
}

static void capture_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (!camera_socket) {
            camera_socket = socket_client_create_unix("/tmp/face_recognition.sock");
        }

        if (camera_socket) {
            Response response;
            // Capture with default name "Person" and ID 1
            int result = socket_client_capture(camera_socket, "Person", 1, &response);

            if (status_label) {
                if (result == 0 && response.success) {
                    lv_label_set_text(status_label, response.message);
                } else {
                    lv_label_set_text(status_label, response.message);
                }
            }
        }
    }
}

static void train_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (!camera_socket) {
            camera_socket = socket_client_create_unix("/tmp/face_recognition.sock");
        }

        if (camera_socket) {
            Response response;
            int result = socket_client_train(camera_socket, &response);

            if (status_label) {
                if (result == 0 && response.success) {
                    lv_label_set_text(status_label, response.message);
                } else {
                    lv_label_set_text(status_label, response.message);
                }
            }
        }
    }
}

static void status_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (!camera_socket) {
            camera_socket = socket_client_create_unix("/tmp/face_recognition.sock");
        }

        if (camera_socket) {
            Response response;
            int result = socket_client_status(camera_socket, &response);

            if (status_label) {
                if (result == 0 && response.success) {
                    lv_label_set_text(status_label, response.message);
                } else {
                    lv_label_set_text(status_label, response.message);
                }
            }
        }
    }
}

static void list_persons_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (!camera_socket) {
            camera_socket = socket_client_create_unix("/tmp/face_recognition.sock");
        }

        if (camera_socket) {
            Response response;
            int result = socket_client_list_persons(camera_socket, &response);

            if (status_label) {
                if (result == 0 && response.success) {
                    lv_label_set_text(status_label, response.message);
                } else {
                    lv_label_set_text(status_label, response.message);
                }
            }
        }
    }
}

static void delete_person_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (!camera_socket) {
            camera_socket = socket_client_create_unix("/tmp/face_recognition.sock");
        }

        if (camera_socket) {
            Response response;
            // Delete with default person name "Person0" for demo
            // In a real implementation, this should prompt for a name
            int result = socket_client_delete_person(camera_socket, "Person0", &response);

            if (status_label) {
                if (result == 0 && response.success) {
                    lv_label_set_text(status_label, response.message);
                } else {
                    lv_label_set_text(status_label, response.message);
                }
            }
        }
    }
}

static void fas_on_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (!camera_socket) {
            camera_socket = socket_client_create_unix("/tmp/face_recognition.sock");
        }

        if (camera_socket) {
            Response response;
            int result = socket_client_fas_on(camera_socket, &response);

            if (status_label) {
                if (result == 0 && response.success) {
                    lv_label_set_text(status_label, response.message);
                } else {
                    lv_label_set_text(status_label, response.message);
                }
            }
        }
    }
}

static void fas_off_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (!camera_socket) {
            camera_socket = socket_client_create_unix("/tmp/face_recognition.sock");
        }

        if (camera_socket) {
            Response response;
            int result = socket_client_fas_off(camera_socket, &response);

            if (status_label) {
                if (result == 0 && response.success) {
                    lv_label_set_text(status_label, response.message);
                } else {
                    lv_label_set_text(status_label, response.message);
                }
            }
        }
    }
}

// ============================================================================
// CAMERA SCREEN COMPONENTS
// ============================================================================

static lv_obj_t *create_camera_content(lv_obj_t *parent) {
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, SCREEN_WIDTH, SCREEN_HEIGHT - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, TITLE_BAR_HEIGHT);
    lv_obj_set_style_bg_color(content, lv_color_hex(get_background_color()), 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);

    // Button dimensions
    const int btn_width = 140;
    const int btn_height = 35;
    const int col_spacing = 10;
    const int row_spacing = 10;
    const int start_y = 10;

    // Create buttons in a grid layout (2 columns)
    typedef struct {
        const char *label;
        lv_event_cb_t callback;
    } ButtonDef;

    ButtonDef buttons[] = {
        {"Camera On", camera_on_callback},
        {"Camera Off", camera_off_callback},
        {"Capture", capture_callback},
        {"Training", train_callback},
        {"Status", status_callback},
        {"List Persons", list_persons_callback},
        {"Delete Person", delete_person_callback},
        {"FAS On", fas_on_callback},
        {"FAS Off", fas_off_callback}
    };

    int num_buttons = sizeof(buttons) / sizeof(buttons[0]);

    for (int i = 0; i < num_buttons; i++) {
        int row = i / 2;
        int col = i % 2;

        lv_obj_t *btn = lv_btn_create(content);
        lv_obj_set_size(btn, btn_width, btn_height);

        int x_pos = col * (btn_width + col_spacing) + 10;
        int y_pos = start_y + row * (btn_height + row_spacing);

        lv_obj_set_pos(btn, x_pos, y_pos);
        apply_button_style(btn, 0);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, buttons[i].label);
        apply_label_style(label);
        lv_obj_center(label);

        lv_obj_add_event_cb(btn, buttons[i].callback, LV_EVENT_CLICKED, NULL);
    }

    // Create status display area
    const int container_width = 280;  // Width that fits within screen
    lv_obj_t *status_container = lv_obj_create(content);
    lv_obj_set_size(status_container, container_width, 100);
    lv_obj_set_pos(status_container, 10, start_y + ((num_buttons + 1) / 2) * (btn_height + row_spacing) + 10);
    lv_obj_set_style_bg_color(status_container, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_border_width(status_container, 2, 0);
    lv_obj_set_style_border_color(status_container, lv_color_hex(get_button_border_color()), 0);
    lv_obj_set_scroll_dir(status_container, LV_DIR_VER);  // Only vertical scrolling

    status_label = lv_label_create(status_container);
    lv_label_set_text(status_label, "Ready");
    lv_label_set_long_mode(status_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(status_label, container_width - 20);  // Container width minus padding
    apply_label_style(status_label);
    lv_obj_align(status_label, LV_ALIGN_TOP_LEFT, 10, 10);

    return content;
}

// ============================================================================
// CAMERA SCREEN CREATION
// ============================================================================

/**
 * Creates the camera screen with title bar, control buttons, and status bar.
 * Provides camera control interface with buttons for all camera operations.
 */
void create_camera_screen(void) {
    lv_obj_t *camera_screen = create_screen_base(SCREEN_CAMERA);

    create_standard_title_bar(camera_screen, SCREEN_CAMERA);
    create_camera_content(camera_screen);
    create_standard_status_bar(camera_screen);

    finalize_screen(camera_screen, SCREEN_CAMERA);
}

/**
 * Cleanup camera resources when screen is destroyed
 */
void cleanup_camera_screen(void) {
    if (camera_socket) {
        socket_client_destroy(camera_socket);
        camera_socket = NULL;
    }
    status_label = NULL;
}
