#include <stddef.h>
#include <wchar.h>
#include <stdint.h>
#include "chunjiin.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ============================================
 * Constants and Lookup Tables
 * ============================================ */

/* Compatibility Jamo for standalone display */
static const int COMPAT_CHO[] = {
    0x3131, 0x3132, 0x3134, 0x3137, 0x3138, 0x3139, 0x3141, 0x3142,
    0x3143, 0x3145, 0x3146, 0x3147, 0x3148, 0x3149, 0x314A, 0x314B,
    0x314C, 0x314D, 0x314E
};

static const int COMPAT_JUNG[] = {
    0x314F, 0x3150, 0x3151, 0x3152, 0x3153, 0x3154, 0x3155, 0x3156,
    0x3157, 0x3158, 0x3159, 0x315A, 0x315B, 0x315C, 0x315D, 0x315E,
    0x315F, 0x3160, 0x3161, 0x3162, 0x3163
};

static const int COMPAT_JONG[] = {
    0, 0x3131, 0x3132, 0x3133, 0x3134, 0x3135, 0x3136, 0x3137, 0x3139,
    0x313A, 0x313B, 0x313C, 0x313D, 0x313E, 0x313F, 0x3140, 0x3141, 0x3142,
    0x3144, 0x3145, 0x3146, 0x3147, 0x3148, 0x314A, 0x314B, 0x314C, 0x314D, 0x314E
};

/* Unicode base for Hangul syllables */
#define HANGUL_SYLLABLE_BASE 44032  /* 0xAC00 */
#define CHO_COUNT 19
#define JUNG_COUNT 21
#define JONG_COUNT 28

/* ============================================
 * Helper Functions
 * ============================================ */

/* Lookup table structure for Hangul Jamo mapping */
typedef struct {
    const wchar_t *jamo;
    int index;
} JamoMap;

/* Chosung lookup table */
static const JamoMap CHO_MAP[] = {
    {L"ㄱ", 0}, {L"ㄲ", 1}, {L"ㄴ", 2}, {L"ㄷ", 3}, {L"ㄸ", 4},
    {L"ㄹ", 5}, {L"ㅁ", 6}, {L"ㅂ", 7}, {L"ㅃ", 8}, {L"ㅅ", 9},
    {L"ㅆ", 10}, {L"ㅇ", 11}, {L"ㅈ", 12}, {L"ㅉ", 13}, {L"ㅊ", 14},
    {L"ㅋ", 15}, {L"ㅌ", 16}, {L"ㅍ", 17}, {L"ㅎ", 18}, {NULL, -1}
};

/* Jungsung lookup table */
static const JamoMap JUNG_MAP[] = {
    {L"ㅏ", 0}, {L"ㅐ", 1}, {L"ㅑ", 2}, {L"ㅒ", 3}, {L"ㅓ", 4},
    {L"ㅔ", 5}, {L"ㅕ", 6}, {L"ㅖ", 7}, {L"ㅗ", 8}, {L"ㅘ", 9},
    {L"ㅙ", 10}, {L"ㅚ", 11}, {L"ㅛ", 12}, {L"ㅜ", 13}, {L"ㅝ", 14},
    {L"ㅞ", 15}, {L"ㅟ", 16}, {L"ㅠ", 17}, {L"ㅡ", 18}, {L"ㅢ", 19},
    {L"ㅣ", 20}, {NULL, -1}
};

/* Jongsung lookup table */
static const JamoMap JONG_MAP[] = {
    {L"", 0}, {L"ㄱ", 1}, {L"ㄲ", 2}, {L"ㄳ", 3}, {L"ㄴ", 4},
    {L"ㄵ", 5}, {L"ㄶ", 6}, {L"ㄷ", 7}, {L"ㄹ", 8}, {L"ㄺ", 9},
    {L"ㄻ", 10}, {L"ㄼ", 11}, {L"ㄽ", 12}, {L"ㄾ", 13}, {L"ㄿ", 14},
    {L"ㅀ", 15}, {L"ㅁ", 16}, {L"ㅂ", 17}, {L"ㅄ", 18}, {L"ㅅ", 19},
    {L"ㅆ", 20}, {L"ㅇ", 21}, {L"ㅈ", 22}, {L"ㅊ", 23}, {L"ㅋ", 24},
    {L"ㅌ", 25}, {L"ㅍ", 26}, {L"ㅎ", 27}, {NULL, -1}
};

/* Find Jamo index from lookup table */
static int find_jamo_index(const JamoMap *map, const wchar_t *jamo) {
    for (int i = 0; map[i].jamo != NULL; i++) {
        if (wcscmp(map[i].jamo, jamo) == 0) {
            return map[i].index;
        }
    }
    return -1;
}

/* Check if string is a dot character */
static bool is_dot(const wchar_t *str) {
    return wcscmp(str, L"·") == 0 || wcscmp(str, L"‥") == 0;
}

/* ============================================
 * UTF-8 Conversion
 * ============================================ */

char* wchar_to_utf8(const wchar_t *wstr, size_t max_len) {
    if (wstr == NULL) {
        return "";
    }

    static char buffer[MAX_TEXT_LEN * 4];  /* UTF-8 can use up to 4 bytes per character */
    char *ptr = buffer;

    for (size_t i = 0; i < max_len && wstr[i] != 0; i++) {
        uint32_t uc = (uint32_t)wstr[i];

        if (uc < 0x80) {
            *ptr++ = (char)uc;
        } else if (uc < 0x800) {
            *ptr++ = (char)(0xC0 | (uc >> 6));
            *ptr++ = (char)(0x80 | (uc & 0x3F));
        } else if (uc < 0x10000) {
            *ptr++ = (char)(0xE0 | (uc >> 12));
            *ptr++ = (char)(0x80 | ((uc >> 6) & 0x3F));
            *ptr++ = (char)(0x80 | (uc & 0x3F));
        } else {
            *ptr++ = (char)(0xF0 | (uc >> 18));
            *ptr++ = (char)(0x80 | ((uc >> 12) & 0x3F));
            *ptr++ = (char)(0x80 | ((uc >> 6) & 0x3F));
            *ptr++ = (char)(0x80 | (uc & 0x3F));
        }
    }
    *ptr = '\0';
    return buffer;
}

/* ============================================
 * Initialization Functions
 * ============================================ */

void chunjiin_init(ChunjiinState *state) {
    hangul_init(&state->hangul);
    state->now_mode = MODE_HANGUL;
    init_engnum(state);
    memset(state->text_buffer, 0, sizeof(state->text_buffer));
    state->cursor_pos = 0;
    CLAMP_CURSOR(state);
}

void hangul_init(HangulState *hangul) {
    memset(hangul->chosung, 0, sizeof(hangul->chosung));
    memset(hangul->jungsung, 0, sizeof(hangul->jungsung));
    memset(hangul->jongsung, 0, sizeof(hangul->jongsung));
    memset(hangul->jongsung2, 0, sizeof(hangul->jongsung2));
    hangul->step = 0;
    hangul->flag_writing = false;
    hangul->flag_dotused = false;
    hangul->flag_doubled = false;
    hangul->flag_addcursor = false;
    hangul->flag_space = false;
}

void init_engnum(ChunjiinState *state) {
    memset(state->engnum, 0, sizeof(state->engnum));
    state->flag_initengnum = false;
    state->flag_engdelete = false;
}

/* ============================================
 * Input Processing Functions
 * ============================================ */

void chunjiin_process_input(ChunjiinState *state, int input) {
    if (input < 0 || input > 11) return;

    if (state->now_mode == MODE_HANGUL) {
        hangul_make(state, input);
        write_hangul(state);
    } else if (state->now_mode == MODE_ENGLISH || state->now_mode == MODE_UPPER_ENGLISH) {
        eng_make(state, input);
        write_engnum(state);
    } else if (state->now_mode == MODE_NUMBER) {
        num_make(state, input);
        write_engnum(state);
    } else { /* MODE_SPECIAL */
        special_make(state, input);
        write_engnum(state);
    }
}

/* ============================================
 * Text Buffer Functions
 * ============================================ */

void delete_char(ChunjiinState *state) {
    if (state->cursor_pos <= 0) return;

    /* Shift all characters after cursor one position left */
    for (int i = state->cursor_pos - 1; i < MAX_TEXT_LEN - 1; i++) {
        state->text_buffer[i] = state->text_buffer[i + 1];
    }
    state->cursor_pos--;
    CLAMP_CURSOR(state);
}

/* ============================================
 * Hangul Composition Functions
 * ============================================ */

int get_unicode(HangulState *hangul, const wchar_t *real_jong) {
    /* Return 0 if no initial consonant and no vowel (or only dots) */
    if (wcslen(hangul->chosung) == 0) {
        if (wcslen(hangul->jungsung) == 0 || is_dot(hangul->jungsung)) {
            return 0;
        }
    }

    /* Find chosung index */
    int cho = find_jamo_index(CHO_MAP, hangul->chosung);
    if (cho < 0) cho = 18; /* Default to ㅎ */

    /* Return compatibility Jamo for standalone consonant */
    if (wcslen(hangul->jungsung) == 0 && wcslen(hangul->jongsung) == 0) {
        return COMPAT_CHO[cho];
    }
    if (is_dot(hangul->jungsung)) {
        return COMPAT_CHO[cho];
    }

    /* Find jungsung index */
    int jung = find_jamo_index(JUNG_MAP, hangul->jungsung);
    if (jung < 0) jung = 20; /* Default to ㅣ */

    /* Return compatibility Jamo for standalone vowel */
    if (wcslen(hangul->chosung) == 0 && wcslen(hangul->jongsung) == 0) {
        return COMPAT_JUNG[jung];
    }

    /* Find jongsung index */
    int jong = find_jamo_index(JONG_MAP, real_jong);
    if (jong < 0) jong = 0; /* No final consonant */

    /* Return compatibility Jamo for standalone final consonant */
    if (wcslen(hangul->chosung) == 0 && wcslen(hangul->jungsung) == 0) {
        return COMPAT_JONG[jong];
    }

    /* Return composed Hangul syllable */
    return HANGUL_SYLLABLE_BASE + cho * 588 + jung * 28 + jong;
}

void check_double(const wchar_t *jong, const wchar_t *jong2, wchar_t *result) {
    result[0] = 0;

    if (wcscmp(jong, L"ㄱ") == 0 && wcscmp(jong2, L"ㅅ") == 0) {
        wcscpy(result, L"ㄳ");
    } else if (wcscmp(jong, L"ㄴ") == 0) {
        if (wcscmp(jong2, L"ㅈ") == 0) wcscpy(result, L"ㄵ");
        else if (wcscmp(jong2, L"ㅎ") == 0) wcscpy(result, L"ㄶ");
    } else if (wcscmp(jong, L"ㄹ") == 0) {
        if (wcscmp(jong2, L"ㄱ") == 0) wcscpy(result, L"ㄺ");
        else if (wcscmp(jong2, L"ㅁ") == 0) wcscpy(result, L"ㄻ");
        else if (wcscmp(jong2, L"ㅂ") == 0) wcscpy(result, L"ㄼ");
        else if (wcscmp(jong2, L"ㅅ") == 0) wcscpy(result, L"ㄽ");
        else if (wcscmp(jong2, L"ㅌ") == 0) wcscpy(result, L"ㄾ");
        else if (wcscmp(jong2, L"ㅍ") == 0) wcscpy(result, L"ㄿ");
        else if (wcscmp(jong2, L"ㅎ") == 0) wcscpy(result, L"ㅀ");
    } else if (wcscmp(jong, L"ㅂ") == 0 && wcscmp(jong2, L"ㅅ") == 0) {
        wcscpy(result, L"ㅄ");
    }
}

/* ============================================
 * Mode Management
 * ============================================ */

void change_mode(ChunjiinState *state) {
    state->now_mode = (state->now_mode == MODE_SPECIAL) ? MODE_HANGUL : state->now_mode + 1;
    hangul_init(&state->hangul);
    init_engnum(state);

    if (state->now_mode == MODE_UPPER_ENGLISH) {
        state->flag_upper = true;
    } else if (state->now_mode == MODE_ENGLISH) {
        state->flag_upper = false;
    }
}

const wchar_t* get_button_text(InputMode mode, int button_num) {
    static const wchar_t *hangul_texts[] = {
        L"ㅇㅁ", L"ㅣ", L"·", L"ㅡ", L"ㄱㅋ",
        L"ㄴㄹ", L"ㄷㅌ", L"ㅂㅍ", L"ㅅㅎ", L"ㅈㅊ",
        L"Space", L"←"
    };
    static const wchar_t *upper_eng_texts[] = {
        L"@?!", L"ABC", L"DEF", L"GHI", L"JKL",
        L"MNO", L"PQR", L"STU", L"VWX", L"YZ.",
        L"Space", L"←"
    };
    static const wchar_t *lower_eng_texts[] = {
        L"@?!", L"abc", L"def", L"ghi", L"jkl",
        L"mno", L"pqr", L"stu", L"vwx", L"yz.",
        L"Space", L"←"
    };
    static const wchar_t *number_texts[] = {
        L"0", L"1", L"2", L"3", L"4",
        L"5", L"6", L"7", L"8", L"9",
        L"Space", L"←"
    };
    static const wchar_t *special_texts[] = {
        L"~.^", L"!@#", L"$%&", L"*()=", L"+{}",
        L"[]=", L"<>|", L"-_", L":;", L"\"'/",
        L"Space", L"←"
    };

    if (button_num < 0 || button_num > 11) return L"";

    switch (mode) {
        case MODE_HANGUL: return hangul_texts[button_num];
        case MODE_UPPER_ENGLISH: return upper_eng_texts[button_num];
        case MODE_ENGLISH: return lower_eng_texts[button_num];
        case MODE_NUMBER: return number_texts[button_num];
        case MODE_SPECIAL: return special_texts[button_num];
        default: return L"";
    }
}
