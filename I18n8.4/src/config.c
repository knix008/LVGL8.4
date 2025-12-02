#include "../include/config.h"
#include "../include/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>

// External reference to the global app state
extern AppState app_state;

// ============================================================================
// CONFIGURATION FILE MANAGEMENT
// ============================================================================

// Ensure config directory exists
static int ensure_config_directory(void) {
    struct stat st = {0};

    if (stat(CONFIG_DIR, &st) == -1) {
        if (mkdir(CONFIG_DIR, 0755) == -1) {
            fprintf(stderr, "Error: Failed to create config directory: %s\n", strerror(errno));
            return -1;
        }
    }

    return 0;
}

// ============================================================================
// JSON HELPER FUNCTIONS
// ============================================================================

// Skip whitespace in JSON
static const char* skip_whitespace(const char* str) {
    while (*str && isspace(*str)) str++;
    return str;
}

// Read entire file into memory
static char* read_file_contents(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = malloc(size + 1);
    if (!content) {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(content, 1, size, file);
    content[bytes_read] = '\0';
    fclose(file);

    return content;
}

// Find a JSON value by key (simple implementation for our use case)
static const char* find_json_value(const char* json, const char* key) {
    char search[256];
    snprintf(search, sizeof(search), "\"%s\"", key);
    
    const char* pos = strstr(json, search);
    if (!pos) return NULL;
    
    pos += strlen(search);
    pos = skip_whitespace(pos);
    
    if (*pos != ':') return NULL;
    pos++;
    pos = skip_whitespace(pos);
    
    return pos;
}

// Extract boolean value
static int parse_bool(const char* str) {
    str = skip_whitespace(str);
    return (strncmp(str, "true", 4) == 0);
}

// ============================================================================
// SAVE STATUS BAR CONFIGURATION
// ============================================================================

/**
 * Saves the current configuration to JSON file.
 * Preserves all sections including border configuration.
 * 
 * @return 0 on success, -1 on failure
 */
int save_status_bar_config(void) {
    // Ensure config directory exists
    if (ensure_config_directory() != 0) {
        return -1;
    }

    // Read existing config to preserve border section
    char* existing_config = read_file_contents(STATUS_BAR_CONFIG_FILE);
    
    // Extract border section if it exists
    char border_section[2048] = "";
    if (existing_config) {
        const char* border_start = strstr(existing_config, "\"border\"");
        if (border_start) {
            // Find the opening brace for border object
            const char* brace = strchr(border_start, '{');
            if (brace) {
                int depth = 1;
                const char* p = brace + 1;
                while (*p && depth > 0) {
                    if (*p == '{') depth++;
                    else if (*p == '}') depth--;
                    p++;
                }
                if (depth == 0) {
                    // Extract border section
                    size_t len = p - border_start;
                    if (len < sizeof(border_section) - 1) {
                        strncpy(border_section, border_start, len);
                        border_section[len] = '\0';
                    }
                }
            }
        }
        free(existing_config);
    }

    FILE *file = fopen(STATUS_BAR_CONFIG_FILE, "w");
    if (!file) {
        fprintf(stderr, "Error: Failed to open config file for writing: %s\n", 
                STATUS_BAR_CONFIG_FILE);
        return -1;
    }

    // Write JSON configuration
    fprintf(file, "{\n");
    fprintf(file, "  \"status_bar\": {\n");
    
    for (int i = 0; i < MAX_STATUS_ICONS; i++) {
        const char *value = app_state.menu_item_selected[i] ? "true" : "false";
        fprintf(file, "    \"%s\": %s%s\n", 
                MENU_ITEMS[i].config_key, value,
                (i < MAX_STATUS_ICONS - 1) ? "," : "");
    }
    
    fprintf(file, "  }");

    // Add border section if it exists
    if (border_section[0] != '\0') {
        fprintf(file, ",\n  %s", border_section);
    }

    fprintf(file, "\n}\n");

    fclose(file);
    return 0;
}

// ============================================================================
// LOAD STATUS BAR CONFIGURATION
// ============================================================================

/**
 * Loads the status bar configuration from JSON file.
 * Restores previously saved enabled/disabled state of status bar icons.
 * 
 * @return 0 on success, -1 on failure
 */
int load_status_bar_config(void) {
    char* content = read_file_contents(STATUS_BAR_CONFIG_FILE);
    
    if (!content) {
        // Initialize with default values (all disabled)
        for (int i = 0; i < MAX_STATUS_ICONS; i++) {
            app_state.menu_item_selected[i] = false;
        }
        return 0;  // Not an error, just use defaults
    }

    // Find status_bar section
    const char* status_bar = find_json_value(content, "status_bar");
    if (status_bar && *status_bar == '{') {
        // Parse each menu item
        for (int i = 0; i < MAX_STATUS_ICONS; i++) {
            const char* value = find_json_value(status_bar, MENU_ITEMS[i].config_key);
            if (value) {
                app_state.menu_item_selected[i] = parse_bool(value);
            } else {
                app_state.menu_item_selected[i] = false;
            }
        }
    } else {
        // Initialize with defaults
        for (int i = 0; i < MAX_STATUS_ICONS; i++) {
            app_state.menu_item_selected[i] = false;
        }
    }

    free(content);
    return 0;
}

// ============================================================================
// THEME CONFIGURATION
// ============================================================================

/**
 * Get the current background color from app state or default
 */
uint32_t get_background_color(void) {
    return app_state.bg_color ? app_state.bg_color : COLOR_BG_DARK;
}

/**
 * Get the current title bar color from app state or default
 */
uint32_t get_title_bar_color(void) {
    return app_state.title_bar_color ? app_state.title_bar_color : COLOR_BG_TITLE;
}

/**
 * Get the current status bar color from app state or default
 */
uint32_t get_status_bar_color(void) {
    return app_state.status_bar_color ? app_state.status_bar_color : COLOR_BG_TITLE;
}

/**
 * Get the current button color from app state or default
 */
uint32_t get_button_color(void) {
    return app_state.button_color ? app_state.button_color : COLOR_BUTTON_BG;
}

/**
 * Get the current button border color from app state or default
 */
uint32_t get_button_border_color(void) {
    return app_state.button_border_color ? app_state.button_border_color : COLOR_BORDER;
}

/**
 * Save theme configuration including background color
 */
int save_theme_config(void) {
    if (ensure_config_directory() != 0) {
        return -1;
    }

    // Read existing config
    char* existing_config = read_file_contents(STATUS_BAR_CONFIG_FILE);
    
    // Extract all sections except theme
    char status_bar_section[1024] = "";
    char border_section[2048] = "";
    
    if (existing_config) {
        // Extract status_bar section
        const char* sb_start = strstr(existing_config, "\"status_bar\"");
        if (sb_start) {
            const char* brace = strchr(sb_start, '{');
            if (brace) {
                int depth = 1;
                const char* p = brace + 1;
                while (*p && depth > 0) {
                    if (*p == '{') depth++;
                    else if (*p == '}') depth--;
                    p++;
                }
                if (depth == 0) {
                    size_t len = p - sb_start;
                    if (len < sizeof(status_bar_section) - 1) {
                        strncpy(status_bar_section, sb_start, len);
                        status_bar_section[len] = '\0';
                    }
                }
            }
        }
        
        // Extract border section
        const char* border_start = strstr(existing_config, "\"border\"");
        if (border_start) {
            const char* brace = strchr(border_start, '{');
            if (brace) {
                int depth = 1;
                const char* p = brace + 1;
                while (*p && depth > 0) {
                    if (*p == '{') depth++;
                    else if (*p == '}') depth--;
                    p++;
                }
                if (depth == 0) {
                    size_t len = p - border_start;
                    if (len < sizeof(border_section) - 1) {
                        strncpy(border_section, border_start, len);
                        border_section[len] = '\0';
                    }
                }
            }
        }
        free(existing_config);
    }

    FILE *file = fopen(STATUS_BAR_CONFIG_FILE, "w");
    if (!file) {
        fprintf(stderr, "Error: Failed to open config file for writing\n");
        return -1;
    }

    fprintf(file, "{\n");
    
    // Write status_bar section if exists
    if (status_bar_section[0] != '\0') {
        fprintf(file, "  %s,\n", status_bar_section);
    }
    
    // Write border section if exists
    if (border_section[0] != '\0') {
        fprintf(file, "  %s,\n", border_section);
    }
    
    // Write theme section
    fprintf(file, "  \"theme\": {\n");
    fprintf(file, "    \"background_color\": \"0x%06X\",\n", app_state.bg_color);
    fprintf(file, "    \"title_bar_color\": \"0x%06X\",\n", app_state.title_bar_color);
    fprintf(file, "    \"status_bar_color\": \"0x%06X\",\n", app_state.status_bar_color);
    fprintf(file, "    \"button_color\": \"0x%06X\",\n", app_state.button_color);
    fprintf(file, "    \"button_border_color\": \"0x%06X\",\n", app_state.button_border_color);
    fprintf(file, "    \"button_width\": %d,\n", app_state.button_width);
    fprintf(file, "    \"button_height\": %d,\n", app_state.button_height);
    fprintf(file, "    \"button_border_width\": %d,\n", app_state.button_border_width);
    fprintf(file, "    \"button_border_radius\": %d\n", app_state.button_border_radius);
    fprintf(file, "  }\n");
    fprintf(file, "}\n");

    fclose(file);
    return 0;
}

/**
 * Load theme configuration
 */
int load_theme_config(void) {
    char* content = read_file_contents(STATUS_BAR_CONFIG_FILE);
    
    if (!content) {
        app_state.bg_color = COLOR_BG_DARK;
        app_state.title_bar_color = COLOR_BG_TITLE;
        app_state.status_bar_color = COLOR_BG_TITLE;
        app_state.button_color = COLOR_BUTTON_BG;
        app_state.button_border_color = COLOR_BORDER;
        app_state.button_width = 120;
        app_state.button_height = 50;
        app_state.button_border_width = 2;
        app_state.button_border_radius = 10;
        return 0;
    }

    // Find theme section
    const char* theme = find_json_value(content, "theme");
    if (theme && *theme == '{') {
        const char* bg_color = find_json_value(theme, "background_color");
        if (bg_color) {
            // Skip quotes if present
            while (*bg_color && (*bg_color == '\"' || isspace(*bg_color))) bg_color++;
            app_state.bg_color = strtoul(bg_color, NULL, 0);
        } else {
            app_state.bg_color = COLOR_BG_DARK;
        }
        
        const char* title_color = find_json_value(theme, "title_bar_color");
        if (title_color) {
            while (*title_color && (*title_color == '\"' || isspace(*title_color))) title_color++;
            app_state.title_bar_color = strtoul(title_color, NULL, 0);
        } else {
            app_state.title_bar_color = COLOR_BG_TITLE;
        }
        
        const char* status_color = find_json_value(theme, "status_bar_color");
        if (status_color) {
            while (*status_color && (*status_color == '\"' || isspace(*status_color))) status_color++;
            app_state.status_bar_color = strtoul(status_color, NULL, 0);
        } else {
            app_state.status_bar_color = COLOR_BG_TITLE;
        }
        
        const char* button_color = find_json_value(theme, "button_color");
        if (button_color) {
            while (*button_color && (*button_color == '\"' || isspace(*button_color))) button_color++;
            app_state.button_color = strtoul(button_color, NULL, 0);
        } else {
            app_state.button_color = COLOR_BUTTON_BG;
        }
        
        const char* button_border = find_json_value(theme, "button_border_color");
        if (button_border) {
            while (*button_border && (*button_border == '\"' || isspace(*button_border))) button_border++;
            app_state.button_border_color = strtoul(button_border, NULL, 0);
        } else {
            app_state.button_border_color = COLOR_BORDER;
        }
        
        const char* btn_width = find_json_value(theme, "button_width");
        if (btn_width) {
            while (*btn_width && isspace(*btn_width)) btn_width++;
            app_state.button_width = atoi(btn_width);
        } else {
            app_state.button_width = 120;
        }
        
        const char* btn_height = find_json_value(theme, "button_height");
        if (btn_height) {
            while (*btn_height && isspace(*btn_height)) btn_height++;
            app_state.button_height = atoi(btn_height);
        } else {
            app_state.button_height = 50;
        }
        
        const char* btn_border_width = find_json_value(theme, "button_border_width");
        if (btn_border_width) {
            while (*btn_border_width && isspace(*btn_border_width)) btn_border_width++;
            app_state.button_border_width = atoi(btn_border_width);
        } else {
            app_state.button_border_width = 2;
        }
        
        const char* btn_border_radius = find_json_value(theme, "button_border_radius");
        if (btn_border_radius) {
            while (*btn_border_radius && isspace(*btn_border_radius)) btn_border_radius++;
            app_state.button_border_radius = atoi(btn_border_radius);
        } else {
            app_state.button_border_radius = 10;
        }
    } else {
        app_state.bg_color = COLOR_BG_DARK;
        app_state.title_bar_color = COLOR_BG_TITLE;
        app_state.status_bar_color = COLOR_BG_TITLE;
        app_state.button_color = COLOR_BUTTON_BG;
        app_state.button_border_color = COLOR_BORDER;
        app_state.button_width = 120;
        app_state.button_height = 50;
        app_state.button_border_width = 2;
        app_state.button_border_radius = 10;
    }

    free(content);
    return 0;
}
