#ifndef CHUNJIIN_H
#define CHUNJIIN_H

#include <stddef.h>
#include <stdbool.h>
#include <wchar.h>

// ============================================================================
// CONSTANTS
// ============================================================================

#define MAX_TEXT_LEN 1024
#define CHUNJIIN_SPACE_KEY 10
#define CHUNJIIN_DELETE_KEY 11

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

typedef enum {
    MODE_HANGUL = 0,
    MODE_UPPER_ENGLISH = 1,
    MODE_ENGLISH = 2,
    MODE_NUMBER = 3,
    MODE_SPECIAL = 4
} InputMode;

typedef struct {
    wchar_t chosung[16];      // 초성 (initial consonant)
    wchar_t jungsung[16];     // 중성 (medial vowel)
    wchar_t jongsung[16];     // 종성 (final consonant)
    wchar_t jongsung2[16];    // 종성2 (double final consonant)
    int step;                 // Current step (0: cho, 1: jung, 2: jong, 3: double jong)
    bool flag_writing;        // Currently writing flag
    bool flag_dotused;        // Dot (·, ‥) used flag
    bool flag_doubled;        // Double consonant flag
    bool flag_addcursor;      // Add cursor flag
    bool flag_space;          // Space flag
} HangulState;

typedef struct {
    HangulState hangul;
    InputMode now_mode;

    wchar_t engnum[16];       // English/number buffer
    bool flag_initengnum;     // English/number initialization flag
    bool flag_engdelete;      // English delete flag
    bool flag_upper;          // Uppercase flag

    wchar_t text_buffer[MAX_TEXT_LEN];  // Text buffer
    int cursor_pos;           // Cursor position
} ChunjiinState;

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

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

/**
 * Initialize Chunjiin state
 * @param state Pointer to ChunjiinState structure
 */
void chunjiin_init(ChunjiinState *state);

/**
 * Initialize Hangul state
 * @param hangul Pointer to HangulState structure
 */
void hangul_init(HangulState *hangul);

/**
 * Initialize English/number buffer
 * @param state Pointer to ChunjiinState structure
 */
void init_engnum(ChunjiinState *state);

/**
 * Process input and update state
 * @param state Pointer to ChunjiinState structure
 * @param input Input button number (0-11)
 */
void chunjiin_process_input(ChunjiinState *state, int input);

// ============================================================================
// UNICODE & HANGUL PROCESSING
// ============================================================================

/**
 * Get Unicode code point for Hangul character
 * @param hangul Pointer to HangulState structure
 * @param real_jong Final consonant string
 * @return Unicode code point
 */
int get_unicode(HangulState *hangul, const wchar_t *real_jong);

/**
 * Check and create double final consonant
 * @param jong First final consonant
 * @param jong2 Second final consonant
 * @param result Output buffer for double consonant
 */
void check_double(const wchar_t *jong, const wchar_t *jong2, wchar_t *result);

#endif // CHUNJIIN_H
