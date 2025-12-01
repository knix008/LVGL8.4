#include "qwerty.h"
#include <string.h>
#include <stdlib.h>
#include <locale.h>

// Hangul constants
#define HANGUL_BASE 0xAC00
#define CHO_COUNT 19
#define JUNG_COUNT 21
#define JONG_COUNT 28

// Hangul jamo tables
static const wchar_t CHO_JAMO[] = {
    L'ㄱ', L'ㄲ', L'ㄴ', L'ㄷ', L'ㄸ', L'ㄹ', L'ㅁ', L'ㅂ', L'ㅃ', L'ㅅ',
    L'ㅆ', L'ㅇ', L'ㅈ', L'ㅉ', L'ㅊ', L'ㅋ', L'ㅌ', L'ㅍ', L'ㅎ'
};

static const wchar_t JUNG_JAMO[] = {
    L'ㅏ', L'ㅐ', L'ㅑ', L'ㅒ', L'ㅓ', L'ㅔ', L'ㅕ', L'ㅖ', L'ㅗ', L'ㅘ',
    L'ㅙ', L'ㅚ', L'ㅛ', L'ㅜ', L'ㅝ', L'ㅞ', L'ㅟ', L'ㅠ', L'ㅡ', L'ㅢ', L'ㅣ'
};

static const wchar_t JONG_JAMO[] = {
    0, L'ㄱ', L'ㄲ', L'ㄳ', L'ㄴ', L'ㄵ', L'ㄶ', L'ㄷ', L'ㄹ', L'ㄺ',
    L'ㄻ', L'ㄼ', L'ㄽ', L'ㄾ', L'ㄿ', L'ㅀ', L'ㅁ', L'ㅂ', L'ㅄ', L'ㅅ',
    L'ㅆ', L'ㅇ', L'ㅈ', L'ㅊ', L'ㅋ', L'ㅌ', L'ㅍ', L'ㅎ'
};

// Key mappings for QWERTY layout
KeyMap key_maps[47] = {
    // Row 0
    {"`", "~", "`", "~"},
    {"1", "!", "1", "!"},
    {"2", "@", "2", "@"},
    {"3", "#", "3", "#"},
    {"4", "$", "4", "$"},
    {"5", "%", "5", "%"},
    {"6", "^", "6", "^"},
    {"7", "&", "7", "&"},
    {"8", "*", "8", "*"},
    {"9", "(", "9", "("},
    {"0", ")", "0", ")"},
    {"-", "_", "-", "_"},
    {"=", "+", "=", "+"},
    // Row 1
    {"q", "Q", "ㅂ", "ㅃ"},
    {"w", "W", "ㅈ", "ㅉ"},
    {"e", "E", "ㄷ", "ㄸ"},
    {"r", "R", "ㄱ", "ㄲ"},
    {"t", "T", "ㅅ", "ㅆ"},
    {"y", "Y", "ㅛ", "ㅛ"},
    {"u", "U", "ㅕ", "ㅕ"},
    {"i", "I", "ㅑ", "ㅑ"},
    {"o", "O", "ㅐ", "ㅒ"},
    {"p", "P", "ㅔ", "ㅖ"},
    {"[", "{", "[", "{"},
    {"]", "}", "]", "}"},
    {"\\", "|", "\\", "|"},
    // Row 2
    {"a", "A", "ㅁ", "ㅁ"},
    {"s", "S", "ㄴ", "ㄴ"},
    {"d", "D", "ㅇ", "ㅇ"},
    {"f", "F", "ㄹ", "ㄹ"},
    {"g", "G", "ㅎ", "ㅎ"},
    {"h", "H", "ㅗ", "ㅗ"},
    {"j", "J", "ㅓ", "ㅓ"},
    {"k", "K", "ㅏ", "ㅏ"},
    {"l", "L", "ㅣ", "ㅣ"},
    {";", ":", ";", ":"},
    {"'", "\"", "'", "\""},
    // Row 3
    {"z", "Z", "ㅋ", "ㅋ"},
    {"x", "X", "ㅌ", "ㅌ"},
    {"c", "C", "ㅊ", "ㅊ"},
    {"v", "V", "ㅍ", "ㅍ"},
    {"b", "B", "ㅠ", "ㅠ"},
    {"n", "N", "ㅜ", "ㅜ"},
    {"m", "M", "ㅡ", "ㅡ"},
    {",", "<", ",", "<"},
    {".", ">", ".", ">"},
    {"/", "?", "/", "?"},
};

// Check if character is a consonant (초성/종성)
int qwerty_is_consonant(wchar_t ch) {
    for (int i = 0; i < CHO_COUNT; i++) {
        if (CHO_JAMO[i] == ch) return 1;
    }
    return 0;
}

// Check if character is a vowel (중성)
int qwerty_is_vowel(wchar_t ch) {
    for (int i = 0; i < JUNG_COUNT; i++) {
        if (JUNG_JAMO[i] == ch) return 1;
    }
    return 0;
}

// Get index in cho/jung/jong arrays
static int get_cho_index(wchar_t ch) {
    for (int i = 0; i < CHO_COUNT; i++) {
        if (CHO_JAMO[i] == ch) return i;
    }
    return -1;
}

static int get_jung_index(wchar_t ch) {
    for (int i = 0; i < JUNG_COUNT; i++) {
        if (JUNG_JAMO[i] == ch) return i;
    }
    return -1;
}

static int get_jong_index(wchar_t ch) {
    for (int i = 1; i < JONG_COUNT; i++) {
        if (JONG_JAMO[i] == ch) return i;
    }
    return -1;
}

// Split combined jongsung into two components
// Returns 1 if jongsung is combined and was split, 0 otherwise
// first and second will contain the split components
static int try_split_jongsung(wchar_t jongsung, wchar_t *first, wchar_t *second) {
    if (jongsung == L'ㄳ') { *first = L'ㄱ'; *second = L'ㅅ'; return 1; }
    if (jongsung == L'ㄵ') { *first = L'ㄴ'; *second = L'ㅈ'; return 1; }
    if (jongsung == L'ㄶ') { *first = L'ㄴ'; *second = L'ㅎ'; return 1; }
    if (jongsung == L'ㄺ') { *first = L'ㄹ'; *second = L'ㄱ'; return 1; }
    if (jongsung == L'ㄻ') { *first = L'ㄹ'; *second = L'ㅁ'; return 1; }
    if (jongsung == L'ㄼ') { *first = L'ㄹ'; *second = L'ㅂ'; return 1; }
    if (jongsung == L'ㄽ') { *first = L'ㄹ'; *second = L'ㅅ'; return 1; }
    if (jongsung == L'ㄾ') { *first = L'ㄹ'; *second = L'ㅌ'; return 1; }
    if (jongsung == L'ㄿ') { *first = L'ㄹ'; *second = L'ㅍ'; return 1; }
    if (jongsung == L'ㅀ') { *first = L'ㄹ'; *second = L'ㅎ'; return 1; }
    if (jongsung == L'ㅄ') { *first = L'ㅂ'; *second = L'ㅅ'; return 1; }
    return 0;  // Not a combined jongsung
}

// Try to combine two vowels into a complex vowel
// Returns the combined vowel or 0 if they cannot be combined
static wchar_t try_combine_vowels(wchar_t first, wchar_t second) {
    // Complex vowel combinations
    if (first == L'ㅗ') {
        if (second == L'ㅏ') return L'ㅘ';
        if (second == L'ㅐ') return L'ㅙ';
        if (second == L'ㅣ') return L'ㅚ';
    }
    if (first == L'ㅜ') {
        if (second == L'ㅓ') return L'ㅝ';
        if (second == L'ㅔ') return L'ㅞ';
        if (second == L'ㅣ') return L'ㅟ';
    }
    if (first == L'ㅡ' && second == L'ㅣ') return L'ㅢ';

    return 0;  // Cannot combine
}

// Compose Hangul syllable from cho, jung, jong
static wchar_t compose_hangul(wchar_t cho, wchar_t jung, wchar_t jong) {
    int cho_idx = get_cho_index(cho);
    int jung_idx = get_jung_index(jung);
    int jong_idx = (jong == 0) ? 0 : get_jong_index(jong);

    if (cho_idx < 0 || jung_idx < 0 || jong_idx < 0) return 0;

    return HANGUL_BASE + (cho_idx * JUNG_COUNT * JONG_COUNT) +
           (jung_idx * JONG_COUNT) + jong_idx;
}

// Initialize qwerty state
void qwerty_init(QwertyState *state) {
    state->current_language = LANG_ENGLISH;
    state->shift_pressed = 0;
    state->caps_lock = 0;
    state->hangul.cho = 0;
    state->hangul.jung = 0;
    state->hangul.jong = 0;
    state->hangul.composing = 0;
}

// Reset composition state
void qwerty_reset_composition(QwertyState *state) {
    state->hangul.cho = 0;
    state->hangul.jung = 0;
    state->hangul.jong = 0;
    state->hangul.composing = 0;
}

// Get the appropriate character for a key
const char* qwerty_get_key_char(QwertyState *state, KeyMap *key_map) {
    if (state->current_language == LANG_KOREAN) {
        if (state->shift_pressed || state->caps_lock) {
            return key_map->korean_shift;
        } else {
            return key_map->korean;
        }
    } else {
        if (state->shift_pressed || state->caps_lock) {
            return key_map->shift;
        } else {
            return key_map->normal;
        }
    }
}

// Process Korean character with composition
// output: buffer to store the result (should be at least 7 bytes)
// delete_previous: set to 1 if previous character should be deleted first
void qwerty_process_korean_char(QwertyState *state, const char *jamo_str,
                                 char *output, int *delete_previous) {
    wchar_t wch;
    *delete_previous = 0;
    output[0] = '\0';

    mbtowc(&wch, jamo_str, MB_CUR_MAX);

    if (qwerty_is_consonant(wch)) {
        if (!state->hangul.composing) {
            // Start new syllable with cho
            state->hangul.cho = wch;
            state->hangul.jung = 0;
            state->hangul.jong = 0;
            state->hangul.composing = 1;
            strcpy(output, jamo_str);
        } else if (state->hangul.jung == 0) {
            // Already have cho without jung, can't add another consonant
            // Keep the previous consonant and start new syllable
            *delete_previous = 0;  // Don't delete - keep previous consonant
            state->hangul.cho = wch;
            state->hangul.jung = 0;
            state->hangul.jong = 0;
            state->hangul.composing = 1;  // Keep composing for next vowel
            strcpy(output, jamo_str);
        } else if (state->hangul.jong == 0) {
            // Have cho+jung, add jong
            *delete_previous = 1;
            state->hangul.jong = wch;
            wchar_t composed = compose_hangul(state->hangul.cho, state->hangul.jung, state->hangul.jong);
            if (composed) {
                int len = wctomb(output, composed);
                if (len > 0) output[len] = '\0';
            } else {
                // Can't compose jong, keep previous syllable and start new
                *delete_previous = 0;  // Keep previous character
                state->hangul.cho = wch;  // Start new syllable with this consonant
                state->hangul.jung = 0;
                state->hangul.jong = 0;
                state->hangul.composing = 1;
                strcpy(output, jamo_str);
            }
        } else {
            // Try to combine jong + new consonant into a double jongseong
            wchar_t double_jong = 0;
            // Check for valid double jongseong (겹받침)
            // List of valid combinations (from Unicode Hangul Jamo table)
            if ((state->hangul.jong == L'ㄱ' && wch == L'ㅅ')) double_jong = L'ㄳ';
            else if ((state->hangul.jong == L'ㄴ' && wch == L'ㅈ')) double_jong = L'ㄵ';
            else if ((state->hangul.jong == L'ㄴ' && wch == L'ㅎ')) double_jong = L'ㄶ';
            else if ((state->hangul.jong == L'ㄹ' && wch == L'ㄱ')) double_jong = L'ㄺ';
            else if ((state->hangul.jong == L'ㄹ' && wch == L'ㅁ')) double_jong = L'ㄻ';
            else if ((state->hangul.jong == L'ㄹ' && wch == L'ㅂ')) double_jong = L'ㄼ';
            else if ((state->hangul.jong == L'ㄹ' && wch == L'ㅅ')) double_jong = L'ㄽ';
            else if ((state->hangul.jong == L'ㄹ' && wch == L'ㅌ')) double_jong = L'ㄾ';
            else if ((state->hangul.jong == L'ㄹ' && wch == L'ㅍ')) double_jong = L'ㄿ';
            else if ((state->hangul.jong == L'ㄹ' && wch == L'ㅎ')) double_jong = L'ㅀ';
            else if ((state->hangul.jong == L'ㅂ' && wch == L'ㅅ')) double_jong = L'ㅄ';
            if (double_jong) {
                // Valid double jongseong
                *delete_previous = 1;
                state->hangul.jong = double_jong;
                wchar_t composed = compose_hangul(state->hangul.cho, state->hangul.jung, state->hangul.jong);
                if (composed) {
                    int len = wctomb(output, composed);
                    if (len > 0) output[len] = '\0';
                } else {
                    // Fallback: treat as new syllable
                    *delete_previous = 0;
                    state->hangul.cho = wch;
                    state->hangul.jung = 0;
                    state->hangul.jong = 0;
                    state->hangul.composing = 1;
                    strcpy(output, jamo_str);
                }
            } else {
                // Already have complete syllable, start new one
                state->hangul.cho = wch;
                state->hangul.jung = 0;
                state->hangul.jong = 0;
                state->hangul.composing = 1;  // Important: keep composing for next vowel
                strcpy(output, jamo_str);
            }
        }
    } else if (qwerty_is_vowel(wch)) {
        if (!state->hangul.composing || state->hangul.cho == 0) {
            // No cho, just insert vowel
            strcpy(output, jamo_str);
            state->hangul.composing = 0;
        } else if (state->hangul.jung == 0) {
            // Have cho, add jung to compose
            *delete_previous = 1;
            state->hangul.jung = wch;
            wchar_t composed = compose_hangul(state->hangul.cho, state->hangul.jung, 0);
            if (composed) {
                int len = wctomb(output, composed);
                if (len > 0) output[len] = '\0';
            } else {
                // Can't compose cho+jung, keep cho and just add vowel separately
                *delete_previous = 0;  // Keep the consonant
                state->hangul.cho = 0;
                state->hangul.jung = 0;
                state->hangul.jong = 0;
                state->hangul.composing = 0;
                strcpy(output, jamo_str);
            }
        } else if (state->hangul.jong != 0) {
            // Have complete syllable with jongsung, need to handle new vowel
            *delete_previous = 1;

            // Check if jong is a combined jongsung that needs to be split
            wchar_t first_jong, second_jong;
            if (try_split_jongsung(state->hangul.jong, &first_jong, &second_jong)) {
                // Combined jongsung: split it
                // First part stays as jong of current syllable
                wchar_t composed = compose_hangul(state->hangul.cho, state->hangul.jung, first_jong);
                char utf8_syllable[7] = {0};
                int len1 = wctomb(utf8_syllable, composed);
                if (len1 > 0) utf8_syllable[len1] = '\0';
                strcpy(output, utf8_syllable);

                // Second part becomes cho of new syllable with the new vowel
                state->hangul.cho = second_jong;
                state->hangul.jung = wch;
                state->hangul.jong = 0;
                wchar_t new_composed = compose_hangul(state->hangul.cho, state->hangul.jung, 0);
                char utf8_new[7] = {0};
                int len2 = wctomb(utf8_new, new_composed);
                if (len2 > 0) utf8_new[len2] = '\0';
                strcat(output, utf8_new);
            } else {
                // Simple jongsung: move it to next syllable as cho
                wchar_t composed = compose_hangul(state->hangul.cho, state->hangul.jung, 0);
                char utf8_syllable[7] = {0};
                int len1 = wctomb(utf8_syllable, composed);
                if (len1 > 0) utf8_syllable[len1] = '\0';
                strcpy(output, utf8_syllable);

                // Start new syllable with jong as cho
                state->hangul.cho = state->hangul.jong;
                state->hangul.jung = wch;
                state->hangul.jong = 0;
                wchar_t new_composed = compose_hangul(state->hangul.cho, state->hangul.jung, 0);
                char utf8_new[7] = {0};
                int len2 = wctomb(utf8_new, new_composed);
                if (len2 > 0) utf8_new[len2] = '\0';
                strcat(output, utf8_new);
            }
        } else {
            // Already have cho+jung, try to combine vowels
            wchar_t combined_vowel = try_combine_vowels(state->hangul.jung, wch);
            if (combined_vowel) {
                // Can combine vowels into complex vowel
                *delete_previous = 1;
                state->hangul.jung = combined_vowel;
                wchar_t composed = compose_hangul(state->hangul.cho, state->hangul.jung, 0);
                if (composed) {
                    int len = wctomb(output, composed);
                    if (len > 0) output[len] = '\0';
                } else {
                    // Can't compose with complex vowel, keep previous and start new
                    *delete_previous = 0;  // Keep previous character
                    state->hangul.cho = 0;
                    state->hangul.jung = 0;
                    state->hangul.jong = 0;
                    state->hangul.composing = 0;
                    strcpy(output, jamo_str);
                }
            } else {
                // Can't combine vowels, start new syllable with this vowel
                state->hangul.cho = 0;
                state->hangul.jung = 0;
                state->hangul.jong = 0;
                state->hangul.composing = 0;
                strcpy(output, jamo_str);
            }
        }
    } else {
        // Not a jamo, just insert
        strcpy(output, jamo_str);
        state->hangul.composing = 0;
    }
}
