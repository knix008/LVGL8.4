#include <lvgl/lvgl.h>
// Forward declarations to fix implicit warnings
#include "../include/state.h"
#include "../include/config.h"
#include "../include/init.h"
#include "../include/inactivity_timer.h"
// Font names array
static const char* font_names[9] = {
    "NotoSansKR-Black.ttf",
    "NotoSansKR-Bold.ttf",
    "NotoSansKR-ExtraBold.ttf",
    "NotoSansKR-ExtraLight.ttf",
    "NotoSansKR-Light.ttf",
    "NotoSansKR-Medium.ttf",
    "NotoSansKR-Regular.ttf",
    "NotoSansKR-SemiBold.ttf",
    "NotoSansKR-Thin.ttf"
};

// Font sizes array
static int font_sizes[] = {12, 14, 16, 18, 20, 24, 28, 32};


// Forward declarations for navigation callbacks
static void admin_prev_page_callback(lv_event_t *e);
static void admin_next_page_callback(lv_event_t *e);

// ============================================================================
// FONT DROPDOWN CALLBACK CONFIGURATION
// ============================================================================

// Font target enum for generic callback handling
typedef enum {
    FONT_TARGET_TITLE,
    FONT_TARGET_STATUS_BAR,
    FONT_TARGET_BUTTON,
    FONT_TARGET_LABEL,
    FONT_TARGET_HOME_CONTENTS
} FontTarget;

// Font attribute type (name vs size)
typedef enum {
    FONT_ATTR_NAME,
    FONT_ATTR_SIZE
} FontAttribute;

// Configuration structure for font dropdown callbacks
typedef struct {
    FontTarget target;
    FontAttribute attribute;
} FontDropdownConfig;

/**
 * Generic font dropdown callback handler.
 * Handles both font name and font size selections for all font targets.
 *
 * @param e Event containing dropdown selection
 */
static void font_dropdown_event_cb(lv_event_t *e) {
    FontDropdownConfig *config = (FontDropdownConfig *)lv_event_get_user_data(e);
    if (!config) return;

    // Reset inactivity timer on user interaction
    inactivity_timer_reset(INACTIVITY_CONTEXT_NON_HOME);

    lv_obj_t *dropdown = lv_event_get_target(e);
    uint16_t idx = lv_dropdown_get_selected(dropdown);

    // Validate index based on attribute type
    bool valid = (config->attribute == FONT_ATTR_NAME && idx < 9) ||
                 (config->attribute == FONT_ATTR_SIZE && idx < 8);

    if (!valid) return;

    // Update app state based on target and attribute
    if (config->attribute == FONT_ATTR_NAME) {
        switch (config->target) {
            case FONT_TARGET_TITLE:
                app_state_set_font_name_title(font_names[idx]);
                break;
            case FONT_TARGET_STATUS_BAR:
                app_state_set_font_name_status_bar(font_names[idx]);
                break;
            case FONT_TARGET_BUTTON:
                app_state_set_font_name_button_label(font_names[idx]);
                break;
            case FONT_TARGET_LABEL:
                app_state_set_font_name_label(font_names[idx]);
                break;
            case FONT_TARGET_HOME_CONTENTS:
                app_state_set_font_name_home_contents(font_names[idx]);
                break;
        }
    } else {
        switch (config->target) {
            case FONT_TARGET_TITLE:
                app_state_set_font_size_title_bar(font_sizes[idx]);
                break;
            case FONT_TARGET_STATUS_BAR:
                app_state_set_font_size_status_bar(font_sizes[idx]);
                break;
            case FONT_TARGET_BUTTON:
                app_state_set_font_size_button_label(font_sizes[idx]);
                break;
            case FONT_TARGET_LABEL:
                app_state_set_font_size_label(font_sizes[idx]);
                break;
            case FONT_TARGET_HOME_CONTENTS:
                app_state_set_font_size_home_contents(font_sizes[idx]);
                break;
        }
    }

    // Save configuration
    save_font_config();

    // Reload font and update UI based on target
    int reload_result = -1;
    switch (config->target) {
        case FONT_TARGET_TITLE:
            reload_result = reload_title_font();
            if (reload_result == 0) update_title_bar_fonts();
            break;
        case FONT_TARGET_STATUS_BAR:
            reload_result = reload_status_bar_font();
            if (reload_result == 0) update_status_bar_fonts();
            break;
        case FONT_TARGET_BUTTON:
            reload_result = reload_button_font();
            if (reload_result == 0) update_button_fonts();
            break;
        case FONT_TARGET_LABEL:
            reload_result = reload_label_font();
            if (reload_result == 0) update_label_fonts();
            break;
        case FONT_TARGET_HOME_CONTENTS:
            reload_result = reload_home_contents_font();
            if (reload_result == 0) update_home_contents_fonts();
            break;
    }
}

// Static configuration instances for each dropdown
static FontDropdownConfig config_title_name = {FONT_TARGET_TITLE, FONT_ATTR_NAME};
static FontDropdownConfig config_title_size = {FONT_TARGET_TITLE, FONT_ATTR_SIZE};
static FontDropdownConfig config_status_name = {FONT_TARGET_STATUS_BAR, FONT_ATTR_NAME};
static FontDropdownConfig config_status_size = {FONT_TARGET_STATUS_BAR, FONT_ATTR_SIZE};
static FontDropdownConfig config_button_name = {FONT_TARGET_BUTTON, FONT_ATTR_NAME};
static FontDropdownConfig config_button_size = {FONT_TARGET_BUTTON, FONT_ATTR_SIZE};
static FontDropdownConfig config_label_name = {FONT_TARGET_LABEL, FONT_ATTR_NAME};
static FontDropdownConfig config_label_size = {FONT_TARGET_LABEL, FONT_ATTR_SIZE};
static FontDropdownConfig config_home_name = {FONT_TARGET_HOME_CONTENTS, FONT_ATTR_NAME};
static FontDropdownConfig config_home_size = {FONT_TARGET_HOME_CONTENTS, FONT_ATTR_SIZE};

#include "../include/admin.h"
#include "../include/config.h"
#include "../include/state.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"
#include "../include/label.h"
#include "../include/home.h"
#include "../include/welcome.h"
#include "../include/calendar.h"
#include "../include/ui_helpers.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// External reference to app state

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
            lv_obj_set_style_bg_color(obj, lv_color_hex(app_state_get_button_color()), 0);
            lv_obj_set_style_border_color(obj, lv_color_hex(app_state_get_button_border_color()), 0);
        }
    }
    
    // Recursively update children
    uint32_t child_count = lv_obj_get_child_cnt(obj);
    for (uint32_t i = 0; i < child_count; i++) {
        lv_obj_t *child = lv_obj_get_child(obj, i);
        update_buttons_recursively(child);
    }
}

// Helper to recursively update label text colors
static void update_labels_recursively(lv_obj_t *obj) {
    if (!obj) return;
    
    // Check if this is a label object
    if (lv_obj_check_type(obj, &lv_label_class)) {
        lv_obj_set_style_text_color(obj, lv_color_hex(app_state_get_label_text_color()), 0);
    }
    
    // Recursively update children
    uint32_t child_count = lv_obj_get_child_cnt(obj);
    for (uint32_t i = 0; i < child_count; i++) {
        lv_obj_t *child = lv_obj_get_child(obj, i);
        update_labels_recursively(child);
    }
}

// Color target enum
typedef enum {
    COLOR_TARGET_BACKGROUND,
    COLOR_TARGET_TITLE_BAR,
    COLOR_TARGET_STATUS_BAR,
    COLOR_TARGET_BUTTON,
    COLOR_TARGET_BUTTON_BORDER,
    COLOR_TARGET_LABEL_TEXT
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

    // Reset inactivity timer on user interaction
    inactivity_timer_reset(INACTIVITY_CONTEXT_NON_HOME);

    // Update app state based on target
    switch (option->target) {
        case COLOR_TARGET_BACKGROUND:
            app_state_set_bg_color(option->color);
            break;
        case COLOR_TARGET_TITLE_BAR:
            app_state_set_title_bar_color(option->color);
            break;
        case COLOR_TARGET_STATUS_BAR:
            app_state_set_status_bar_color(option->color);
            break;
        case COLOR_TARGET_BUTTON:
            app_state_set_button_color(option->color);
            break;
        case COLOR_TARGET_BUTTON_BORDER:
            app_state_set_button_border_color(option->color);
            break;
        case COLOR_TARGET_LABEL_TEXT:
            app_state_set_label_text_color(option->color);
            break;
    }
    
    // Save configuration
    save_theme_config();
    
    // Update the shared status bar directly if it exists
    if (option->target == COLOR_TARGET_STATUS_BAR && app_state_get_status_bar()) {
        lv_obj_set_style_bg_color(app_state_get_status_bar(), lv_color_hex(app_state_get_status_bar_color()), 0);
    }
    
    // Update the home screen title bar directly if it exists
    if (option->target == COLOR_TARGET_TITLE_BAR && app_state_get_title_bar()) {
        lv_obj_set_style_bg_color(app_state_get_title_bar(), lv_color_hex(app_state_get_title_bar_color()), 0);
    }
    
    // Update all cached screens' title bars and backgrounds
    extern ScreenState screen_stack[];
    extern int screen_stack_top;
    for (int i = 0; i <= screen_stack_top; i++) {
        if (screen_stack[i].screen) {
            // Update screen background
            lv_obj_set_style_bg_color(screen_stack[i].screen, lv_color_hex(app_state_get_bg_color()), 0);
            
            // Update all children (title bars, content, etc.)
            uint32_t child_count = lv_obj_get_child_cnt(screen_stack[i].screen);
            for (uint32_t j = 0; j < child_count; j++) {
                lv_obj_t *child = lv_obj_get_child(screen_stack[i].screen, j);
                if (child) {
                    void *user_data = lv_obj_get_user_data(child);
                    
                    if (user_data == (void*)1) {
                        // Title bar
                        lv_obj_set_style_bg_color(child, lv_color_hex(app_state_get_title_bar_color()), 0);
                        // Update buttons within title bar
                        if (option->target == COLOR_TARGET_BUTTON || option->target == COLOR_TARGET_BUTTON_BORDER) {
                            update_buttons_recursively(child);
                        }
                    } else if (user_data == (void*)2) {
                        // Status bar (though it's shared, update it anyway)
                        lv_obj_set_style_bg_color(child, lv_color_hex(app_state_get_status_bar_color()), 0);
                        // Update buttons within status bar
                        if (option->target == COLOR_TARGET_BUTTON || option->target == COLOR_TARGET_BUTTON_BORDER) {
                            update_buttons_recursively(child);
                        }
                    } else {
                        // Content area
                        lv_obj_set_style_bg_color(child, lv_color_hex(app_state_get_bg_color()), 0);
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
        lv_obj_set_style_bg_color(current, lv_color_hex(app_state_get_bg_color()), 0);
        
        // Update all children
        uint32_t child_count = lv_obj_get_child_cnt(current);
        for (uint32_t i = 0; i < child_count; i++) {
            lv_obj_t *child = lv_obj_get_child(current, i);
            if (child) {
                // Use user data to identify bars
                void *user_data = lv_obj_get_user_data(child);
                
                if (user_data == (void*)1) {
                    // Title bar (ID = 1)
                    lv_obj_set_style_bg_color(child, lv_color_hex(app_state_get_title_bar_color()), 0);
                    // Update buttons within title bar
                    if (option->target == COLOR_TARGET_BUTTON || option->target == COLOR_TARGET_BUTTON_BORDER) {
                        update_buttons_recursively(child);
                    }
                } else if (user_data == (void*)2) {
                    // Status bar (ID = 2)
                    lv_obj_set_style_bg_color(child, lv_color_hex(app_state_get_status_bar_color()), 0);
                    // Update buttons within status bar
                    if (option->target == COLOR_TARGET_BUTTON || option->target == COLOR_TARGET_BUTTON_BORDER) {
                        update_buttons_recursively(child);
                    }
                } else {
                    // Content area
                    lv_obj_set_style_bg_color(child, lv_color_hex(app_state_get_bg_color()), 0);
                    // Update buttons recursively within content
                    if (option->target == COLOR_TARGET_BUTTON || option->target == COLOR_TARGET_BUTTON_BORDER) {
                        update_buttons_recursively(child);
                    }
                    // Update labels recursively if label text color changed
                    if (option->target == COLOR_TARGET_LABEL_TEXT) {
                        update_labels_recursively(child);
                    }
                }
                // Update labels in title bar and status bar too
                if (option->target == COLOR_TARGET_LABEL_TEXT) {
                    update_labels_recursively(child);
                }
            }
        }
    }
    
    // Update labels on all screens if label text color changed
    if (option->target == COLOR_TARGET_LABEL_TEXT) {
        extern ScreenState screen_stack[];
        extern int screen_stack_top;
        for (int i = 0; i <= screen_stack_top; i++) {
            if (screen_stack[i].screen) {
                update_labels_recursively(screen_stack[i].screen);
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
        // Force redraw of the entire screen
        lv_obj_invalidate(active_screen);
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
            if (opt->target == COLOR_TARGET_BACKGROUND && opt->color == app_state_get_bg_color()) is_selected = true;
            else if (opt->target == COLOR_TARGET_TITLE_BAR && opt->color == app_state_get_title_bar_color()) is_selected = true;
            else if (opt->target == COLOR_TARGET_STATUS_BAR && opt->color == app_state_get_status_bar_color()) is_selected = true;
            else if (opt->target == COLOR_TARGET_BUTTON && opt->color == app_state_get_button_color()) is_selected = true;
            else if (opt->target == COLOR_TARGET_BUTTON_BORDER && opt->color == app_state_get_button_border_color()) is_selected = true;
            else if (opt->target == COLOR_TARGET_LABEL_TEXT && opt->color == app_state_get_label_text_color()) is_selected = true;
            
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
    static ColorOption label_text_options[4];
    
    ColorOption *options;
    if (target == COLOR_TARGET_BACKGROUND) {
        bg_options[0] = (ColorOption){"어두운 회색", 0x2A2A2A, target};
        bg_options[1] = (ColorOption){"검정", 0x000000, target};
        bg_options[2] = (ColorOption){"흰색", 0xFFFFFF, target};
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
    } else if (target == COLOR_TARGET_LABEL_TEXT) {
        label_text_options[0] = (ColorOption){"흰색", 0xFFFFFF, target};
        label_text_options[1] = (ColorOption){"검정", 0x000000, target};
        label_text_options[2] = (ColorOption){"회색", 0x888888, target};
        label_text_options[3] = (ColorOption){"파랑", 0x4A4AFF, target};
        options = label_text_options;
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
        if (target == COLOR_TARGET_BACKGROUND) current_color = app_state_get_bg_color();
        else if (target == COLOR_TARGET_TITLE_BAR) current_color = app_state_get_title_bar_color();
        else if (target == COLOR_TARGET_STATUS_BAR) current_color = app_state_get_status_bar_color();
        else if (target == COLOR_TARGET_BUTTON) current_color = app_state_get_button_color();
        else if (target == COLOR_TARGET_BUTTON_BORDER) current_color = app_state_get_button_border_color();
        else if (target == COLOR_TARGET_LABEL_TEXT) current_color = app_state_get_label_text_color();
        
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

    // Reset inactivity timer on user interaction
    inactivity_timer_reset(INACTIVITY_CONTEXT_NON_HOME);

    // Update app state and set language
    if (set_language(language) == 0) {
        app_state_set_language(language);

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
                                        const char *language_code, int x_pos, int y_pos) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 90, 40);
    lv_obj_set_pos(btn, x_pos, y_pos);
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
// CALENDAR FUNCTIONALITY
// ============================================================================

static lv_obj_t *calendar_display_label = NULL;

// Helper to update calendar display
static void update_calendar_display(void) {
    if (!calendar_display_label) return;
    
    char date_text[64];
    char display_text[128];
    
    calendar_date_t calendar_date = app_state_get_calendar_date();
    calendar_format_date_string(&calendar_date, date_text, sizeof(date_text));
    
    // Get day of week name
    int day_of_week = calendar_get_day_of_week(&calendar_date);
    const char* day_name = calendar_get_day_name(day_of_week);
    
    // Format display with day of week
    snprintf(display_text, sizeof(display_text), "%s (%s)", date_text, day_name);
    lv_label_set_text(calendar_display_label, display_text);
    
    // Save configuration
    save_theme_config();
}



// ============================================================================
// CALENDAR POPUP FUNCTIONALITY
// ============================================================================

// Static variables for popup calendar
static calendar_date_t popup_calendar_date;
static lv_obj_t *popup_calendar_display_label = NULL;
static lv_obj_t *popup_month_label = NULL;
static lv_obj_t *popup_day_label = NULL;
static lv_obj_t *popup_year_label = NULL;
static lv_obj_t *popup_month_button = NULL;
static lv_obj_t *popup_day_button = NULL;
static lv_obj_t *popup_year_button = NULL;

// Calendar selection mode for popup
typedef enum {
    POPUP_CALENDAR_MODE_MONTH,
    POPUP_CALENDAR_MODE_DAY,
    POPUP_CALENDAR_MODE_YEAR
} popup_calendar_mode_t;

static popup_calendar_mode_t popup_current_mode = POPUP_CALENDAR_MODE_MONTH;

// Calendar popup close callback
static void calendar_popup_close_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *popup = btn;
        // Find the root popup container
        while (popup && lv_obj_get_parent(popup) && lv_obj_get_parent(popup) != lv_scr_act()) {
            popup = lv_obj_get_parent(popup);
        }
        if (popup && lv_obj_get_parent(popup) == lv_scr_act()) {
            // Reset static variables to avoid dangling pointers
            popup_calendar_display_label = NULL;
            popup_month_label = NULL;
            popup_day_label = NULL;
            popup_year_label = NULL;
            popup_month_button = NULL;
            popup_day_button = NULL;
            popup_year_button = NULL;
            
            lv_obj_del(popup);
        }
    }
}



// Helper function to update popup button colors based on selection
static void popup_update_button_colors(void) {
    lv_color_t default_color = lv_color_hex(0xFF9800); // Orange
    lv_color_t selected_color = lv_color_hex(0xBF360C); // Darker orange
    
    // Update button colors based on current mode
    if (popup_month_button) {
        lv_obj_set_style_bg_color(popup_month_button, 
            (popup_current_mode == POPUP_CALENDAR_MODE_MONTH) ? selected_color : default_color, 0);
    }
    if (popup_day_button) {
        lv_obj_set_style_bg_color(popup_day_button, 
            (popup_current_mode == POPUP_CALENDAR_MODE_DAY) ? selected_color : default_color, 0);
    }
    if (popup_year_button) {
        lv_obj_set_style_bg_color(popup_year_button, 
            (popup_current_mode == POPUP_CALENDAR_MODE_YEAR) ? selected_color : default_color, 0);
    }
}

// Helper function to update popup displays
static void popup_update_calendar_displays(void) {
    if (!popup_calendar_display_label) return;
    
    char date_text[64];
    char main_display_text[128];
    
    calendar_format_date_string(&popup_calendar_date, date_text, sizeof(date_text));
    
    // Get day of week name
    int day_of_week = calendar_get_day_of_week(&popup_calendar_date);
    const char* day_name = calendar_get_day_name(day_of_week);
    
    // Format main display with day of week
    snprintf(main_display_text, sizeof(main_display_text), "%s (%s)", date_text, day_name);
    lv_label_set_text(popup_calendar_display_label, main_display_text);
    
    // Update individual labels
    if (popup_month_label) {
        lv_label_set_text(popup_month_label, calendar_get_month_abbr(popup_calendar_date.month));
    }
    if (popup_day_label) {
        char day_text[8];
        snprintf(day_text, sizeof(day_text), "%d", popup_calendar_date.day);
        lv_label_set_text(popup_day_label, day_text);
    }
    if (popup_year_label) {
        char year_text[8];
        snprintf(year_text, sizeof(year_text), "%d", popup_calendar_date.year);
        lv_label_set_text(popup_year_label, year_text);
    }
    
    // Update button colors
    popup_update_button_colors();
}

// Popup navigation callbacks
static void popup_calendar_prev_cb(lv_event_t *e) {
    (void)e;
    inactivity_timer_reset(INACTIVITY_CONTEXT_NON_HOME);
    switch (popup_current_mode) {
        case POPUP_CALENDAR_MODE_MONTH:
            calendar_prev_month(&popup_calendar_date);
            break;
        case POPUP_CALENDAR_MODE_DAY:
            calendar_prev_day(&popup_calendar_date);
            break;
        case POPUP_CALENDAR_MODE_YEAR:
            calendar_prev_year(&popup_calendar_date);
            break;
    }
    popup_update_calendar_displays();
}

static void popup_calendar_next_cb(lv_event_t *e) {
    (void)e;
    inactivity_timer_reset(INACTIVITY_CONTEXT_NON_HOME);
    switch (popup_current_mode) {
        case POPUP_CALENDAR_MODE_MONTH:
            calendar_next_month(&popup_calendar_date);
            break;
        case POPUP_CALENDAR_MODE_DAY:
            calendar_next_day(&popup_calendar_date);
            break;
        case POPUP_CALENDAR_MODE_YEAR:
            calendar_next_year(&popup_calendar_date);
            break;
    }
    popup_update_calendar_displays();
}

static void popup_calendar_select_month_cb(lv_event_t *e) {
    (void)e;
    inactivity_timer_reset(INACTIVITY_CONTEXT_NON_HOME);
    popup_current_mode = POPUP_CALENDAR_MODE_MONTH;
    popup_update_calendar_displays();
}

static void popup_calendar_select_day_cb(lv_event_t *e) {
    (void)e;
    inactivity_timer_reset(INACTIVITY_CONTEXT_NON_HOME);
    popup_current_mode = POPUP_CALENDAR_MODE_DAY;
    popup_update_calendar_displays();
}

static void popup_calendar_select_year_cb(lv_event_t *e) {
    (void)e;
    inactivity_timer_reset(INACTIVITY_CONTEXT_NON_HOME);
    popup_current_mode = POPUP_CALENDAR_MODE_YEAR;
    popup_update_calendar_displays();
}

static void popup_calendar_enter_cb(lv_event_t *e) {
    (void)e;

    inactivity_timer_reset(INACTIVITY_CONTEXT_NON_HOME);

    // Update app state with popup calendar date
    app_state_set_calendar_date(popup_calendar_date);
    
    // Update the main display
    update_calendar_display();
    
    // Save configuration
    save_theme_config();
    
    // Close the popup safely
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *popup = btn;
    // Find the root popup container
    while (popup && lv_obj_get_parent(popup) && lv_obj_get_parent(popup) != lv_scr_act()) {
        popup = lv_obj_get_parent(popup);
    }
    if (popup && lv_obj_get_parent(popup) == lv_scr_act()) {
        // Reset static variables to avoid dangling pointers
        popup_calendar_display_label = NULL;
        popup_month_label = NULL;
        popup_day_label = NULL;
        popup_year_label = NULL;
        popup_month_button = NULL;
        popup_day_button = NULL;
        popup_year_button = NULL;
        
        lv_obj_del(popup);
    }
}

// Show calendar popup when display is clicked
void show_calendar_popup(lv_event_t *e) {
    (void)e; // Unused parameter

    // Reset inactivity timer on user interaction
    inactivity_timer_reset(INACTIVITY_CONTEXT_NON_HOME);

    lv_obj_t *parent = lv_scr_act(); // Get the active screen

    // Initialize popup calendar with current app state date
    popup_calendar_date = app_state_get_calendar_date();
    popup_current_mode = POPUP_CALENDAR_MODE_MONTH;
    
    // Create popup overlay and container using helpers
    lv_obj_t *popup = create_popup_overlay(parent);
    lv_obj_t *calendar_container = create_popup_container(popup, 300, 280);
    
    // Title
    lv_obj_t *title_label = lv_label_create(calendar_container);
    lv_label_set_text(title_label, get_label("admin_screen.calendar_setting"));
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 10);
    // Use label font for regular labels
    if (app_state_get_font_label()) {
        lv_obj_set_style_text_font(title_label, app_state_get_font_label(), 0);
    }
    
    // Main display area for selected date (like Korean input text area)
    lv_obj_t *calendar_display = lv_label_create(calendar_container);
    lv_obj_set_style_bg_color(calendar_display, lv_color_hex(0x333333), 0); // Dark gray like Korean input
    lv_obj_set_style_bg_opa(calendar_display, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(calendar_display, lv_color_hex(get_button_border_color()), 0);
    lv_obj_set_style_border_width(calendar_display, 2, 0);
    lv_obj_set_style_text_color(calendar_display, lv_color_white(), 0);
    lv_obj_set_style_pad_all(calendar_display, 8, 0);
    lv_obj_set_style_text_align(calendar_display, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(calendar_display, 280, 50);
    lv_obj_align(calendar_display, LV_ALIGN_TOP_MID, 0, 60);
    // Use label font for regular labels
    if (app_state_get_font_label()) {
        lv_obj_set_style_text_font(calendar_display, app_state_get_font_label(), 0);
    }
    popup_calendar_display_label = calendar_display;
    
    // Navigation row - centered layout
    int label_width = 50;
    int label_height = 32;
    int nav_row_y_offset = 45;
    
    // Previous button
    lv_obj_t *prev_btn = create_nav_button(calendar_container, "<", 45, label_height, 0, 
                                           popup_calendar_prev_cb, NULL);
    lv_obj_align(prev_btn, LV_ALIGN_CENTER, -110, nav_row_y_offset);
    
    // Month button
    lv_obj_t *month_btn = lv_btn_create(calendar_container);
    lv_obj_set_size(month_btn, label_width, label_height);
    lv_obj_align(month_btn, LV_ALIGN_CENTER, -55, nav_row_y_offset);
    apply_button_style(month_btn, app_state_get_button_color());
    lv_obj_t *month_btn_label = lv_label_create(month_btn);
    lv_obj_set_style_text_color(month_btn_label, lv_color_white(), 0);
    // Use button font for button labels
    if (app_state_get_font_button()) {
        lv_obj_set_style_text_font(month_btn_label, app_state_get_font_button(), 0);
    }
    lv_obj_center(month_btn_label);
    lv_obj_add_event_cb(month_btn, popup_calendar_select_month_cb, LV_EVENT_CLICKED, NULL);
    popup_month_label = month_btn_label;
    popup_month_button = month_btn;
    
    // Day button
    lv_obj_t *day_btn = lv_btn_create(calendar_container);
    lv_obj_set_size(day_btn, label_width, label_height);
    lv_obj_align(day_btn, LV_ALIGN_CENTER, 0, nav_row_y_offset);
    apply_button_style(day_btn, app_state_get_button_color());
    lv_obj_t *day_btn_label = lv_label_create(day_btn);
    lv_obj_set_style_text_color(day_btn_label, lv_color_white(), 0);
    // Use button font for button labels
    if (app_state_get_font_button()) {
        lv_obj_set_style_text_font(day_btn_label, app_state_get_font_button(), 0);
    }
    lv_obj_center(day_btn_label);
    lv_obj_add_event_cb(day_btn, popup_calendar_select_day_cb, LV_EVENT_CLICKED, NULL);
    popup_day_label = day_btn_label;
    popup_day_button = day_btn;
    
    // Year button
    lv_obj_t *year_btn = lv_btn_create(calendar_container);
    lv_obj_set_size(year_btn, label_width, label_height);
    lv_obj_align(year_btn, LV_ALIGN_CENTER, 55, nav_row_y_offset);
    apply_button_style(year_btn, app_state_get_button_color());
    lv_obj_t *year_btn_label = lv_label_create(year_btn);
    lv_obj_set_style_text_color(year_btn_label, lv_color_white(), 0);
    // Use button font for button labels
    if (app_state_get_font_button()) {
        lv_obj_set_style_text_font(year_btn_label, app_state_get_font_button(), 0);
    }
    lv_obj_center(year_btn_label);
    lv_obj_add_event_cb(year_btn, popup_calendar_select_year_cb, LV_EVENT_CLICKED, NULL);
    popup_year_label = year_btn_label;
    popup_year_button = year_btn;
    
    // Next button
    lv_obj_t *next_btn = create_nav_button(calendar_container, ">", 45, label_height, 0, 
                                           popup_calendar_next_cb, NULL);
    lv_obj_align(next_btn, LV_ALIGN_CENTER, 110, nav_row_y_offset);
    
    // Enter button
    lv_obj_t *enter_btn = create_button_with_label(calendar_container, get_label("admin_screen.select"), 
                                                   90, 32, 0, popup_calendar_enter_cb, NULL);
    lv_obj_align(enter_btn, LV_ALIGN_CENTER, 0, 105);
    
    // Create close button using helper
    create_close_button(calendar_container, calendar_popup_close_cb, NULL);
    
    // Initialize displays
    popup_update_calendar_displays();
}

// ============================================================================
// FONT SETTING SECTION HELPER
// ============================================================================

/**
 * Creates a font setting section with label, font dropdown, and size dropdown
 * @param parent Parent object
 * @param y_pos Y position for the section
 * @param section_label Text for the section label
 * @param current_font_name Current font name to select
 * @param current_font_size Current font size to select
 * @param font_config Configuration for font name dropdown
 * @param size_config Configuration for font size dropdown
 */
static void create_font_setting_section(lv_obj_t *parent, int y_pos, const char *section_label,
                                        const char *current_font_name, int current_font_size,
                                        FontDropdownConfig *font_config, FontDropdownConfig *size_config) {
    // Section label
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, section_label);
    apply_label_style(label);
    lv_obj_set_pos(label, 10, y_pos);

    // Font selection dropdown
    lv_obj_t *font_dropdown = lv_dropdown_create(parent);
    lv_dropdown_set_options(font_dropdown,
        "NotoSansKR-Black\nNotoSansKR-Bold\nNotoSansKR-ExtraBold\nNotoSansKR-ExtraLight\nNotoSansKR-Light\nNotoSansKR-Medium\nNotoSansKR-Regular\nNotoSansKR-SemiBold\nNotoSansKR-Thin");
    lv_obj_set_width(font_dropdown, 220);
    lv_obj_set_pos(font_dropdown, 10, y_pos + 30);

    // Restore font selection from config
    int font_idx = 0;
    for (int i = 0; i < 9; ++i) {
        if (strcmp(current_font_name, font_names[i]) == 0) {
            font_idx = i;
            break;
        }
    }
    lv_dropdown_set_selected(font_dropdown, font_idx);
    lv_obj_add_event_cb(font_dropdown, font_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, (void*)font_config);

    // Font size label
    lv_obj_t *size_label = lv_label_create(parent);
    lv_label_set_text(size_label, get_label("admin_screen.font_size"));
    apply_label_style(size_label);
    lv_obj_set_pos(size_label, 240, y_pos);

    // Font size selection dropdown
    lv_obj_t *size_dropdown = lv_dropdown_create(parent);
    lv_dropdown_set_options(size_dropdown, "12\n14\n16\n18\n20\n24\n28\n32");
    lv_obj_set_width(size_dropdown, 80);
    lv_obj_set_pos(size_dropdown, 240, y_pos + 30);

    // Restore font size selection from config
    int size_idx = 4; // Default to 20
    for (int i = 0; i < 8; ++i) {
        if (current_font_size == font_sizes[i]) {
            size_idx = i;
            break;
        }
    }
    lv_dropdown_set_selected(size_dropdown, size_idx);
    lv_obj_add_event_cb(size_dropdown, font_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, (void*)size_config);
}

// ============================================================================
// MULTI-PAGE MANAGEMENT
// ============================================================================

#define ADMIN_PAGE_COUNT 4
#define PAGE_TITLE_BAR_HEIGHT 50
static int current_admin_page = 0;  // Current page index (0-3)
static lv_obj_t *admin_content_container = NULL;  // Reference to content container
static lv_obj_t *admin_page_title_bar = NULL;  // Page title bar container
static lv_obj_t *admin_prev_btn = NULL;  // Previous page button
static lv_obj_t *admin_next_btn = NULL;  // Next page button
static lv_obj_t *admin_page_label = NULL;  // Page indicator label

// Page names for display
static const char* page_names[] = {
    "Calendar",
    "Font",
    "Colors",
    "Language"
};

// Forward declarations for page creation functions
static void create_admin_page_calendar(lv_obj_t *content);
static void create_admin_page_font(lv_obj_t *content);
static void create_admin_page_colors(lv_obj_t *content);
static void create_admin_page_language(lv_obj_t *content);

// ============================================================================
// PAGE NAVIGATION
// ============================================================================

/**
 * Creates the page title bar with prev/next buttons and page information
 * This bar appears at the top of the content area, below the main title bar
 */
static void create_page_title_bar(lv_obj_t *parent) {
    // Create title bar container with equal margins on left and right
    int title_bar_margin = 5;
    int border_width = 2;
    // Get the actual width of the parent container
    // Since we set padding to 0 on the content container, this gives us the full width
    int parent_width = lv_obj_get_width(parent);
    // If width is 0 or invalid, fall back to screen width
    if (parent_width <= 0) {
        parent_width = SCREEN_WIDTH;
    }
    // Width calculation: parent width minus margins on both sides
    // Border is drawn inside the object, so it doesn't reduce the width
    // Using alignment centers the object, so margins are automatically applied
    int title_bar_width = parent_width - (title_bar_margin * 2);
    admin_page_title_bar = lv_obj_create(parent);
    lv_obj_set_size(admin_page_title_bar, title_bar_width, PAGE_TITLE_BAR_HEIGHT);
    lv_obj_align(admin_page_title_bar, LV_ALIGN_TOP_MID, 0, CONTENT_PADDING);
    lv_obj_set_style_bg_color(admin_page_title_bar, lv_color_hex(app_state_get_title_bar_color()), 0);
    lv_obj_set_style_border_width(admin_page_title_bar, border_width, 0);
    lv_obj_set_style_border_color(admin_page_title_bar, lv_color_hex(app_state_get_button_border_color()), 0);
    lv_obj_set_style_radius(admin_page_title_bar, 5, 0);
    lv_obj_set_style_pad_all(admin_page_title_bar, 0, 0);  // Remove default padding
    lv_obj_clear_flag(admin_page_title_bar, LV_OBJ_FLAG_SCROLLABLE);

    // Previous button (left side)
    admin_prev_btn = lv_btn_create(admin_page_title_bar);
    lv_obj_set_size(admin_prev_btn, 60, 36);
    lv_obj_align(admin_prev_btn, LV_ALIGN_LEFT_MID, 5, 0);
    apply_button_style(admin_prev_btn, app_state_get_button_color());

    lv_obj_t *prev_label = lv_label_create(admin_prev_btn);
    lv_label_set_text(prev_label, "<");
    lv_obj_set_style_text_color(prev_label, lv_color_hex(app_state_get_label_text_color()), 0);
    if (app_state_get_font_button()) {
        lv_obj_set_style_text_font(prev_label, app_state_get_font_button(), 0);
    }
    lv_obj_center(prev_label);
    lv_obj_add_event_cb(admin_prev_btn, admin_prev_page_callback, LV_EVENT_CLICKED, NULL);

    // Page label (center)
    admin_page_label = lv_label_create(admin_page_title_bar);
    char page_text[64];
    snprintf(page_text, sizeof(page_text), "%s (%d/%d)",
             page_names[current_admin_page], current_admin_page + 1, ADMIN_PAGE_COUNT);
    lv_label_set_text(admin_page_label, page_text);
    lv_obj_set_style_text_color(admin_page_label, lv_color_hex(app_state_get_label_text_color()), 0);
    if (app_state_get_font_label()) {
        lv_obj_set_style_text_font(admin_page_label, app_state_get_font_label(), 0);
    }
    lv_obj_align(admin_page_label, LV_ALIGN_CENTER, 0, 0);

    // Next button (right side)
    admin_next_btn = lv_btn_create(admin_page_title_bar);
    lv_obj_set_size(admin_next_btn, 60, 36);
    lv_obj_align(admin_next_btn, LV_ALIGN_RIGHT_MID, -5, 0);
    apply_button_style(admin_next_btn, app_state_get_button_color());

    lv_obj_t *next_label = lv_label_create(admin_next_btn);
    lv_label_set_text(next_label, ">");
    lv_obj_set_style_text_color(next_label, lv_color_hex(app_state_get_label_text_color()), 0);
    if (app_state_get_font_button()) {
        lv_obj_set_style_text_font(next_label, app_state_get_font_button(), 0);
    }
    lv_obj_center(next_label);
    lv_obj_add_event_cb(admin_next_btn, admin_next_page_callback, LV_EVENT_CLICKED, NULL);
}

static void update_page_navigation_buttons(void) {
    // Update prev button state
    if (admin_prev_btn) {
        if (current_admin_page == 0) {
            lv_obj_add_state(admin_prev_btn, LV_STATE_DISABLED);
        } else {
            lv_obj_clear_state(admin_prev_btn, LV_STATE_DISABLED);
        }
    }

    // Update next button state
    if (admin_next_btn) {
        if (current_admin_page == ADMIN_PAGE_COUNT - 1) {
            lv_obj_add_state(admin_next_btn, LV_STATE_DISABLED);
        } else {
            lv_obj_clear_state(admin_next_btn, LV_STATE_DISABLED);
        }
    }

    // Update page indicator with page name
    if (admin_page_label) {
        char page_text[64];
        snprintf(page_text, sizeof(page_text), "%s (%d/%d)",
                 page_names[current_admin_page], current_admin_page + 1, ADMIN_PAGE_COUNT);
        lv_label_set_text(admin_page_label, page_text);
    }
}

static void refresh_admin_page(void) {
    if (!admin_content_container) return;

    // Clear all children from content container
    lv_obj_clean(admin_content_container);

    // Recreate page title bar
    create_page_title_bar(admin_content_container);

    // Create the current page content
    switch (current_admin_page) {
        case 0:
            create_admin_page_calendar(admin_content_container);
            break;
        case 1:
            create_admin_page_font(admin_content_container);
            break;
        case 2:
            create_admin_page_colors(admin_content_container);
            break;
        case 3:
            create_admin_page_language(admin_content_container);
            break;
        default:
            break;
    }

    // Update navigation button states
    update_page_navigation_buttons();
}

static void admin_prev_page_callback(lv_event_t *e) {
    (void)e;
    inactivity_timer_reset(INACTIVITY_CONTEXT_NON_HOME);
    if (current_admin_page > 0) {
        current_admin_page--;
        refresh_admin_page();
    }
}

static void admin_next_page_callback(lv_event_t *e) {
    (void)e;
    inactivity_timer_reset(INACTIVITY_CONTEXT_NON_HOME);
    if (current_admin_page < ADMIN_PAGE_COUNT - 1) {
        current_admin_page++;
        refresh_admin_page();
    }
}

// ============================================================================
// PAGE CONTENT CREATION
// ============================================================================

// Page 0: Calendar Settings
static void create_admin_page_calendar(lv_obj_t *content) {
    // Calculate Y offset for content below page title bar
    int content_y_offset = CONTENT_PADDING + PAGE_TITLE_BAR_HEIGHT + 10;

    // Calendar Settings Section
    lv_obj_t *calendar_title = lv_label_create(content);
    lv_label_set_text(calendar_title, get_label("admin_screen.calendar_setting"));
    apply_label_style(calendar_title);
    lv_obj_set_pos(calendar_title, CONTENT_PADDING, content_y_offset);

    // Calendar date display button (clickable to open popup)
    lv_obj_t *calendar_btn = lv_btn_create(content);
    lv_obj_set_size(calendar_btn, 260, 50);
    lv_obj_set_pos(calendar_btn, CONTENT_PADDING, content_y_offset + 25);
    apply_button_style(calendar_btn, app_state_get_button_color());

    // Create label inside the button for the calendar text
    calendar_display_label = lv_label_create(calendar_btn);
    lv_obj_set_style_text_color(calendar_display_label, lv_color_white(), 0);
    if (app_state_get_font_button()) {
        lv_obj_set_style_text_font(calendar_display_label, app_state_get_font_button(), 0);
    }
    lv_obj_center(calendar_display_label);
    lv_obj_add_event_cb(calendar_btn, show_calendar_popup, LV_EVENT_CLICKED, NULL);

    // Initialize calendar with current date or system date
    calendar_date_t calendar_date = app_state_get_calendar_date();
    if (calendar_date.year == 0) {
        calendar_init(&calendar_date);
        app_state_set_calendar_date(calendar_date);
    }
    update_calendar_display();
}

// Page 1: Font Settings
static void create_admin_page_font(lv_obj_t *content) {
    // Calculate Y offset for content below page title bar
    int content_y_offset = CONTENT_PADDING + PAGE_TITLE_BAR_HEIGHT + 10;

    // Font Settings Sections
    create_font_setting_section(content, content_y_offset, get_label("admin_screen.title_bar_font"),
                                app_state_get_font_name_title(),
                                app_state_get_font_size_title_bar(),
                                &config_title_name,
                                &config_title_size);

    create_font_setting_section(content, content_y_offset + 80, get_label("admin_screen.status_bar_font"),
                                app_state_get_font_name_status_bar(),
                                app_state_get_font_size_status_bar(),
                                &config_status_name,
                                &config_status_size);

    create_font_setting_section(content, content_y_offset + 160, get_label("admin_screen.button_font"),
                                app_state_get_font_name_button_label(),
                                app_state_get_font_size_button_label(),
                                &config_button_name,
                                &config_button_size);

    create_font_setting_section(content, content_y_offset + 240, get_label("admin_screen.label_font"),
                                app_state_get_font_name_label(),
                                app_state_get_font_size_label(),
                                &config_label_name,
                                &config_label_size);

    create_font_setting_section(content, content_y_offset + 320, get_label("admin_screen.home_contents_font"),
                                app_state_get_font_name_home_contents(),
                                app_state_get_font_size_home_contents(),
                                &config_home_name,
                                &config_home_size);
}

// Page 2: Color Settings
static void create_admin_page_colors(lv_obj_t *content) {
    // Calculate Y offset for content below page title bar
    int content_y_offset = CONTENT_PADDING + PAGE_TITLE_BAR_HEIGHT + 10;

    // Color sections
    create_color_section(content, get_label("admin_screen.label_text_color"), content_y_offset, COLOR_TARGET_LABEL_TEXT);
    create_color_section(content, get_label("admin_screen.background_color"), content_y_offset + 80, COLOR_TARGET_BACKGROUND);
    create_color_section(content, get_label("admin_screen.title_bar_color"), content_y_offset + 160, COLOR_TARGET_TITLE_BAR);
    create_color_section(content, get_label("admin_screen.status_bar_color"), content_y_offset + 240, COLOR_TARGET_STATUS_BAR);
    create_color_section(content, get_label("admin_screen.button_color"), content_y_offset + 320, COLOR_TARGET_BUTTON);
    create_color_section(content, get_label("admin_screen.button_border_color"), content_y_offset + 400, COLOR_TARGET_BUTTON_BORDER);
}

// Page 3: Language Settings
static void create_admin_page_language(lv_obj_t *content) {
    // Calculate Y offset for content below page title bar
    int content_y_offset = CONTENT_PADDING + PAGE_TITLE_BAR_HEIGHT + 10;

    // Language Settings Section
    int lang_section_y = content_y_offset;
    lv_obj_t *language_title = lv_label_create(content);
    lv_label_set_text(language_title, get_label("admin_screen.language_title"));
    apply_label_style(language_title);
    lv_obj_set_pos(language_title, 10, lang_section_y);

    // Korean and English buttons in the same row
    int lang_btn_y = lang_section_y + 35;
    create_language_button(content, get_label("admin_screen.language_korean"), "ko", 10, lang_btn_y);
    create_language_button(content, get_label("admin_screen.language_english"), "en", 120, lang_btn_y);

    // Info text below language buttons
    lv_obj_t *info_label = lv_label_create(content);
    lv_label_set_long_mode(info_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(info_label, SCREEN_WIDTH - CONTENT_WIDTH_PADDING);
    lv_label_set_text(info_label, get_label("admin_screen.info_text"));
    lv_obj_set_style_text_color(info_label, lv_color_hex(0xAAAAAA), 0);
    if (app_state_get_font_label()) {
        lv_obj_set_style_text_font(info_label, app_state_get_font_label(), 0);
    }
    lv_obj_set_pos(info_label, CONTENT_PADDING, lang_btn_y + 60);
}

// ============================================================================
// ADMIN CONTENT CREATION
// ============================================================================

static lv_obj_t *create_admin_content(lv_obj_t *parent) {
    lv_obj_t *content = create_standard_content(parent);

    // Disable scrolling - using multi-page system instead
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_OFF);
    // Ensure content container has no padding that could affect child positioning
    lv_obj_set_style_pad_all(content, 0, 0);

    // Store content container reference for page refresh
    admin_content_container = content;

    // Reset to first page when screen is created
    current_admin_page = 0;

    // Create page title bar at the top of content area
    create_page_title_bar(content);

    // Create initial page content
    create_admin_page_calendar(content);

    // Initial button state update
    update_page_navigation_buttons();

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
