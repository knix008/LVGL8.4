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

// Jongsung combination lookup table
typedef struct {
    wchar_t combined;
    wchar_t first;
    wchar_t second;
} JongsungCombination;

static const JongsungCombination JONGSUNG_COMBINATIONS[] = {
    {L'ㄳ', L'ㄱ', L'ㅅ'},
    {L'ㄵ', L'ㄴ', L'ㅈ'},
    {L'ㄶ', L'ㄴ', L'ㅎ'},
    {L'ㄺ', L'ㄹ', L'ㄱ'},
    {L'ㄻ', L'ㄹ', L'ㅁ'},
    {L'ㄼ', L'ㄹ', L'ㅂ'},
    {L'ㄽ', L'ㄹ', L'ㅅ'},
    {L'ㄾ', L'ㄹ', L'ㅌ'},
    {L'ㄿ', L'ㄹ', L'ㅍ'},
    {L'ㅀ', L'ㄹ', L'ㅎ'},
    {L'ㅄ', L'ㅂ', L'ㅅ'},
    {0, 0, 0}  // Sentinel
};

// Split combined jongsung into two components
// Returns 1 if jongsung is combined and was split, 0 otherwise
// first and second will contain the split components
static int try_split_jongsung(wchar_t jongsung, wchar_t *first, wchar_t *second) {
    for (int i = 0; JONGSUNG_COMBINATIONS[i].combined != 0; i++) {
        if (JONGSUNG_COMBINATIONS[i].combined == jongsung) {
            *first = JONGSUNG_COMBINATIONS[i].first;
            *second = JONGSUNG_COMBINATIONS[i].second;
            return 1;
        }
    }
    return 0;
}

// Combine two consonants into a double jongsung
// Returns the combined jongsung or 0 if they cannot be combined
static wchar_t try_combine_jongsung(wchar_t first, wchar_t second) {
    for (int i = 0; JONGSUNG_COMBINATIONS[i].combined != 0; i++) {
        if (JONGSUNG_COMBINATIONS[i].first == first &&
            JONGSUNG_COMBINATIONS[i].second == second) {
            return JONGSUNG_COMBINATIONS[i].combined;
        }
    }
    return 0;
}

// Vowel combination lookup table
typedef struct {
    wchar_t first;
    wchar_t second;
    wchar_t combined;
} VowelCombination;

static const VowelCombination VOWEL_COMBINATIONS[] = {
    {L'ㅗ', L'ㅏ', L'ㅘ'},
    {L'ㅗ', L'ㅐ', L'ㅙ'},
    {L'ㅗ', L'ㅣ', L'ㅚ'},
    {L'ㅜ', L'ㅓ', L'ㅝ'},
    {L'ㅜ', L'ㅔ', L'ㅞ'},
    {L'ㅜ', L'ㅣ', L'ㅟ'},
    {L'ㅡ', L'ㅣ', L'ㅢ'},
    {0, 0, 0}  // Sentinel
};

// Try to combine two vowels into a complex vowel
// Returns the combined vowel or 0 if they cannot be combined
static wchar_t try_combine_vowels(wchar_t first, wchar_t second) {
    for (int i = 0; VOWEL_COMBINATIONS[i].combined != 0; i++) {
        if (VOWEL_COMBINATIONS[i].first == first &&
            VOWEL_COMBINATIONS[i].second == second) {
            return VOWEL_COMBINATIONS[i].combined;
        }
    }
    return 0;
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

// Helper function to convert wchar_t to UTF-8 string
static int wchar_to_utf8(wchar_t wch, char *output) {
    int len = wctomb(output, wch);
    if (len > 0) {
        output[len] = '\0';
        return len;
    }
    output[0] = '\0';
    return 0;
}

// Helper function to start a new syllable with a consonant
static void start_new_consonant(QwertyState *state, wchar_t consonant,
                                const char *jamo_str, char *output) {
    state->hangul.cho = consonant;
    state->hangul.jung = 0;
    state->hangul.jong = 0;
    state->hangul.composing = 1;
    strcpy(output, jamo_str);
}

// Helper function to compose and output a syllable
static int compose_and_output(wchar_t cho, wchar_t jung, wchar_t jong, char *output) {
    wchar_t composed = compose_hangul(cho, jung, jong);
    if (composed) {
        return wchar_to_utf8(composed, output);
    }
    output[0] = '\0';
    return 0;
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

// Handle consonant input when starting a new syllable or have only cho
static void handle_consonant_no_jung(QwertyState *state, wchar_t wch,
                                     const char *jamo_str, char *output,
                                     int *delete_previous) {
    if (!state->hangul.composing) {
        start_new_consonant(state, wch, jamo_str, output);
    } else if (state->hangul.jung == 0) {
        *delete_previous = 0;
        start_new_consonant(state, wch, jamo_str, output);
    }
}

// Handle consonant input when have cho+jung (adding jong)
static void handle_consonant_add_jong(QwertyState *state, wchar_t wch,
                                      const char *jamo_str, char *output,
                                      int *delete_previous) {
    *delete_previous = 1;
    state->hangul.jong = wch;
    if (!compose_and_output(state->hangul.cho, state->hangul.jung,
                           state->hangul.jong, output)) {
        *delete_previous = 0;
        start_new_consonant(state, wch, jamo_str, output);
    }
}

// Handle consonant input when have complete syllable (cho+jung+jong)
static void handle_consonant_with_jong(QwertyState *state, wchar_t wch,
                                       const char *jamo_str, char *output,
                                       int *delete_previous) {
    wchar_t double_jong = try_combine_jongsung(state->hangul.jong, wch);
    if (double_jong) {
        *delete_previous = 1;
        state->hangul.jong = double_jong;
        if (!compose_and_output(state->hangul.cho, state->hangul.jung,
                               state->hangul.jong, output)) {
            *delete_previous = 0;
            start_new_consonant(state, wch, jamo_str, output);
        }
    } else {
        start_new_consonant(state, wch, jamo_str, output);
    }
}

// Process consonant character
static void process_consonant(QwertyState *state, wchar_t wch,
                              const char *jamo_str, char *output,
                              int *delete_previous) {
    if (!state->hangul.composing || state->hangul.jung == 0) {
        handle_consonant_no_jung(state, wch, jamo_str, output, delete_previous);
    } else if (state->hangul.jong == 0) {
        handle_consonant_add_jong(state, wch, jamo_str, output, delete_previous);
    } else {
        handle_consonant_with_jong(state, wch, jamo_str, output, delete_previous);
    }
}

// Handle vowel input when have no cho
static void handle_vowel_no_cho(QwertyState *state, const char *jamo_str,
                                char *output) {
    strcpy(output, jamo_str);
    state->hangul.composing = 0;
}

// Handle vowel input when have cho but no jung
static void handle_vowel_add_jung(QwertyState *state, wchar_t wch,
                                  const char *jamo_str, char *output,
                                  int *delete_previous) {
    *delete_previous = 1;
    state->hangul.jung = wch;
    if (!compose_and_output(state->hangul.cho, state->hangul.jung, 0, output)) {
        *delete_previous = 0;
        state->hangul.cho = 0;
        state->hangul.jung = 0;
        state->hangul.jong = 0;
        state->hangul.composing = 0;
        strcpy(output, jamo_str);
    }
}

// Handle vowel input when have complete syllable with jong
static void handle_vowel_split_jong(QwertyState *state, wchar_t wch,
                                    char *output, int *delete_previous) {
    *delete_previous = 1;

    wchar_t first_jong, second_jong;
    char utf8_syllable[7] = {0};
    char utf8_new[7] = {0};

    if (try_split_jongsung(state->hangul.jong, &first_jong, &second_jong)) {
        compose_and_output(state->hangul.cho, state->hangul.jung, first_jong, utf8_syllable);
        strcpy(output, utf8_syllable);

        state->hangul.cho = second_jong;
        state->hangul.jung = wch;
        state->hangul.jong = 0;
        compose_and_output(state->hangul.cho, state->hangul.jung, 0, utf8_new);
        strcat(output, utf8_new);
    } else {
        compose_and_output(state->hangul.cho, state->hangul.jung, 0, utf8_syllable);
        strcpy(output, utf8_syllable);

        state->hangul.cho = state->hangul.jong;
        state->hangul.jung = wch;
        state->hangul.jong = 0;
        compose_and_output(state->hangul.cho, state->hangul.jung, 0, utf8_new);
        strcat(output, utf8_new);
    }
}

// Handle vowel input when have cho+jung (try to combine vowels)
static void handle_vowel_combine(QwertyState *state, wchar_t wch,
                                 const char *jamo_str, char *output,
                                 int *delete_previous) {
    wchar_t combined_vowel = try_combine_vowels(state->hangul.jung, wch);
    if (combined_vowel) {
        *delete_previous = 1;
        state->hangul.jung = combined_vowel;
        if (!compose_and_output(state->hangul.cho, state->hangul.jung, 0, output)) {
            *delete_previous = 0;
            state->hangul.cho = 0;
            state->hangul.jung = 0;
            state->hangul.jong = 0;
            state->hangul.composing = 0;
            strcpy(output, jamo_str);
        }
    } else {
        state->hangul.cho = 0;
        state->hangul.jung = 0;
        state->hangul.jong = 0;
        state->hangul.composing = 0;
        strcpy(output, jamo_str);
    }
}

// Process vowel character
static void process_vowel(QwertyState *state, wchar_t wch,
                          const char *jamo_str, char *output,
                          int *delete_previous) {
    if (!state->hangul.composing || state->hangul.cho == 0) {
        handle_vowel_no_cho(state, jamo_str, output);
    } else if (state->hangul.jung == 0) {
        handle_vowel_add_jung(state, wch, jamo_str, output, delete_previous);
    } else if (state->hangul.jong != 0) {
        handle_vowel_split_jong(state, wch, output, delete_previous);
    } else {
        handle_vowel_combine(state, wch, jamo_str, output, delete_previous);
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
        process_consonant(state, wch, jamo_str, output, delete_previous);
    } else if (qwerty_is_vowel(wch)) {
        process_vowel(state, wch, jamo_str, output, delete_previous);
    } else {
        strcpy(output, jamo_str);
        state->hangul.composing = 0;
    }
}
