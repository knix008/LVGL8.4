# Camera Refactoring: Before & After Comparison

## 1. Button Callback Pattern

### BEFORE (Duplicated 9 times)
```c
static void camera_on_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (!camera_socket) {
            camera_socket = socket_client_create_unix("/tmp/face_recognition.sock");
        }
        if (!camera_socket) {
            return;
        }
        
        Response response;
        int result = socket_client_camera_on(camera_socket, &response);
        
        if (result == 0) {
            lv_label_set_text(status_label, response.message);
        } else {
            lv_label_set_text(status_label, response.message);
        }
        
        // Start streaming when camera turns on
        if (!stream_timer) {
            // 30+ lines of stream initialization...
        }
    }
}

static void camera_off_callback(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (!camera_socket) {
            camera_socket = socket_client_create_unix("/tmp/face_recognition.sock");
        }
        if (!camera_socket) {
            return;
        }
        
        Response response;
        int result = socket_client_camera_off(camera_socket, &response);
        
        if (result == 0) {
            lv_label_set_text(status_label, response.message);
        } else {
            lv_label_set_text(status_label, response.message);
        }
        
        // Stop streaming when camera turns off
        if (stream_timer) {
            // 15+ lines of stream cleanup...
        }
    }
}

// ... 7 more similar callbacks with ~20-50 lines each
```

### AFTER (Clean & DRY)
```c
// Data-driven button definition
static const CameraButton camera_buttons[] = {
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

// Reusable socket helper
static SocketClient* ensure_socket_connection(void) {
    if (!camera_socket) {
        camera_socket = socket_client_create_unix("/tmp/face_recognition.sock");
    }
    return camera_socket;
}

// Generic command executor
static void execute_socket_command(SocketCmdFunc cmd_func) {
    SocketClient *socket = ensure_socket_connection();
    if (!socket || !status_label) return;

    Response response;
    cmd_func(socket, &response);
    lv_label_set_text(status_label, response.message);
}

// Concise callbacks
static void camera_on_callback(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        execute_socket_command(socket_client_camera_on);
        camera_stream_start(ensure_socket_connection());
    }
}

static void camera_off_callback(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        execute_socket_command(socket_client_camera_off);
        camera_stream_stop();
    }
}

static void status_callback(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        execute_socket_command(socket_client_status);
    }
}

// ... other callbacks are equally concise (3-7 lines each)
```

**Reduction:** ~400 lines → ~80 lines (80% reduction in callback code)

---

## 2. Button Grid Creation

### BEFORE (Manual positioning)
```c
static lv_obj_t* create_camera_content(lv_obj_t *parent) {
    // ... content setup ...

    // Button 1: Camera On
    lv_obj_t *btn_camera_on = lv_btn_create(content);
    lv_obj_set_size(btn_camera_on, 140, 35);
    lv_obj_set_pos(btn_camera_on, 10, 10);
    apply_button_style(btn_camera_on, 0);
    lv_obj_t *label_on = lv_label_create(btn_camera_on);
    lv_label_set_text(label_on, "Camera On");
    apply_label_style(label_on);
    lv_obj_center(label_on);
    lv_obj_add_event_cb(btn_camera_on, camera_on_callback, LV_EVENT_CLICKED, NULL);

    // Button 2: Camera Off
    lv_obj_t *btn_camera_off = lv_btn_create(content);
    lv_obj_set_size(btn_camera_off, 140, 35);
    lv_obj_set_pos(btn_camera_off, 160, 10);
    apply_button_style(btn_camera_off, 0);
    lv_obj_t *label_off = lv_label_create(btn_camera_off);
    lv_label_set_text(label_off, "Camera Off");
    apply_label_style(label_off);
    lv_obj_center(label_off);
    lv_obj_add_event_cb(btn_camera_off, camera_off_callback, LV_EVENT_CLICKED, NULL);

    // ... 7 more buttons with similar code ...

    return content;
}
```

### AFTER (Loop-based with array)
```c
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
        lv_label_set_text(label, camera_buttons[i].label);
        apply_label_style(label);
        lv_obj_center(label);

        lv_obj_add_event_cb(btn, camera_buttons[i].callback, LV_EVENT_CLICKED, NULL);
    }
}

static lv_obj_t* create_camera_content(lv_obj_t *parent) {
    // ... content setup ...
    create_button_grid(content, start_y);
    // ... status and stream containers ...
    return content;
}
```

**Benefits:**
- Add new button: Just update `camera_buttons[]` array
- Change layout: Modify loop logic once
- Consistent styling: Applied uniformly
- No copy-paste errors

---

## 3. Stream Handling

### BEFORE (Embedded in camera.c)
```c
// In camera.c (120+ lines)

static lv_timer_t *stream_timer = NULL;
static int stream_fd = -1;

static void stream_timer_callback(lv_timer_t *timer) {
    if (stream_fd < 0 || !stream_label) return;
    
    char buffer[512];
    ssize_t bytes_read = read(stream_fd, buffer, sizeof(buffer) - 1);
    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        
        const char *current_text = lv_label_get_text(stream_label);
        char new_text[1024];
        snprintf(new_text, sizeof(new_text), "%s\n%s", current_text, buffer);
        
        // Keep only last 10 lines
        int line_count = 0;
        char *line = new_text;
        while (*line) {
            if (*line == '\n') line_count++;
            line++;
        }
        
        if (line_count > 10) {
            char *start = new_text;
            int skip_lines = line_count - 10;
            while (skip_lines > 0 && *start) {
                if (*start == '\n') skip_lines--;
                start++;
            }
            lv_label_set_text(stream_label, start);
        } else {
            lv_label_set_text(stream_label, new_text);
        }
        
        // Auto-scroll
        lv_obj_t *parent = lv_obj_get_parent(stream_label);
        if (parent) {
            lv_obj_scroll_to_y(parent, LV_COORD_MAX, LV_ANIM_OFF);
        }
    }
}

static void start_stream(SocketClient *socket) {
    if (stream_timer) return;
    
    // Get FD from socket
    stream_fd = socket_client_get_fd(socket);
    if (stream_fd < 0) return;
    
    // Make non-blocking
    int flags = fcntl(stream_fd, F_GETFL, 0);
    fcntl(stream_fd, F_SETFL, flags | O_NONBLOCK);
    
    // Create timer
    stream_timer = lv_timer_create(stream_timer_callback, 100, NULL);
}

static void stop_stream(void) {
    if (stream_timer) {
        lv_timer_del(stream_timer);
        stream_timer = NULL;
    }
    stream_fd = -1;
}

// Used in camera_on_callback and camera_off_callback
```

### AFTER (Separate module)

**include/camera_stream.h:**
```c
#ifndef CAMERA_STREAM_H
#define CAMERA_STREAM_H

#include "lvgl/lvgl.h"
#include "socket.h"
#include <stdbool.h>

void camera_stream_init(lv_obj_t *label);
void camera_stream_start(SocketClient *socket);
void camera_stream_stop(void);
bool camera_stream_is_active(void);
void camera_stream_cleanup(void);

#endif
```

**src/camera_stream.c:**
```c
// Complete implementation isolated in dedicated module
// 137 lines with full error handling, state management, etc.
```

**Usage in camera.c:**
```c
#include "camera_stream.h"

// In create_camera_content():
lv_obj_t *stream_label = create_stream_container(content, stream_y);
camera_stream_init(stream_label);

// In callbacks:
camera_stream_start(ensure_socket_connection());  // On
camera_stream_stop();                             // Off

// In cleanup:
camera_stream_cleanup();
```

**Benefits:**
- **Testable:** Can unit test stream module independently
- **Reusable:** Could use in other screens
- **Maintainable:** Changes don't affect camera.c
- **Clear ownership:** All stream logic in one place

---

## 4. UI Container Creation

### BEFORE (Inline creation)
```c
static lv_obj_t* create_camera_content(lv_obj_t *parent) {
    // ... button code ...
    
    // Status display
    lv_obj_t *status_container = lv_obj_create(content);
    lv_obj_set_size(status_container, 280, 100);
    lv_obj_set_pos(status_container, 10, 220);
    lv_obj_set_style_bg_color(status_container, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_border_width(status_container, 2, 0);
    lv_obj_set_style_border_color(status_container, lv_color_hex(get_button_border_color()), 0);
    lv_obj_set_scroll_dir(status_container, LV_DIR_VER);
    
    status_label = lv_label_create(status_container);
    lv_label_set_text(status_label, "Ready");
    lv_label_set_long_mode(status_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(status_label, 260);
    apply_label_style(status_label);
    lv_obj_align(status_label, LV_ALIGN_TOP_LEFT, 10, 10);
    
    // Stream display
    lv_obj_t *stream_container = lv_obj_create(content);
    lv_obj_set_size(stream_container, 280, 120);
    lv_obj_set_pos(stream_container, 10, 330);
    lv_obj_set_style_bg_color(stream_container, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_border_width(stream_container, 2, 0);
    lv_obj_set_style_border_color(stream_container, lv_color_hex(0x00AA00), 0);
    lv_obj_set_scroll_dir(stream_container, LV_DIR_VER);
    
    stream_label = lv_label_create(stream_container);
    lv_label_set_text(stream_label, "Stream: Not Connected");
    lv_label_set_long_mode(stream_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(stream_label, 260);
    apply_label_style(stream_label);
    lv_obj_align(stream_label, LV_ALIGN_TOP_LEFT, 10, 10);
}
```

### AFTER (Helper functions)
```c
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
    // Similar pattern for stream container
}

static lv_obj_t* create_camera_content(lv_obj_t *parent) {
    // ...
    create_button_grid(content, start_y);
    status_label = create_status_container(content, status_y, "Ready");
    lv_obj_t *stream_label = create_stream_container(content, stream_y);
    camera_stream_init(stream_label);
}
```

**Benefits:**
- **DRY:** Container creation logic reusable
- **Consistent:** Same pattern for all containers
- **Readable:** Intent clear from function names
- **Flexible:** Easy to add more containers

---

## Summary of Improvements

| Aspect | Before | After | Improvement |
|--------|--------|-------|-------------|
| **Lines of Code** | 471 | 302 | **-35%** |
| **Callback Duplication** | 9 × ~40 lines | 9 × ~5 lines | **-88%** |
| **Stream Handling** | Embedded | Separate module | **100% isolation** |
| **Socket Connection** | 9 × copy-paste | 1 helper | **-89%** |
| **Button Creation** | Manual × 9 | Loop + array | **-75%** |
| **Maintainability** | Low | High | **✓ Significant** |
| **Testability** | Difficult | Easy | **✓ Significant** |
| **Extensibility** | Hard | Simple | **✓ Significant** |

## Key Takeaways

1. **Data-driven design** eliminates repetition
2. **Helper functions** reduce coupling
3. **Module extraction** improves organization
4. **Clear APIs** enhance usability
5. **Consistent patterns** ease maintenance
