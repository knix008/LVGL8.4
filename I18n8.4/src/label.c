#include "../include/label.h"
#include "../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ============================================================================
// LABEL STORAGE
// ============================================================================

#define MAX_LABELS 200
#define MAX_KEY_LENGTH 256
#define MAX_VALUE_LENGTH 512
#define MAX_LANGUAGE_CODE 4

typedef struct {
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
} LabelEntry;

static LabelEntry labels[MAX_LABELS];
static int label_count = 0;
static char current_language[MAX_LANGUAGE_CODE] = "ko";  // Default to Korean

// ============================================================================
// JSON PARSING HELPERS
// ============================================================================

static const char* skip_whitespace(const char* str) {
    while (*str && isspace(*str)) str++;
    return str;
}

static const char* skip_string(const char* str) {
    if (*str != '"') return str;
    str++; // Skip opening quote
    while (*str && *str != '"') {
        if (*str == '\\' && *(str + 1)) str++; // Skip escaped char
        str++;
    }
    if (*str == '"') str++; // Skip closing quote
    return str;
}

static const char* skip_value(const char* str);

static const char* skip_object(const char* str) {
    if (*str != '{') return str;
    str++; // Skip opening brace
    int depth = 1;
    while (*str && depth > 0) {
        str = skip_whitespace(str);
        if (*str == '{') {
            depth++;
            str++;
        }
        else if (*str == '}') {
            depth--;
            str++;
        }
        else if (*str == '"') str = skip_string(str);
        else if (*str) str++;
    }
    return str;
}

static const char* skip_array(const char* str) {
    if (*str != '[') return str;
    str++; // Skip opening bracket
    int depth = 1;
    while (*str && depth > 0) {
        str = skip_whitespace(str);
        if (*str == '[') depth++;
        else if (*str == ']') depth--;
        else if (*str == '"') str = skip_string(str);
        else if (*str == '{') str = skip_object(str);
        else str++;
    }
    return str;
}

static const char* skip_value(const char* str) {
    str = skip_whitespace(str);
    if (*str == '"') return skip_string(str);
    if (*str == '{') return skip_object(str);
    if (*str == '[') return skip_array(str);
    // Skip other values (numbers, true, false, null)
    while (*str && *str != ',' && *str != '}' && *str != ']') str++;
    return str;
}

static int extract_string(const char* str, char* buffer, int max_len) {
    str = skip_whitespace(str);
    if (*str != '"') return -1;
    
    str++; // Skip opening quote
    int i = 0;
    while (*str && *str != '"' && i < max_len - 1) {
        if (*str == '\\') {
            str++;
            if (*str == 'n') buffer[i++] = '\n';
            else if (*str == 't') buffer[i++] = '\t';
            else if (*str == 'r') buffer[i++] = '\r';
            else if (*str == '\\') buffer[i++] = '\\';
            else if (*str == '"') buffer[i++] = '"';
            else buffer[i++] = *str;
            str++;
        } else {
            buffer[i++] = *str++;
        }
    }
    buffer[i] = '\0';
    return 0;
}

// ============================================================================
// RECURSIVE JSON PARSING
// ============================================================================

static void parse_json_object(const char* json, const char* prefix) {
    if (!json || label_count >= MAX_LABELS) return;
    
    json = skip_whitespace(json);
    if (*json != '{') return;
    json++; // Skip opening brace
    
    while (*json && *json != '}' && label_count < MAX_LABELS) {
        json = skip_whitespace(json);
        if (*json == '}') break;
        if (*json == ',') {
            json++;
            json = skip_whitespace(json);
            if (*json == '}') break;
        }
        
        // Extract key
        char key[MAX_KEY_LENGTH];
        if (*json != '"') break;
        if (extract_string(json, key, sizeof(key)) != 0) break;
        const char* after_key = skip_string(json);
        json = after_key;
        
        // Skip colon
        json = skip_whitespace(json);
        if (*json != ':') break;
        json++;
        json = skip_whitespace(json);
        
        // Build full key path
        char full_key[MAX_KEY_LENGTH];
        if (prefix[0]) {
            size_t prefix_len = strlen(prefix);
            size_t key_len = strlen(key);
            if (prefix_len + key_len + 2 < MAX_KEY_LENGTH) {
                size_t written = 0;
                strncpy(full_key, prefix, MAX_KEY_LENGTH - 1);
                written = strlen(full_key);
                full_key[written] = '.';
                written++;
                strncpy(full_key + written, key, MAX_KEY_LENGTH - written - 1);
                full_key[MAX_KEY_LENGTH - 1] = '\0';
            } else {
                continue; // Skip if key would be too long
            }
        } else {
            strncpy(full_key, key, MAX_KEY_LENGTH - 1);
            full_key[MAX_KEY_LENGTH - 1] = '\0';
        }
        
        // Check value type
        if (*json == '{') {
            // Nested object - recurse
            const char* obj_start = json;
            parse_json_object(json, full_key);
            json = skip_object(obj_start);
        } else if (*json == '"') {
            // String value - store it
            char value[MAX_VALUE_LENGTH];
            if (extract_string(json, value, sizeof(value)) == 0) {
                size_t key_len = strlen(full_key);
                size_t val_len = strlen(value);
                if (key_len < MAX_KEY_LENGTH && val_len < MAX_VALUE_LENGTH) {
                    strncpy(labels[label_count].key, full_key, MAX_KEY_LENGTH);
                    strncpy(labels[label_count].value, value, MAX_VALUE_LENGTH);
                    labels[label_count].key[MAX_KEY_LENGTH - 1] = '\0';
                    labels[label_count].value[MAX_VALUE_LENGTH - 1] = '\0';
                    label_count++;
                }
            }
            json = skip_string(json);
        } else {
            // Other value type - skip it
            json = skip_value(json);
        }
    }
}

// ============================================================================
// PUBLIC API
// ============================================================================

int load_labels(void) {
    label_count = 0;

    FILE* file = fopen("config/language.json", "r");
    if (!file) {
        fprintf(stderr, "Error: Failed to open config/language.json\n");
        return -1;
    }

    // Read file with boundary checking
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Check file size against maximum buffer
    if (size > MAX_LABELS_JSON_SIZE - 1) {
        fclose(file);
        fprintf(stderr, "Error: language.json exceeds maximum size (%ld > %d)\n",
                size, MAX_LABELS_JSON_SIZE - 1);
        return -1;
    }

    static char content[MAX_LABELS_JSON_SIZE];
    size_t bytes_read = fread(content, 1, size, file);
    content[bytes_read] = '\0';
    fclose(file);

    // Parse JSON
    parse_json_object(content, "");

    if (label_count == 0) {
        fprintf(stderr, "Error: No labels loaded from language.json\n");
        return -1;
    }

    return 0;
}

const char* get_label(const char* key_path) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].key, key_path) == 0) {
            return labels[i].value;
        }
    }

    // Return key if not found (only show error in debug mode)
    #ifdef DEBUG_LABELS
    fprintf(stderr, "Warning: Label not found: %s\n", key_path);
    #endif
    return key_path;
}

int set_language(const char* language) {
    if (!language || (strcmp(language, "ko") != 0 && strcmp(language, "en") != 0)) {
        fprintf(stderr, "Error: Invalid language code: %s\n", language ? language : "NULL");
        return -1;
    }

    // Store the language preference
    strncpy(current_language, language, MAX_LANGUAGE_CODE - 1);
    current_language[MAX_LANGUAGE_CODE - 1] = '\0';

    // Reload labels from the JSON file
    label_count = 0;

    FILE* file = fopen("config/language.json", "r");
    if (!file) {
        fprintf(stderr, "Error: Failed to open config/language.json\n");
        return -1;
    }

    // Read file with boundary checking
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Check file size against maximum buffer
    if (size > MAX_LABELS_JSON_SIZE - 1) {
        fclose(file);
        fprintf(stderr, "Error: language.json exceeds maximum size (%ld > %d)\n",
                size, MAX_LABELS_JSON_SIZE - 1);
        return -1;
    }

    static char content[MAX_LABELS_JSON_SIZE];
    size_t bytes_read = fread(content, 1, size, file);
    content[bytes_read] = '\0';
    fclose(file);

    // Parse JSON starting from the language-specific key
    char lang_prefix[MAX_KEY_LENGTH];
    snprintf(lang_prefix, sizeof(lang_prefix), "%s", language);

    // Find the language section in the JSON
    char search_key[256];
    snprintf(search_key, sizeof(search_key), "\"%s\"", language);
    const char* lang_start = strstr(content, search_key);

    if (lang_start) {
        // Move past the key to the value (the opening brace of the language object)
        lang_start += strlen(search_key);
        lang_start = skip_whitespace(lang_start);
        if (*lang_start == ':') {
            lang_start++;
            lang_start = skip_whitespace(lang_start);
            // Now parse the language-specific object
            parse_json_object(lang_start, "");
        }
    }

    if (label_count == 0) {
        fprintf(stderr, "Error: No labels loaded for language: %s\n", language);
        return -1;
    }

    return 0;
}

const char* get_language(void) {
    return current_language;
}

void free_labels(void) {
    label_count = 0;
}
