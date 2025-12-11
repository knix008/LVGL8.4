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
                                        const char *language_code, int x_pos) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 90, 40);
    lv_obj_set_pos(btn, x_pos, 665);
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
    popup_current_mode = POPUP_CALENDAR_MODE_MONTH;
    popup_update_calendar_displays();
}

static void popup_calendar_select_day_cb(lv_event_t *e) {
    (void)e;
    popup_current_mode = POPUP_CALENDAR_MODE_DAY;
    popup_update_calendar_displays();
}

static void popup_calendar_select_year_cb(lv_event_t *e) {
    (void)e;
    popup_current_mode = POPUP_CALENDAR_MODE_YEAR;
    popup_update_calendar_displays();
}

static void popup_calendar_enter_cb(lv_event_t *e) {
    (void)e;
    
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
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(title_label, app_state_get_font_20(), 0);
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
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(calendar_display, app_state_get_font_20(), 0);
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
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(month_btn_label, app_state_get_font_20(), 0);
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
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(day_btn_label, app_state_get_font_20(), 0);
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
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(year_btn_label, app_state_get_font_20(), 0);
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

    // Calendar Settings Section (First Row)
    lv_obj_t *calendar_title = lv_label_create(content);
    lv_label_set_text(calendar_title, get_label("admin_screen.calendar_setting"));
    apply_label_style(calendar_title);
    lv_obj_set_pos(calendar_title, CONTENT_PADDING, 40);

    // Calendar date display button (clickable to open popup)
    lv_obj_t *calendar_btn = lv_btn_create(content);
    lv_obj_set_size(calendar_btn, 260, 50);
    lv_obj_set_pos(calendar_btn, CONTENT_PADDING, 65);
    apply_button_style(calendar_btn, app_state_get_button_color());
    
    // Create label inside the button for the calendar text
    calendar_display_label = lv_label_create(calendar_btn);
    lv_obj_set_style_text_color(calendar_display_label, lv_color_white(), 0);
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(calendar_display_label, app_state_get_font_20(), 0);
    }
    lv_obj_center(calendar_display_label);
    
    // Add click event to button
    lv_obj_add_event_cb(calendar_btn, show_calendar_popup, LV_EVENT_CLICKED, NULL);

    // Initialize calendar with current date or system date
    calendar_date_t calendar_date = app_state_get_calendar_date();
    if (calendar_date.year == 0) {
        calendar_init(&calendar_date);
        app_state_set_calendar_date(calendar_date);
    }
    update_calendar_display();

    // Create five color sections (moved down to make room for calendar)
    create_color_section(content, get_label("admin_screen.background_color"), 140, COLOR_TARGET_BACKGROUND);
    create_color_section(content, get_label("admin_screen.title_bar_color"), 220, COLOR_TARGET_TITLE_BAR);
    create_color_section(content, get_label("admin_screen.status_bar_color"), 300, COLOR_TARGET_STATUS_BAR);
    create_color_section(content, get_label("admin_screen.button_color"), 380, COLOR_TARGET_BUTTON);
    create_color_section(content, get_label("admin_screen.button_border_color"), 460, COLOR_TARGET_BUTTON_BORDER);
    create_color_section(content, get_label("admin_screen.label_text_color"), 540, COLOR_TARGET_LABEL_TEXT);

    // Language Settings Section (moved down to avoid overlap with label text color section)
    lv_obj_t *language_title = lv_label_create(content);
    lv_label_set_text(language_title, get_label("admin_screen.language_title"));
    apply_label_style(language_title);
    lv_obj_set_pos(language_title, 10, 630);

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
    if (app_state_get_font_20()) {
        lv_obj_set_style_text_font(info_label, app_state_get_font_20(), 0);
    }
    lv_obj_set_pos(info_label, CONTENT_PADDING, 710);

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
