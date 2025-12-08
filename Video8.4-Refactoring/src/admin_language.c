#include "../include/admin_language.h"
#include "../include/config.h"
#include "../include/state.h"
#include "../include/types.h"
#include "../include/style.h"
#include "../include/screen.h"
#include "../include/navigation.h"
#include "../include/label.h"
#include "../include/home.h"
#include "../include/welcome.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// LANGUAGE SELECTION IMPLEMENTATION
// ============================================================================

// Forward declarations
static void refresh_admin_screen_timer_cb(lv_timer_t *timer);
static void language_button_clicked(lv_event_t *e);

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

// Helper to create language button
static lv_obj_t *create_language_button(lv_obj_t *parent, const char *label_text,
                                        const char *language_code, int x_pos, int y_pos) {
    if (!parent || !label_text || !language_code) return NULL;

    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 105, 40);  // Reduced to 105 to account for scrollbar
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

// Creates language selection buttons
void create_language_section(lv_obj_t *parent, int y_pos) {
    if (!parent) return;

    // Language Settings Section Title - left aligned at 5px
    lv_obj_t *language_title = lv_label_create(parent);
    lv_label_set_text(language_title, get_label("admin_screen.language_title"));
    apply_label_style(language_title);
    lv_obj_set_pos(language_title, 5, y_pos);

    int button_y = y_pos + 35;
    int button_width = 105;
    int spacing = 12;

    // Start at left position 5px
    int start_x = 5;

    // Korean button
    create_language_button(parent, get_label("admin_screen.language_korean"), "ko", start_x, button_y);

    // English button
    create_language_button(parent, get_label("admin_screen.language_english"), "en", start_x + button_width + spacing, button_y);
}
