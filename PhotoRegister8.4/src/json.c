/**
 * @file json.c
 * @brief Implementation of JSON parsing utilities
 */

#include "../include/json.h"
#include <stdio.h>
#include <string.h>

const char *json_find_closing_brace(const char *opening_brace) {
    if (!opening_brace || *opening_brace != '{') {
        return NULL;
    }

    int depth = 1;
    const char *p = opening_brace + 1;

    while (*p && depth > 0) {
        if (*p == '{') {
            depth++;
        } else if (*p == '}') {
            depth--;
        }
        p++;
    }

    return (depth == 0) ? (p - 1) : NULL;
}

bool json_extract_section(const char *json_str,
                          const char *key,
                          char *output,
                          size_t output_size) {
    if (!json_str || !key || !output || output_size == 0) {
        return false;
    }

    // Build search pattern: "key"
    char search_key[128];
    snprintf(search_key, sizeof(search_key), "\"%s\"", key);

    // Find the key in the JSON string
    const char *key_start = strstr(json_str, search_key);
    if (!key_start) {
        return false;
    }

    // Find the opening brace after the key
    const char *brace = strchr(key_start, '{');
    if (!brace) {
        return false;
    }

    // Find matching closing brace
    const char *closing_brace = json_find_closing_brace(brace);
    if (!closing_brace) {
        return false;
    }

    // Calculate length of section (from key start to after closing brace)
    size_t section_len = (closing_brace + 1) - key_start;

    // Check if output buffer is large enough
    if (section_len >= output_size) {
        return false;
    }

    // Copy section to output buffer
    strncpy(output, key_start, section_len);
    output[section_len] = '\0';

    return true;
}
