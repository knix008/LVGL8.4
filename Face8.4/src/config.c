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
