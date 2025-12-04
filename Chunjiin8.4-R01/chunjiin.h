#ifndef CHUNJIIN_H
#define CHUNJIIN_H

#include <stddef.h>
#include <stdbool.h>
#include <wchar.h>

/* Constants */
#define MAX_TEXT_LEN 1024
#define JAMO_BUFFER_SIZE 16

/* Helper macros */
#define CLAMP_CURSOR(state) do { \
    if ((state)->cursor_pos > MAX_TEXT_LEN) (state)->cursor_pos = MAX_TEXT_LEN; \
    if ((state)->cursor_pos < 0) (state)->cursor_pos = 0; \
} while(0)

/* Input modes */
typedef enum {
    MODE_HANGUL = 0,
    MODE_UPPER_ENGLISH = 1,
    MODE_ENGLISH = 2,
    MODE_NUMBER = 3,
    MODE_SPECIAL = 4
} InputMode;

/* Hangul composition state */
typedef struct {
    wchar_t chosung[JAMO_BUFFER_SIZE];     /* Initial consonant (초성) */
    wchar_t jungsung[JAMO_BUFFER_SIZE];    /* Vowel (중성) */
    wchar_t jongsung[JAMO_BUFFER_SIZE];    /* Final consonant (종성) */
    wchar_t jongsung2[JAMO_BUFFER_SIZE];   /* Second final consonant for double (겹받침) */
    int step;                               /* Current composition step (0:cho, 1:jung, 2:jong, 3:double) */
    bool flag_writing;                      /* Currently composing character */
    bool flag_dotused;                      /* Dot (·, ‥) was used */
    bool flag_doubled;                      /* Double final consonant active */
    bool flag_addcursor;                    /* Cursor increment needed */
    bool flag_space;                        /* Space key pressed */
} HangulState;

/* Main Chunjiin input state */
typedef struct {
    HangulState hangul;                     /* Hangul composition state */
    InputMode now_mode;                     /* Current input mode */

    wchar_t engnum[JAMO_BUFFER_SIZE];      /* English/number character buffer */
    bool flag_initengnum;                   /* Initialize engnum buffer flag */
    bool flag_engdelete;                    /* English delete flag */
    bool flag_upper;                        /* Uppercase mode flag */

    wchar_t text_buffer[MAX_TEXT_LEN];     /* Main text buffer */
    int cursor_pos;                         /* Cursor position in buffer */
} ChunjiinState;

/* ============================================
 * Utility Functions
 * ============================================ */

/* Convert wide character string to UTF-8 */
char* wchar_to_utf8(const wchar_t *wstr, size_t max_len);

/* ============================================
 * Initialization Functions
 * ============================================ */

/* Initialize Chunjiin state */
void chunjiin_init(ChunjiinState *state);

/* Initialize Hangul composition state */
void hangul_init(HangulState *hangul);

/* Initialize English/number buffer */
void init_engnum(ChunjiinState *state);

/* ============================================
 * Input Processing Functions
 * ============================================ */

/* Main input processor - dispatches to mode-specific handlers */
void chunjiin_process_input(ChunjiinState *state, int input);

/* Process Hangul input */
void hangul_make(ChunjiinState *state, int input);

/* Process English input */
void eng_make(ChunjiinState *state, int input);

/* Process number input */
void num_make(ChunjiinState *state, int input);

/* Process special character input */
void special_make(ChunjiinState *state, int input);

/* ============================================
 * Text Buffer Functions
 * ============================================ */

/* Write Hangul character to buffer */
void write_hangul(ChunjiinState *state);

/* Write English/number character to buffer */
void write_engnum(ChunjiinState *state);

/* Delete character at cursor */
void delete_char(ChunjiinState *state);

/* ============================================
 * Hangul Composition Functions
 * ============================================ */

/* Get Unicode code point for Hangul syllable or Jamo */
int get_unicode(HangulState *hangul, const wchar_t *real_jong);

/* Check and create double final consonant */
void check_double(const wchar_t *jong, const wchar_t *jong2, wchar_t *result);

/* ============================================
 * Mode Management
 * ============================================ */

/* Cycle to next input mode */
void change_mode(ChunjiinState *state);

/* Get button label text for current mode and button number */
const wchar_t* get_button_text(InputMode mode, int button_num);

#endif // CHUNJIIN_H
