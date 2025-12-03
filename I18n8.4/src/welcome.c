#include "../include/welcome.h"
#include "../include/config.h"
#include "../include/label.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// ============================================================================
// WELCOME MESSAGE STORAGE
// ============================================================================

#define MAX_WELCOME_MESSAGES 4  // morning, afternoon, evening, night
#define MAX_MESSAGE_LENGTH 256

typedef struct {
    char morning[MAX_MESSAGE_LENGTH];
    char afternoon[MAX_MESSAGE_LENGTH];
    char evening[MAX_MESSAGE_LENGTH];
    char night[MAX_MESSAGE_LENGTH];
} WelcomeMessages;

static WelcomeMessages welcome_messages = {0};
static int messages_loaded = 0;

// ============================================================================
// JSON PARSING HELPERS (simplified version)
// ============================================================================

static const char* skip_whitespace(const char* str) {
    while (*str && isspace(*str)) str++;
    return str;
}

static const char* skip_string(const char* str) {
    if (*str != '\"') return str;
    str++;  // Skip opening quote
    while (*str && *str != '\"') {
        if (*str == '\\' && *(str + 1)) str++;  // Skip escaped char
        str++;
    }
    if (*str == '\"') str++;  // Skip closing quote
    return str;
}

static int extract_string(const char* str, char* buffer, int max_len) {
    str = skip_whitespace(str);
    if (*str != '\"') return -1;

    str++;  // Skip opening quote
    int i = 0;
    while (*str && *str != '\"' && i < max_len - 1) {
        if (*str == '\\') {
            str++;
            if (*str == 'n') buffer[i++] = '\n';
            else if (*str == 't') buffer[i++] = '\t';
            else if (*str == 'r') buffer[i++] = '\r';
            else if (*str == '\\') buffer[i++] = '\\';
            else if (*str == '\"') buffer[i++] = '\"';
            else buffer[i++] = *str;
            str++;
        } else {
            buffer[i++] = *str++;
        }
    }
    buffer[i] = '\0';
    return 0;
}

static const char* find_key_value(const char* json, const char* key, char* value, int max_len) {
    // Search for "key":
    char search_str[256];
    snprintf(search_str, sizeof(search_str), "\"%s\"", key);

    const char* pos = strstr(json, search_str);
    if (!pos) return NULL;

    // Move past the key
    pos += strlen(search_str);
    pos = skip_whitespace(pos);

    // Expect ':'
    if (*pos != ':') return NULL;
    pos++;
    pos = skip_whitespace(pos);

    // Extract the string value
    if (extract_string(pos, value, max_len) == 0) {
        return skip_string(pos);
    }

    return NULL;
}

// ============================================================================
// WELCOME MESSAGE API
// ============================================================================

int welcome_load(void) {
    const char *language = get_language();
    if (!language) return -1;

    FILE* file = fopen("config/welcome.json", "r");
    if (!file) {
        fprintf(stderr, "Error: Failed to open config/welcome.json\n");
        return -1;
    }

    // Read file with boundary checking
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Check file size against maximum buffer
    if (size > MAX_WELCOME_JSON_SIZE - 1) {
        fclose(file);
        fprintf(stderr, "Error: welcome.json exceeds maximum size (%ld > %d)\n",
                size, MAX_WELCOME_JSON_SIZE - 1);
        return -1;
    }

    static char content[MAX_WELCOME_JSON_SIZE];
    size_t bytes_read = fread(content, 1, size, file);
    content[bytes_read] = '\0';
    fclose(file);

    // Find language section
    char lang_search[256];
    snprintf(lang_search, sizeof(lang_search), "\"%s\"", language);
    const char* lang_pos = strstr(content, lang_search);

    if (!lang_pos) {
        fprintf(stderr, "Error: Language section not found in welcome.json\n");
        return -1;
    }

    // Move to language object
    lang_pos += strlen(lang_search);
    lang_pos = skip_whitespace(lang_pos);
    if (*lang_pos != ':') {
        return -1;
    }
    lang_pos++;
    lang_pos = skip_whitespace(lang_pos);
    if (*lang_pos != '{') {
        return -1;
    }

    // Extract messages
    memset(&welcome_messages, 0, sizeof(welcome_messages));

    find_key_value(lang_pos, "morning", welcome_messages.morning, sizeof(welcome_messages.morning));
    find_key_value(lang_pos, "afternoon", welcome_messages.afternoon, sizeof(welcome_messages.afternoon));
    find_key_value(lang_pos, "evening", welcome_messages.evening, sizeof(welcome_messages.evening));
    find_key_value(lang_pos, "night", welcome_messages.night, sizeof(welcome_messages.night));

    messages_loaded = 1;
    return 0;
}

const char* welcome_get_message(void) {
    if (!messages_loaded) {
        return "";
    }

    // Get current hour
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    int hour = timeinfo->tm_hour;

    // Select message based on time period
    if (hour >= WELCOME_MORNING_START_HOUR && hour < WELCOME_MORNING_END_HOUR) {
        return welcome_messages.morning;
    } else if (hour >= WELCOME_AFTERNOON_START_HOUR && hour < WELCOME_AFTERNOON_END_HOUR) {
        return welcome_messages.afternoon;
    } else if (hour >= WELCOME_EVENING_START_HOUR && hour < WELCOME_EVENING_END_HOUR) {
        return welcome_messages.evening;
    } else {
        return welcome_messages.night;
    }
}

void welcome_free(void) {
    messages_loaded = 0;
    memset(&welcome_messages, 0, sizeof(welcome_messages));
}
