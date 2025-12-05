#ifndef WELCOME_H
#define WELCOME_H

// ============================================================================
// WELCOME MESSAGE API
// ============================================================================

/**
 * Load welcome messages from JSON file
 * @return 0 on success, -1 on failure
 */
int welcome_load(void);

/**
 * Get the appropriate welcome message based on current time and language
 * @return Welcome message text or empty string if not found
 */
const char* welcome_get_message(void);

/**
 * Free all loaded welcome messages
 */
void welcome_free(void);

#endif
