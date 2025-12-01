#ifndef QWERTY_H
#define QWERTY_H

#include <wchar.h>

// Language modes
typedef enum {
    LANG_ENGLISH,
    LANG_KOREAN
} LanguageMode;

// Key mapping structure
typedef struct {
    const char *normal;
    const char *shift;
    const char *korean;
    const char *korean_shift;
} KeyMap;

// Hangul composition state
typedef struct {
    wchar_t cho;      // Initial consonant (초성)
    wchar_t jung;     // Vowel (중성)
    wchar_t jong;     // Final consonant (종성)
    int composing;
} HangulState;

// Qwerty keyboard state
typedef struct {
    LanguageMode current_language;
    int shift_pressed;
    int caps_lock;
    HangulState hangul;
} QwertyState;

// Function declarations
void qwerty_init(QwertyState *state);
const char* qwerty_get_key_char(QwertyState *state, KeyMap *key_map);
void qwerty_process_korean_char(QwertyState *state, const char *jamo_str,
                                 char *output, int *delete_previous);
void qwerty_reset_composition(QwertyState *state);
int qwerty_is_consonant(wchar_t ch);
int qwerty_is_vowel(wchar_t ch);

// Key mappings array (47 keys)
extern KeyMap key_maps[47];

#endif // QWERTY_H
