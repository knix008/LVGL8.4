#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <time.h>

// ============================================================================
// LOGGING SYSTEM
// ============================================================================

/**
 * Initialize the logging system.
 * Creates the log directory if it doesn't exist and opens the log file.
 * Must be called once at application startup before any logging.
 *
 * @return 0 on success, -1 on failure
 */
int log_init(void);

/**
 * Write an error message to the log file with timestamp.
 * Format: [YYYY-MM-DD HH:MM:SS] ERROR: message
 *
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void log_error(const char *format, ...);

/**
 * Write a warning message to the log file with timestamp.
 * Format: [YYYY-MM-DD HH:MM:SS] WARNING: message
 *
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void log_warning(const char *format, ...);

/**
 * Write an info message to the log file with timestamp.
 * Format: [YYYY-MM-DD HH:MM:SS] INFO: message
 *
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void log_info(const char *format, ...);

/**
 * Write a debug message to the log file with timestamp.
 * Format: [YYYY-MM-DD HH:MM:SS] DEBUG: message
 *
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void log_debug(const char *format, ...);

/**
 * Close the logging system and flush all pending writes.
 * Should be called before application exit.
 */
void log_close(void);

#endif
