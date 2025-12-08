#ifndef INPUT_BASE_H
#define INPUT_BASE_H

#include <stddef.h>
#include <stdbool.h>
#include <wchar.h>

// ============================================================================
// CONSTANTS
// ============================================================================

#define MAX_TEXT_LEN 1024
#define INPUT_SPACE_KEY 10
#define INPUT_DELETE_KEY 11

// ============================================================================
// MACROS
// ============================================================================

#define CLAMP_CURSOR(state) do { \
    if ((state)->cursor_pos > MAX_TEXT_LEN) (state)->cursor_pos = MAX_TEXT_LEN; \
    if ((state)->cursor_pos < 0) (state)->cursor_pos = 0; \
} while(0)

// ============================================================================
// TYPES & ENUMERATIONS
// ============================================================================

/**
 * Input mode enumeration
 */
typedef enum {
    MODE_HANGUL = 0,
    MODE_UPPER_ENGLISH = 1,
    MODE_ENGLISH = 2,
    MODE_NUMBER = 3,
    MODE_SPECIAL = 4
} InputMode;

/**
 * Base input state structure
 * Contains common state for all input modes
 */
typedef struct {
    InputMode now_mode;           // Current input mode
    wchar_t text_buffer[MAX_TEXT_LEN];  // Text buffer
    int cursor_pos;               // Cursor position
    wchar_t engnum[16];           // English/number buffer
    bool flag_initengnum;         // English/number initialization flag
    bool flag_engdelete;          // English delete flag
    bool flag_upper;              // Uppercase flag
} InputBaseState;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Convert wide character string to UTF-8
 * @param wstr Wide character string to convert
 * @param max_len Maximum length of the wide string
 * @return UTF-8 encoded string (static buffer, not thread-safe)
 */
char* wchar_to_utf8(const wchar_t *wstr, size_t max_len);

/**
 * Convert wide character to lowercase
 * @param ch Wide character to convert
 * @return Lowercase wide character
 */
wchar_t towlower_simple(wchar_t ch);

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

/**
 * Initialize base input state
 * @param state Pointer to InputBaseState structure (must not be NULL)
 */
void input_base_init(InputBaseState *state);

/**
 * Initialize English/number buffer
 * @param state Pointer to InputBaseState structure (must not be NULL)
 */
void init_engnum(InputBaseState *state);

// ============================================================================
// TEXT OPERATIONS
// ============================================================================

/**
 * Delete character at cursor position
 * @param state Pointer to InputBaseState structure (must not be NULL)
 */
void input_delete_char(InputBaseState *state);

/**
 * Write English/number character to text buffer
 * @param state Pointer to InputBaseState structure (must not be NULL)
 */
void write_engnum(InputBaseState *state);

// ============================================================================
// MODE MANAGEMENT
// ============================================================================

/**
 * Change input mode (Hangul -> English -> Number -> Special)
 * @param state Pointer to InputBaseState structure (must not be NULL)
 */
void change_mode(InputBaseState *state);

/**
 * Get button text for current mode
 * @param mode Current input mode
 * @param button_num Button number (0-11)
 * @return Wide character string for button label
 */
const wchar_t* get_button_text(InputMode mode, int button_num);

// ============================================================================
// INPUT MODE PROCESSING FUNCTIONS
// ============================================================================

/**
 * Process English input
 * @param state Pointer to InputBaseState structure (must not be NULL)
 * @param input Input button number
 */
void eng_make(InputBaseState *state, int input);

/**
 * Process number input
 * @param state Pointer to InputBaseState structure (must not be NULL)
 * @param input Input button number
 */
void num_make(InputBaseState *state, int input);

/**
 * Process special character input
 * @param state Pointer to InputBaseState structure (must not be NULL)
 * @param input Input button number
 */
void special_make(InputBaseState *state, int input);

#endif // INPUT_BASE_H
