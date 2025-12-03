#include "../include/admin.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"
#include "../include/label.h"
#include "../include/home.h"
#include <stdio.h>

// External reference to app state
extern AppState app_state;

// ============================================================================
// COLOR SELECTION
// ============================================================================

// Helper function to recursively update button colors
static void update_buttons_recursively(lv_obj_t *obj) {
    if (!obj) return;
    
    // Check if this is a button
    if (lv_obj_check_type(obj, &lv_btn_class)) {
        void *user_data = lv_obj_get_user_data(obj);
        
        // Skip color picker buttons - they have ColorOption pointer (not 1, 2, or NULL)
        if (user_data != (void*)1 && user_data != (void*)2 && user_data != NULL) {
            // This might be a color picker button, skip it
            // (ColorOption pointers will be valid memory addresses, not 1 or 2)
            if ((uintptr_t)user_data > 0x1000) {
                return; // Skip this button, it's a color picker
            }
        }
        if (user_data != (void*)1 && user_data != (void*)2) {
            lv_obj_set_style_bg_color(obj, lv_color_hex(app_state.button_color), 0);
            lv_obj_set_style_border_color(obj, lv_color_hex(app_state.button_border_color), 0);
        }
    }
    
    // Recursively update children
    uint32_t child_count = lv_obj_get_child_cnt(obj);
    for (uint32_t i = 0; i < child_count; i++) {
        lv_obj_t *child = lv_obj_get_child(obj, i);
        update_buttons_recursively(child);
    }
}

// Color target enum
typedef enum {
    COLOR_TARGET_BACKGROUND,
    COLOR_TARGET_TITLE_BAR,
    COLOR_TARGET_STATUS_BAR,
    COLOR_TARGET_BUTTON,
    COLOR_TARGET_BUTTON_BORDER
} ColorTarget;

// Predefined color options
typedef struct {
    const char *name;
    uint32_t color;
    ColorTarget target;  // Which color this button sets
} ColorOption;

// Forward declarations
static void update_color_picker_buttons(lv_obj_t *obj, ColorTarget target);

// Event handler for color selection
static void color_button_clicked(lv_event_t *e) {
    ColorOption *option = (ColorOption *)lv_event_get_user_data(e);
    
    // Update app state based on target
    switch (option->target) {
        case COLOR_TARGET_BACKGROUND:
            app_state.bg_color = option->color;
            break;
        case COLOR_TARGET_TITLE_BAR:
            app_state.title_bar_color = option->color;
            break;
        case COLOR_TARGET_STATUS_BAR:
            app_state.status_bar_color = option->color;
            break;
        case COLOR_TARGET_BUTTON:
            app_state.button_color = option->color;
            break;
        case COLOR_TARGET_BUTTON_BORDER:
            app_state.button_border_color = option->color;
            break;
    }
    
    // Save configuration
    save_theme_config();
    
    // Update the shared status bar directly if it exists
    if (option->target == COLOR_TARGET_STATUS_BAR && app_state.status_bar) {
        lv_obj_set_style_bg_color(app_state.status_bar, lv_color_hex(app_state.status_bar_color), 0);
    }
    
    // Update the home screen title bar directly if it exists
    if (option->target == COLOR_TARGET_TITLE_BAR && app_state.title_bar) {
        lv_obj_set_style_bg_color(app_state.title_bar, lv_color_hex(app_state.title_bar_color), 0);
    }
    
    // Update all cached screens' title bars and backgrounds
    extern ScreenState screen_stack[];
    extern int screen_stack_top;
    for (int i = 0; i <= screen_stack_top; i++) {
        if (screen_stack[i].screen) {
            // Update screen background
            lv_obj_set_style_bg_color(screen_stack[i].screen, lv_color_hex(app_state.bg_color), 0);
            
            // Update all children (title bars, content, etc.)
            uint32_t child_count = lv_obj_get_child_cnt(screen_stack[i].screen);
            for (uint32_t j = 0; j < child_count; j++) {
                lv_obj_t *child = lv_obj_get_child(screen_stack[i].screen, j);
                if (child) {
                    void *user_data = lv_obj_get_user_data(child);
                    
                    if (user_data == (void*)1) {
                        // Title bar
                        lv_obj_set_style_bg_color(child, lv_color_hex(app_state.title_bar_color), 0);
                        // Update buttons within title bar
                        if (option->target == COLOR_TARGET_BUTTON || option->target == COLOR_TARGET_BUTTON_BORDER) {
                            update_buttons_recursively(child);
                        }
                    } else if (user_data == (void*)2) {
                        // Status bar (though it's shared, update it anyway)
                        lv_obj_set_style_bg_color(child, lv_color_hex(app_state.status_bar_color), 0);
                        // Update buttons within status bar
                        if (option->target == COLOR_TARGET_BUTTON || option->target == COLOR_TARGET_BUTTON_BORDER) {
                            update_buttons_recursively(child);
                        }
                    } else {
                        // Content area
                        lv_obj_set_style_bg_color(child, lv_color_hex(app_state.bg_color), 0);
                        // Update buttons recursively within content
                        if (option->target == COLOR_TARGET_BUTTON || option->target == COLOR_TARGET_BUTTON_BORDER) {
                            update_buttons_recursively(child);
                        }
                    }
                }
            }
        }
    }
    
    // Update current screen immediately
    lv_obj_t *current = lv_scr_act();
    if (current) {
        // Update background
        lv_obj_set_style_bg_color(current, lv_color_hex(app_state.bg_color), 0);
        
        // Update all children
        uint32_t child_count = lv_obj_get_child_cnt(current);
        for (uint32_t i = 0; i < child_count; i++) {
            lv_obj_t *child = lv_obj_get_child(current, i);
            if (child) {
                // Use user data to identify bars
                void *user_data = lv_obj_get_user_data(child);
                
                if (user_data == (void*)1) {
                    // Title bar (ID = 1)
                    lv_obj_set_style_bg_color(child, lv_color_hex(app_state.title_bar_color), 0);
                    // Update buttons within title bar
                    if (option->target == COLOR_TARGET_BUTTON || option->target == COLOR_TARGET_BUTTON_BORDER) {
                        update_buttons_recursively(child);
                    }
                } else if (user_data == (void*)2) {
                    // Status bar (ID = 2)
                    lv_obj_set_style_bg_color(child, lv_color_hex(app_state.status_bar_color), 0);
                    // Update buttons within status bar
                    if (option->target == COLOR_TARGET_BUTTON || option->target == COLOR_TARGET_BUTTON_BORDER) {
                        update_buttons_recursively(child);
                    }
                } else {
                    // Content area
                    lv_obj_set_style_bg_color(child, lv_color_hex(app_state.bg_color), 0);
                    // Update buttons recursively within content
                    if (option->target == COLOR_TARGET_BUTTON || option->target == COLOR_TARGET_BUTTON_BORDER) {
                        update_buttons_recursively(child);
                    }
                }
            }
        }
    }
    
    // Update color picker buttons in admin screen to show new selection
    lv_obj_t *active_screen = lv_scr_act();
    if (active_screen) {
        uint32_t child_count = lv_obj_get_child_cnt(active_screen);
        for (uint32_t i = 0; i < child_count; i++) {
            lv_obj_t *child = lv_obj_get_child(active_screen, i);
            if (child) {
                // Find content area (no user data or user_data != 1, 2)
                void *user_data = lv_obj_get_user_data(child);
                if (user_data != (void*)1 && user_data != (void*)2) {
                    // Update color picker buttons within content
                    update_color_picker_buttons(child, option->target);
                }
            }
        }
    }
}

// Helper to update color picker button borders based on current selection
static void update_color_picker_buttons(lv_obj_t *obj, ColorTarget target) {
    (void)target; // Unused - we check all colors
    if (!obj) return;
    
    uint32_t child_count = lv_obj_get_child_cnt(obj);
    for (uint32_t i = 0; i < child_count; i++) {
        lv_obj_t *child = lv_obj_get_child(obj, i);
        if (!child) continue;
        
        void *user_data = lv_obj_get_user_data(child);
        
        // If this is a color picker button (has ColorOption pointer)
        if (user_data != NULL && user_data != (void*)1 && user_data != (void*)2 && (uintptr_t)user_data > 0x1000) {
            ColorOption *opt = (ColorOption*)user_data;
            
            // Check if this button's color matches its specific target's current setting
            bool is_selected = false;
            if (opt->target == COLOR_TARGET_BACKGROUND && opt->color == app_state.bg_color) is_selected = true;
            else if (opt->target == COLOR_TARGET_TITLE_BAR && opt->color == app_state.title_bar_color) is_selected = true;
            else if (opt->target == COLOR_TARGET_STATUS_BAR && opt->color == app_state.status_bar_color) is_selected = true;
            else if (opt->target == COLOR_TARGET_BUTTON && opt->color == app_state.button_color) is_selected = true;
            else if (opt->target == COLOR_TARGET_BUTTON_BORDER && opt->color == app_state.button_border_color) is_selected = true;
            
            // Update border based on selection
            if (is_selected) {
                lv_obj_set_style_border_color(child, lv_color_hex(0x00FF00), 0);
                lv_obj_set_style_border_width(child, 4, 0);
            } else {
                lv_obj_set_style_border_color(child, lv_color_hex(0xFFFFFF), 0);
                lv_obj_set_style_border_width(child, 2, 0);
            }
            
            // Force redraw
            lv_obj_invalidate(child);
        } else {
            // Recurse into children
            update_color_picker_buttons(child, target);
        }
    }
}

// ============================================================================
// ADMIN SCREEN COMPONENTS
// ============================================================================

// Helper to create a color picker section
static void create_color_section(lv_obj_t *parent, const char *title, int y_pos, ColorTarget target) {
    // Section title
    lv_obj_t *section_label = lv_label_create(parent);
    lv_label_set_text(section_label, title);
    apply_label_style(section_label);
    lv_obj_set_pos(section_label, 10, y_pos);
    
    // Color buttons - create static array for each target
    static ColorOption bg_options[4];
    static ColorOption title_options[4];
    static ColorOption status_options[4];
    static ColorOption button_options[4];
    static ColorOption button_border_options[4];
    
    ColorOption *options;
    if (target == COLOR_TARGET_BACKGROUND) {
        bg_options[0] = (ColorOption){"어두운 회색", 0x2A2A2A, target};
        bg_options[1] = (ColorOption){"검정", 0x000000, target};
        bg_options[2] = (ColorOption){"남색", 0x1A1A40, target};
        bg_options[3] = (ColorOption){"진한 녹색", 0x1A3A1A, target};
        options = bg_options;
    } else if (target == COLOR_TARGET_TITLE_BAR) {
        title_options[0] = (ColorOption){"어두운 회색", 0x1A1A1A, target};
        title_options[1] = (ColorOption){"검정", 0x000000, target};
        title_options[2] = (ColorOption){"파랑", 0x0A0A50, target};
        title_options[3] = (ColorOption){"빨강", 0x500A0A, target};
        options = title_options;
    } else if (target == COLOR_TARGET_STATUS_BAR) {
        status_options[0] = (ColorOption){"어두운 회색", 0x1A1A1A, target};
        status_options[1] = (ColorOption){"검정", 0x000000, target};
        status_options[2] = (ColorOption){"파랑", 0x0A0A50, target};
        status_options[3] = (ColorOption){"자주색", 0x3A0A3A, target};
        options = status_options;
    } else if (target == COLOR_TARGET_BUTTON) {
        button_options[0] = (ColorOption){"진한 회색", 0x1A1A1A, target};
        button_options[1] = (ColorOption){"검정", 0x000000, target};
        button_options[2] = (ColorOption){"회색", 0x444444, target};
        button_options[3] = (ColorOption){"진한 파랑", 0x0D0D3A, target};
        options = button_options;
    } else {
        button_border_options[0] = (ColorOption){"회색", 0x888888, target};
        button_border_options[1] = (ColorOption){"흰색", 0xFFFFFF, target};
        button_border_options[2] = (ColorOption){"파랑", 0x4A4AFF, target};
        button_border_options[3] = (ColorOption){"초록", 0x4AFF4A, target};
        options = button_border_options;
    }
    
    int button_y = y_pos + 30;
    int button_width = 60;
    int button_height = 40;
    int spacing = 5;
    
    for (int i = 0; i < 4; i++) {
        lv_obj_t *btn = lv_btn_create(parent);
        lv_obj_set_size(btn, button_width, button_height);
        lv_obj_set_pos(btn, 10 + i * (button_width + spacing), button_y);
        
        // Store ColorOption pointer in user_data so we can identify target and color
        lv_obj_set_user_data(btn, (void*)&options[i]);
        
        // Set button color to preview
        lv_obj_set_style_bg_color(btn, lv_color_hex(options[i].color), 0);
        
        // Highlight if current selection
        uint32_t current_color = 0;
        if (target == COLOR_TARGET_BACKGROUND) current_color = app_state.bg_color;
        else if (target == COLOR_TARGET_TITLE_BAR) current_color = app_state.title_bar_color;
        else if (target == COLOR_TARGET_STATUS_BAR) current_color = app_state.status_bar_color;
        else if (target == COLOR_TARGET_BUTTON) current_color = app_state.button_color;
        else if (target == COLOR_TARGET_BUTTON_BORDER) current_color = app_state.button_border_color;
        
        if (current_color == options[i].color) {
            lv_obj_set_style_border_color(btn, lv_color_hex(0x00FF00), 0);
            lv_obj_set_style_border_width(btn, 4, 0);
        } else {
            lv_obj_set_style_border_color(btn, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_border_width(btn, 2, 0);
        }
        
        // Add click event
        lv_obj_add_event_cb(btn, color_button_clicked, LV_EVENT_CLICKED, (void *)&options[i]);
    }
}

// ============================================================================
// LANGUAGE SELECTION
// ============================================================================

// Forward declare for timer callback
static void refresh_admin_screen_timer_cb(lv_timer_t *timer);

// Event handler for language button clicks
static void language_button_clicked(lv_event_t *e) {
    const char *language = (const char *)lv_event_get_user_data(e);

    if (!language) return;

    // Update app state and set language
    if (set_language(language) == 0) {
        strncpy(app_state.current_language, language, 3);
        app_state.current_language[3] = '\0';

        // Save configuration
        save_theme_config();

        // Use a timer to defer screen update to avoid deleting active screen
        lv_timer_t *timer = lv_timer_create(refresh_admin_screen_timer_cb, 10, NULL);
        lv_timer_set_repeat_count(timer, 1);
    }
}

// Timer callback to refresh all screens after language change
static void refresh_admin_screen_timer_cb(lv_timer_t *timer) {
    (void)timer;

    // Get references to screen stack
    extern ScreenState screen_stack[];
    extern int screen_stack_top;

    // Update home screen button labels (it's not recreated like other screens)
    update_home_screen_labels();

    // Mark all non-main screens as invalid (set screen to NULL)
    // This forces them to be recreated with new labels when navigated to
    for (int i = 1; i <= screen_stack_top; i++) {  // Start from 1 to skip SCREEN_MAIN
        if (screen_stack[i].screen) {
            screen_stack[i].screen = NULL;
        }
    }

    // Reload the current admin screen (stay on admin screen after language change)
    show_screen(SCREEN_ADMIN);
}

// Helper to create language button
static lv_obj_t *create_language_button(lv_obj_t *parent, const char *label_text,
                                        const char *language_code, int x_pos) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 90, 40);
    lv_obj_set_pos(btn, x_pos, 505);
    apply_button_style(btn, 0);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, label_text);
    apply_label_style(label);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // Store language code as user data
    lv_obj_add_event_cb(btn, language_button_clicked, LV_EVENT_CLICKED, (void *)language_code);

    return btn;
}

// ============================================================================
// BUTTON DIMENSION SETTINGS
// ============================================================================


static lv_obj_t *create_admin_content(lv_obj_t *parent) {
    lv_obj_t *content = create_standard_content(parent);
    
    // Enable vertical scrolling with wider scrollbar
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);
    lv_obj_set_style_pad_right(content, 15, LV_PART_SCROLLBAR);
    lv_obj_set_style_width(content, 8, LV_PART_SCROLLBAR);

    // Main title
    lv_obj_t *title_label = lv_label_create(content);
    lv_label_set_text(title_label, get_label("admin_screen.title"));
    apply_label_style(title_label);
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, CONTENT_PADDING, CONTENT_PADDING);

    // Create five color sections
    create_color_section(content, get_label("admin_screen.background_color"), 50, COLOR_TARGET_BACKGROUND);
    create_color_section(content, get_label("admin_screen.title_bar_color"), 130, COLOR_TARGET_TITLE_BAR);
    create_color_section(content, get_label("admin_screen.status_bar_color"), 210, COLOR_TARGET_STATUS_BAR);
    create_color_section(content, get_label("admin_screen.button_color"), 290, COLOR_TARGET_BUTTON);
    create_color_section(content, get_label("admin_screen.button_border_color"), 370, COLOR_TARGET_BUTTON_BORDER);

    // Language Settings Section
    lv_obj_t *language_title = lv_label_create(content);
    lv_label_set_text(language_title, get_label("admin_screen.language_title"));
    apply_label_style(language_title);
    lv_obj_set_pos(language_title, 10, 470);

    // Korean button
    create_language_button(content, get_label("admin_screen.language_korean"), "ko", 10);

    // English button
    create_language_button(content, get_label("admin_screen.language_english"), "en", 110);

    // Info text at bottom
    lv_obj_t *info_label = lv_label_create(content);
    lv_label_set_long_mode(info_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(info_label, SCREEN_WIDTH - CONTENT_WIDTH_PADDING);
    lv_label_set_text(info_label, get_label("admin_screen.info_text"));
    lv_obj_set_style_text_color(info_label, lv_color_hex(0xAAAAAA), 0);
    if (app_state.font_20) {
        lv_obj_set_style_text_font(info_label, app_state.font_20, 0);
    }
    lv_obj_set_pos(info_label, CONTENT_PADDING, 550);

    return content;
}


// ============================================================================
// ADMIN SCREEN CREATION
// ============================================================================

/**
 * Creates the admin settings screen with title bar, content area, and status bar.
 * Uses the standard screen creation pattern.
 */
void create_admin_screen(void) {
    lv_obj_t *admin_screen = create_screen_base(SCREEN_ADMIN);

    create_standard_title_bar(admin_screen, SCREEN_ADMIN);
    create_admin_content(admin_screen);
    create_standard_status_bar(admin_screen);

    finalize_screen(admin_screen, SCREEN_ADMIN);
}
