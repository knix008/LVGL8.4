#include "../include/config.h"
#include "../include/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

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
// SAVE STATUS BAR CONFIGURATION
// ============================================================================

/**
 * Saves the current status bar configuration to YAML file.
 * Persists enabled/disabled state of status bar icons.
 * 
 * @return 0 on success, -1 on failure
 */
int save_status_bar_config(void) {
    // Ensure config directory exists
    if (ensure_config_directory() != 0) {
        return -1;
    }

    FILE *file = fopen(STATUS_BAR_CONFIG_FILE, "w");
    if (!file) {
        fprintf(stderr, "Error: Failed to open config file for writing: %s\n", 
                STATUS_BAR_CONFIG_FILE);
        return -1;
    }

    yaml_emitter_t emitter;
    yaml_event_t event;

    // Initialize emitter
    if (!yaml_emitter_initialize(&emitter)) {
        fprintf(stderr, "Error: Failed to initialize YAML emitter\n");
        fclose(file);
        return -1;
    }

    yaml_emitter_set_output_file(&emitter, file);

    // Start YAML document
    yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING);
    if (!yaml_emitter_emit(&emitter, &event)) {
        goto emitter_error;
    }

    yaml_document_start_event_initialize(&event, NULL, NULL, NULL, 1);
    if (!yaml_emitter_emit(&emitter, &event)) {
        goto emitter_error;
    }

    // Start mapping (root object)
    yaml_mapping_start_event_initialize(&event, NULL, 
                                        (yaml_char_t *)YAML_MAP_TAG, 1, 
                                        YAML_ANY_MAPPING_STYLE);
    if (!yaml_emitter_emit(&emitter, &event)) {
        goto emitter_error;
    }

    // Add "status_bar" key
    yaml_scalar_event_initialize(&event, NULL, (yaml_char_t *)YAML_STR_TAG,
                                (yaml_char_t *)"status_bar", 
                                strlen("status_bar"),
                                1, 1, YAML_PLAIN_SCALAR_STYLE);
    if (!yaml_emitter_emit(&emitter, &event)) {
        goto emitter_error;
    }

    // Start nested mapping for status_bar
    yaml_mapping_start_event_initialize(&event, NULL, 
                                        (yaml_char_t *)YAML_MAP_TAG, 1, 
                                        YAML_ANY_MAPPING_STYLE);
    if (!yaml_emitter_emit(&emitter, &event)) {
        goto emitter_error;
    }

    // Iterate through status bar configuration
    for (int i = 0; i < MAX_STATUS_ICONS; i++) {
        // Key: menu item name
        yaml_scalar_event_initialize(&event, NULL, (yaml_char_t *)YAML_STR_TAG,
                                    (yaml_char_t *)MENU_ITEMS[i].config_key, 
                                    strlen(MENU_ITEMS[i].config_key),
                                    1, 1, YAML_PLAIN_SCALAR_STYLE);
        if (!yaml_emitter_emit(&emitter, &event)) {
            goto emitter_error;
        }

        // Value: enabled/disabled
        const char *value = app_state.menu_item_selected[i] ? "true" : "false";
        yaml_scalar_event_initialize(&event, NULL, (yaml_char_t *)YAML_BOOL_TAG,
                                    (yaml_char_t *)value, strlen(value),
                                    1, 1, YAML_PLAIN_SCALAR_STYLE);
        if (!yaml_emitter_emit(&emitter, &event)) {
            goto emitter_error;
        }
    }

    // End status_bar mapping
    yaml_mapping_end_event_initialize(&event);
    if (!yaml_emitter_emit(&emitter, &event)) {
        goto emitter_error;
    }

    // End root mapping
    yaml_mapping_end_event_initialize(&event);
    if (!yaml_emitter_emit(&emitter, &event)) {
        goto emitter_error;
    }

    // End document
    yaml_document_end_event_initialize(&event, 1);
    if (!yaml_emitter_emit(&emitter, &event)) {
        goto emitter_error;
    }

    yaml_stream_end_event_initialize(&event);
    if (!yaml_emitter_emit(&emitter, &event)) {
        goto emitter_error;
    }

    // Cleanup
    yaml_emitter_delete(&emitter);
    fclose(file);

    return 0;

emitter_error:
    fprintf(stderr, "Error: Failed to emit YAML event\n");
    yaml_emitter_delete(&emitter);
    fclose(file);
    return -1;
}

// ============================================================================
// LOAD STATUS BAR CONFIGURATION
// ============================================================================

/**
 * Loads the status bar configuration from YAML file.
 * Restores previously saved enabled/disabled state of status bar icons.
 * 
 * @return 0 on success, -1 on failure
 */
int load_status_bar_config(void) {
    FILE *file = fopen(STATUS_BAR_CONFIG_FILE, "r");
    if (!file) {
        // Initialize with default values (all disabled)
        for (int i = 0; i < MAX_STATUS_ICONS; i++) {
            app_state.menu_item_selected[i] = false;
        }
        return 0;  // Not an error, just use defaults
    }

    yaml_parser_t parser;
    yaml_event_t event;

    // Initialize parser
    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Error: Failed to initialize YAML parser\n");
        fclose(file);
        return -1;
    }

    yaml_parser_set_input_file(&parser, file);

    int in_root_mapping = 0;
    int in_status_bar_mapping = 0;
    char *last_key = NULL;

    // Parse YAML events
    do {
        if (!yaml_parser_parse(&parser, &event)) {
            fprintf(stderr, "Error: Parser error %d\n", parser.error);
            goto parser_error;
        }

        switch (event.type) {
            case YAML_MAPPING_START_EVENT:
                if (!in_root_mapping) {
                    in_root_mapping = 1;
                } else if (in_root_mapping && last_key && strcmp(last_key, "status_bar") == 0) {
                    in_status_bar_mapping = 1;
                    free(last_key);
                    last_key = NULL;
                }
                break;

            case YAML_SCALAR_EVENT:
                if (in_status_bar_mapping) {
                    if (!last_key) {
                        // This is a key
                        last_key = strdup((char *)event.data.scalar.value);
                    } else {
                        // This is a value
                        const char *value = (char *)event.data.scalar.value;
                        
                        // Find which menu item this corresponds to
                        for (int i = 0; i < MAX_STATUS_ICONS; i++) {
                            if (strcmp(last_key, MENU_ITEMS[i].config_key) == 0) {
                                // Set the configuration
                                if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) {
                                    app_state.menu_item_selected[i] = true;
                                } else {
                                    app_state.menu_item_selected[i] = false;
                                }
                                break;
                            }
                        }
                        
                        free(last_key);
                        last_key = NULL;
                    }
                } else if (in_root_mapping && !last_key) {
                    // Store root-level keys (like "status_bar")
                    last_key = strdup((char *)event.data.scalar.value);
                }
                break;

            case YAML_MAPPING_END_EVENT:
                if (in_status_bar_mapping) {
                    in_status_bar_mapping = 0;
                } else if (in_root_mapping) {
                    in_root_mapping = 0;
                }
                break;

            default:
                break;
        }

        if (event.type != YAML_STREAM_END_EVENT) {
            yaml_event_delete(&event);
        }
    } while (event.type != YAML_STREAM_END_EVENT);

    yaml_event_delete(&event);

    // Cleanup
    if (last_key) {
        free(last_key);
    }
    yaml_parser_delete(&parser);
    fclose(file);

    return 0;

parser_error:
    if (last_key) {
        free(last_key);
    }
    yaml_parser_delete(&parser);
    fclose(file);
    return -1;
}
