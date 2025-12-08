#include "../include/logger.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

// ============================================================================
// LOGGING CONSTANTS
// ============================================================================

#define LOG_DIR "log"
#define LOG_FILE "log/app.log"
#define MAX_LOG_MESSAGE_LENGTH 1024

// ============================================================================
// LOGGING GLOBAL STATE
// ============================================================================

static FILE *log_file = NULL;
static int log_initialized = 0;

// ============================================================================
// LOGGING HELPER FUNCTIONS
// ============================================================================

/**
 * Ensure log directory exists
 */
static int ensure_log_directory(void) {
    struct stat st = {0};

    if (stat(LOG_DIR, &st) == -1) {
        if (mkdir(LOG_DIR, 0755) == -1) {
            fprintf(stderr, "Error: Failed to create log directory: %s\n", strerror(errno));
            return -1;
        }
    }

    return 0;
}

/**
 * Get current timestamp as string
 */
static void get_timestamp(char *buffer, int max_len) {
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    strftime(buffer, max_len, "%Y-%m-%d %H:%M:%S", timeinfo);
}

/**
 * Write formatted message to log file with timestamp
 */
static void log_message(const char *level, const char *format, va_list args) {
    if (!log_initialized || !log_file) {
        return;
    }

    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));

    // Write timestamp and level
    fprintf(log_file, "[%s] %s: ", timestamp, level);

    // Write formatted message
    vfprintf(log_file, format, args);

    // Write newline if not present
    if (format && format[strlen(format) - 1] != '\n') {
        fprintf(log_file, "\n");
    }

    // Flush to ensure data is written
    fflush(log_file);
}

// ============================================================================
// PUBLIC LOGGING API
// ============================================================================

int log_init(void) {
    if (log_initialized) {
        return 0;  // Already initialized
    }

    // Ensure log directory exists
    if (ensure_log_directory() != 0) {
        return -1;
    }

    // Open log file for appending
    log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        fprintf(stderr, "Error: Failed to open log file: %s\n", strerror(errno));
        return -1;
    }

    log_initialized = 1;

    // Write startup message
    //log_info("Application started");

    return 0;
}

void log_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_message("ERROR", format, args);
    va_end(args);
}

void log_warning(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_message("WARNING", format, args);
    va_end(args);
}

void log_info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_message("INFO", format, args);
    va_end(args);
}

void log_debug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_message("DEBUG", format, args);
    va_end(args);
}

void log_close(void) {
    if (log_file) {
    //    log_info("Application closing");
        fclose(log_file);
        log_file = NULL;
    }
    log_initialized = 0;
}
