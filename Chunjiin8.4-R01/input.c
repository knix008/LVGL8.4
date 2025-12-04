#include "chunjiin.h"
#include <string.h>
#include <wchar.h>

/* ============================================
 * Helper Functions
 * ============================================ */

/* Convert wide char to lowercase */
static wchar_t towlower_simple(wchar_t ch) {
    if (ch >= L'A' && ch <= L'Z') {
        return ch + (L'a' - L'A');
    }
    return ch;
}

/* Check if string is a dot character */
static bool is_dot_char(const wchar_t *str) {
    return wcscmp(str, L"·") == 0 || wcscmp(str, L"‥") == 0;
}

/* Helper function for multi-tap character cycling */
static void cycle_char(ChunjiinState *state, const wchar_t *char_set, size_t len) {
    wchar_t ch[4] = {char_set[0], char_set[1], char_set[2], 0};

    if (wcslen(state->engnum) == 0) {
        /* First press - use first character */
        state->engnum[0] = ch[0];
        state->engnum[1] = 0;
    } else if (state->engnum[0] == ch[0]) {
        /* Second press - use second character */
        state->engnum[0] = ch[1];
        state->flag_engdelete = true;
    } else if (state->engnum[0] == ch[1] && len >= 3) {
        /* Third press (if available) - use third character */
        state->engnum[0] = ch[2];
        state->flag_engdelete = true;
    } else if (state->engnum[0] == ch[1] && len == 2) {
        /* For 2-character buttons, cycle back to first */
        state->engnum[0] = ch[0];
        state->flag_engdelete = true;
    } else if (state->engnum[0] == ch[2] && len >= 3) {
        /* After third, cycle back to first */
        state->engnum[0] = ch[0];
        state->flag_engdelete = true;
    } else {
        /* Different button pressed - reset to first character */
        state->engnum[0] = ch[0];
        state->engnum[1] = 0;
    }
}

/* ============================================
 * Number Input Implementation
 * ============================================ */

void num_make(ChunjiinState *state, int input) {
    if (state->cursor_pos >= MAX_TEXT_LEN) return;

    if (input == 10) { /* Space */
        wcscpy(state->engnum, L" ");
    } else if (input == 11) { /* Delete */
        delete_char(state);
    } else {
        swprintf(state->engnum, JAMO_BUFFER_SIZE, L"%d", input);
    }

    state->flag_initengnum = true;
}

/* ============================================
 * Special Character Input Implementation
 * ============================================ */

void special_make(ChunjiinState *state, int input) {
    if (state->cursor_pos >= MAX_TEXT_LEN) return;

    if (input == 10) { /* Space */
        if (wcslen(state->engnum) == 0) {
            wcscpy(state->engnum, L" ");
        } else {
            state->engnum[0] = 0;
        }
        state->flag_initengnum = true;
    } else if (input == 11) { /* Delete */
        delete_char(state);
        init_engnum(state);
    } else {
        /* Character set mapping */
        const wchar_t *char_sets[] = {
            L"~.^",  /* 0 */
            L"!@#",  /* 1 */
            L"$%&",  /* 2 */
            L"*()",  /* 3 */
            L"+{}",  /* 4 */
            L"[]=",  /* 5 */
            L"<>|",  /* 6 */
            L"-_",   /* 7 */
            L":;",   /* 8 */
            L"\"'/"  /* 9 */
        };

        if (input >= 0 && input < 10) {
            cycle_char(state, char_sets[input], wcslen(char_sets[input]));
        }
    }
}

/* ============================================
 * English Input Implementation
 * ============================================ */

void eng_make(ChunjiinState *state, int input) {
    if (state->cursor_pos >= MAX_TEXT_LEN) return;

    if (input == 10) { /* Space */
        if (wcslen(state->engnum) == 0) {
            wcscpy(state->engnum, L" ");
        } else {
            state->engnum[0] = 0;
        }
        state->flag_initengnum = true;
    } else if (input == 11) { /* Delete */
        delete_char(state);
        init_engnum(state);
    } else {
        /* T9/Phone keypad character mapping */
        const wchar_t *char_sets[] = {
            L"@?!",  /* 0 */
            L"ABC",  /* 1 */
            L"DEF",  /* 2 */
            L"GHI",  /* 3 */
            L"JKL",  /* 4 */
            L"MNO",  /* 5 */
            L"PQR",  /* 6 */
            L"STU",  /* 7 */
            L"VWX",  /* 8 */
            L"YZ."   /* 9 */
        };

        if (input >= 0 && input < 10) {
            cycle_char(state, char_sets[input], wcslen(char_sets[input]));
        }
    }
}

/* ============================================
 * Hangul Input Implementation - Helper Functions
 * ============================================ */

/* Handle consonant input for Hangul */
static void hangul_process_consonant(ChunjiinState *state, int input) {
    HangulState *hangul = &state->hangul;
    wchar_t beforedata[JAMO_BUFFER_SIZE] = {0};
    wchar_t nowdata[JAMO_BUFFER_SIZE] = {0};
    wchar_t overdata[JAMO_BUFFER_SIZE] = {0};

    /* Transition from vowel to final consonant */
    if (hangul->step == 1) {
        if (is_dot_char(hangul->jungsung)) {
            hangul_init(hangul);
        } else {
            hangul->step = 2;
        }
    }

    /* Get current consonant based on step */
    if (hangul->step == 0) {
        wcscpy(beforedata, hangul->chosung);
    } else if (hangul->step == 2) {
        wcscpy(beforedata, hangul->jongsung);
    } else if (hangul->step == 3) {
        wcscpy(beforedata, hangul->jongsung2);
    }

    /* Process consonant based on input button */
    switch (input) {
        case 4: /* ㄱ, ㅋ, ㄲ, ㄺ */
            if (wcslen(beforedata) == 0) {
                if (hangul->step == 2 && wcslen(hangul->chosung) == 0) {
                    wcscpy(overdata, L"ㄱ");
                } else {
                    wcscpy(nowdata, L"ㄱ");
                }
            } else if (wcscmp(beforedata, L"ㄱ") == 0) {
                wcscpy(nowdata, L"ㅋ");
            } else if (wcscmp(beforedata, L"ㅋ") == 0) {
                wcscpy(nowdata, L"ㄲ");
            } else if (wcscmp(beforedata, L"ㄲ") == 0) {
                wcscpy(nowdata, L"ㄱ");
            } else if (wcscmp(beforedata, L"ㄹ") == 0 && hangul->step == 2) {
                hangul->step = 3;
                wcscpy(nowdata, L"ㄱ");
            } else {
                wcscpy(overdata, L"ㄱ");
            }
            break;

        case 5: /* ㄴ, ㄹ */
            if (wcslen(beforedata) == 0) {
                if (hangul->step == 2 && wcslen(hangul->chosung) == 0) {
                    wcscpy(overdata, L"ㄴ");
                } else {
                    wcscpy(nowdata, L"ㄴ");
                }
            } else if (wcscmp(beforedata, L"ㄴ") == 0) {
                wcscpy(nowdata, L"ㄹ");
            } else if (wcscmp(beforedata, L"ㄹ") == 0) {
                wcscpy(nowdata, L"ㄴ");
            } else {
                wcscpy(overdata, L"ㄴ");
            }
            break;

        case 6: /* ㄷ, ㅌ, ㄸ, ㄾ */
            if (wcslen(beforedata) == 0) {
                if (hangul->step == 2 && wcslen(hangul->chosung) == 0) {
                    wcscpy(overdata, L"ㄷ");
                } else {
                    wcscpy(nowdata, L"ㄷ");
                }
            } else if (wcscmp(beforedata, L"ㄷ") == 0) {
                wcscpy(nowdata, L"ㅌ");
            } else if (wcscmp(beforedata, L"ㅌ") == 0) {
                wcscpy(nowdata, L"ㄸ");
            } else if (wcscmp(beforedata, L"ㄸ") == 0) {
                wcscpy(nowdata, L"ㄷ");
            } else if (wcscmp(beforedata, L"ㄹ") == 0 && hangul->step == 2) {
                hangul->step = 3;
                wcscpy(nowdata, L"ㄷ");
            } else {
                wcscpy(overdata, L"ㄷ");
            }
            break;

        case 7: /* ㅂ, ㅍ, ㅃ, ㄼ, ㄿ */
            if (wcslen(beforedata) == 0) {
                if (hangul->step == 2 && wcslen(hangul->chosung) == 0) {
                    wcscpy(overdata, L"ㅂ");
                } else {
                    wcscpy(nowdata, L"ㅂ");
                }
            } else if (wcscmp(beforedata, L"ㅂ") == 0) {
                wcscpy(nowdata, L"ㅍ");
            } else if (wcscmp(beforedata, L"ㅍ") == 0) {
                wcscpy(nowdata, L"ㅃ");
            } else if (wcscmp(beforedata, L"ㅃ") == 0) {
                wcscpy(nowdata, L"ㅂ");
            } else if (wcscmp(beforedata, L"ㄹ") == 0 && hangul->step == 2) {
                hangul->step = 3;
                wcscpy(nowdata, L"ㅂ");
            } else {
                wcscpy(overdata, L"ㅂ");
            }
            break;

        case 8: /* ㅅ, ㅎ, ㅆ, ㄳ, ㄶ, ㄽ, ㅀ, ㅄ */
            if (wcslen(beforedata) == 0) {
                if (hangul->step == 2 && wcslen(hangul->chosung) == 0) {
                    wcscpy(overdata, L"ㅅ");
                } else {
                    wcscpy(nowdata, L"ㅅ");
                }
            } else if (wcscmp(beforedata, L"ㅅ") == 0) {
                wcscpy(nowdata, L"ㅎ");
            } else if (wcscmp(beforedata, L"ㅎ") == 0) {
                wcscpy(nowdata, L"ㅆ");
            } else if (wcscmp(beforedata, L"ㅆ") == 0) {
                wcscpy(nowdata, L"ㅅ");
            } else if ((wcscmp(beforedata, L"ㄱ") == 0 || wcscmp(beforedata, L"ㄴ") == 0 ||
                        wcscmp(beforedata, L"ㄹ") == 0 || wcscmp(beforedata, L"ㅂ") == 0) &&
                       hangul->step == 2) {
                hangul->step = 3;
                wcscpy(nowdata, L"ㅅ");
            } else {
                wcscpy(overdata, L"ㅅ");
            }
            break;

        case 9: /* ㅈ, ㅊ, ㅉ, ㄵ */
            if (wcslen(beforedata) == 0) {
                if (hangul->step == 2 && wcslen(hangul->chosung) == 0) {
                    wcscpy(overdata, L"ㅈ");
                } else {
                    wcscpy(nowdata, L"ㅈ");
                }
            } else if (wcscmp(beforedata, L"ㅈ") == 0) {
                wcscpy(nowdata, L"ㅊ");
            } else if (wcscmp(beforedata, L"ㅊ") == 0) {
                wcscpy(nowdata, L"ㅉ");
            } else if (wcscmp(beforedata, L"ㅉ") == 0) {
                wcscpy(nowdata, L"ㅈ");
            } else if (wcscmp(beforedata, L"ㄴ") == 0 && hangul->step == 2) {
                hangul->step = 3;
                wcscpy(nowdata, L"ㅈ");
            } else {
                wcscpy(overdata, L"ㅈ");
            }
            break;

        case 0: /* ㅇ, ㅁ, ㄻ */
            if (wcslen(beforedata) == 0) {
                if (hangul->step == 2 && wcslen(hangul->chosung) == 0) {
                    wcscpy(overdata, L"ㅇ");
                } else {
                    wcscpy(nowdata, L"ㅇ");
                }
            } else if (wcscmp(beforedata, L"ㅇ") == 0) {
                wcscpy(nowdata, L"ㅁ");
            } else if (wcscmp(beforedata, L"ㅁ") == 0) {
                wcscpy(nowdata, L"ㅇ");
            } else if (wcscmp(beforedata, L"ㄹ") == 0 && hangul->step == 2) {
                hangul->step = 3;
                wcscpy(nowdata, L"ㅇ");
            } else {
                wcscpy(overdata, L"ㅇ");
            }
            break;
    }

    /* Update state based on processing results */
    if (wcslen(nowdata) > 0) {
        if (hangul->step == 0) {
            wcscpy(hangul->chosung, nowdata);
        } else if (hangul->step == 2) {
            wcscpy(hangul->jongsung, nowdata);
        } else { /* step == 3 */
            wcscpy(hangul->jongsung2, nowdata);
        }
    }
    if (wcslen(overdata) > 0) {
        hangul->flag_writing = false;
        hangul_init(hangul);
        wcscpy(hangul->chosung, overdata);
    }
}

/* Handle vowel input for Hangul */
static void hangul_process_vowel(ChunjiinState *state, int input) {
    HangulState *hangul = &state->hangul;
    wchar_t beforedata[JAMO_BUFFER_SIZE] = {0};
    wchar_t nowdata[JAMO_BUFFER_SIZE] = {0};
    bool batchim = false;

    /* Handle final consonant becoming initial consonant of next syllable */
    if (hangul->step == 2) {
        delete_char(state);
        wchar_t s[JAMO_BUFFER_SIZE];
        wcscpy(s, hangul->jongsung);

        if (!hangul->flag_doubled) {
            hangul->jongsung[0] = 0;
            hangul->flag_writing = false;
            write_hangul(state);
        }
        hangul_init(hangul);
        wcscpy(hangul->chosung, s);
        hangul->step = 0;
        batchim = true;
    } else if (hangul->step == 3) {
        wchar_t s[JAMO_BUFFER_SIZE];
        wcscpy(s, hangul->jongsung2);

        if (hangul->flag_doubled) {
            delete_char(state);
        } else {
            delete_char(state);
            hangul->jongsung2[0] = 0;
            hangul->flag_writing = false;
            write_hangul(state);
        }
        hangul_init(hangul);
        wcscpy(hangul->chosung, s);
        hangul->step = 0;
        batchim = true;
    }

    wcscpy(beforedata, hangul->jungsung);
    hangul->step = 1;

    /* Process vowel based on input button */
    if (input == 1) { /* ㅣ ㅓ ㅕ ㅐ ㅔ ㅖㅒ ㅚ ㅟ ㅙ ㅝ ㅞ ㅢ */
        if (wcslen(beforedata) == 0) {
            wcscpy(nowdata, L"ㅣ");
        } else if (wcscmp(beforedata, L"·") == 0) {
            wcscpy(nowdata, L"ㅓ");
            hangul->flag_dotused = true;
        } else if (wcscmp(beforedata, L"‥") == 0) {
            wcscpy(nowdata, L"ㅕ");
            hangul->flag_dotused = true;
        } else if (wcscmp(beforedata, L"ㅏ") == 0) {
            wcscpy(nowdata, L"ㅐ");
        } else if (wcscmp(beforedata, L"ㅑ") == 0) {
            wcscpy(nowdata, L"ㅒ");
        } else if (wcscmp(beforedata, L"ㅓ") == 0) {
            wcscpy(nowdata, L"ㅔ");
        } else if (wcscmp(beforedata, L"ㅕ") == 0) {
            wcscpy(nowdata, L"ㅖ");
        } else if (wcscmp(beforedata, L"ㅗ") == 0) {
            wcscpy(nowdata, L"ㅚ");
        } else if (wcscmp(beforedata, L"ㅜ") == 0) {
            wcscpy(nowdata, L"ㅟ");
        } else if (wcscmp(beforedata, L"ㅠ") == 0) {
            wcscpy(nowdata, L"ㅝ");
        } else if (wcscmp(beforedata, L"ㅘ") == 0) {
            wcscpy(nowdata, L"ㅙ");
        } else if (wcscmp(beforedata, L"ㅝ") == 0) {
            wcscpy(nowdata, L"ㅞ");
        } else if (wcscmp(beforedata, L"ㅡ") == 0) {
            wcscpy(nowdata, L"ㅢ");
        } else {
            hangul_init(hangul);
            hangul->step = 1;
            wcscpy(nowdata, L"ㅣ");
        }
    } else if (input == 2) { /* ·,‥,ㅏ,ㅑ,ㅜ,ㅠ,ㅘ */
        if (wcslen(beforedata) == 0) {
            wcscpy(nowdata, L"·");
            if (batchim) {
                hangul->flag_addcursor = true;
            }
        } else if (wcscmp(beforedata, L"·") == 0) {
            wcscpy(nowdata, L"‥");
            hangul->flag_dotused = true;
        } else if (wcscmp(beforedata, L"‥") == 0) {
            wcscpy(nowdata, L"·");
            hangul->flag_dotused = true;
        } else if (wcscmp(beforedata, L"ㅣ") == 0) {
            wcscpy(nowdata, L"ㅏ");
        } else if (wcscmp(beforedata, L"ㅏ") == 0) {
            wcscpy(nowdata, L"ㅑ");
        } else if (wcscmp(beforedata, L"ㅡ") == 0) {
            wcscpy(nowdata, L"ㅜ");
        } else if (wcscmp(beforedata, L"ㅜ") == 0) {
            wcscpy(nowdata, L"ㅠ");
        } else if (wcscmp(beforedata, L"ㅚ") == 0) {
            wcscpy(nowdata, L"ㅘ");
        } else {
            hangul_init(hangul);
            hangul->step = 1;
            wcscpy(nowdata, L"·");
        }
    } else if (input == 3) { /* ㅡ, ㅗ, ㅛ */
        if (wcslen(beforedata) == 0) {
            wcscpy(nowdata, L"ㅡ");
        } else if (wcscmp(beforedata, L"·") == 0) {
            wcscpy(nowdata, L"ㅗ");
            hangul->flag_dotused = true;
        } else if (wcscmp(beforedata, L"‥") == 0) {
            wcscpy(nowdata, L"ㅛ");
            hangul->flag_dotused = true;
        } else {
            hangul_init(hangul);
            hangul->step = 1;
            wcscpy(nowdata, L"ㅡ");
        }
    }

    wcscpy(hangul->jungsung, nowdata);
}

/* ============================================
 * Main Hangul Input Function
 * ============================================ */

void hangul_make(ChunjiinState *state, int input) {
    if (state->cursor_pos >= MAX_TEXT_LEN) return;

    HangulState *hangul = &state->hangul;

    if (input == 10) { /* Space */
        if (hangul->flag_writing) {
            hangul_init(hangul);
        } else {
            hangul->flag_space = true;
        }
    } else if (input == 11) { /* Delete */
        if (hangul->step == 0) {
            if (wcslen(hangul->chosung) == 0) {
                delete_char(state);
                hangul->flag_writing = false;
            } else {
                hangul->chosung[0] = 0;
            }
        } else if (hangul->step == 1) {
            if (is_dot_char(hangul->jungsung)) {
                delete_char(state);
                if (wcslen(hangul->chosung) == 0) {
                    hangul->flag_writing = false;
                }
            }
            hangul->jungsung[0] = 0;
            hangul->step = 0;
        } else if (hangul->step == 2) {
            hangul->jongsung[0] = 0;
            hangul->step = 1;
        } else if (hangul->step == 3) {
            hangul->jongsung2[0] = 0;
            hangul->step = 2;
        }
    } else if (input >= 1 && input <= 3) { /* Vowels */
        hangul_process_vowel(state, input);
    } else { /* Consonants (0, 4-9) */
        hangul_process_consonant(state, input);
    }
}

/* ============================================
 * Text Writing Functions
 * ============================================ */

void write_hangul(ChunjiinState *state) {
    HangulState *hangul = &state->hangul;
    int position = state->cursor_pos;
    wchar_t str[MAX_TEXT_LEN] = {0};
    wchar_t real_jongsung[JAMO_BUFFER_SIZE] = {0};

    bool dotflag = false;
    bool doubleflag = false;
    bool spaceflag = false;
    bool impossiblejongsungflag = false;
    wchar_t unicode;

    /* Check for double final consonant */
    check_double(hangul->jongsung, hangul->jongsung2, real_jongsung);
    if (wcslen(real_jongsung) == 0) {
        wcscpy(real_jongsung, hangul->jongsung);
        if (wcslen(hangul->jongsung2) != 0) {
            doubleflag = true;
        }
    }

    /* Check for impossible final consonants (ㅃ, ㅉ, ㄸ) */
    if (wcscmp(hangul->jongsung, L"ㅃ") == 0 ||
        wcscmp(hangul->jongsung, L"ㅉ") == 0 ||
        wcscmp(hangul->jongsung, L"ㄸ") == 0) {
        doubleflag = true;
        impossiblejongsungflag = true;
        unicode = (wchar_t)get_unicode(hangul, L"");
    } else {
        unicode = (wchar_t)get_unicode(hangul, real_jongsung);
    }

    /* Build the string before cursor */
    if (!hangul->flag_writing) {
        wcsncpy(str, state->text_buffer, position);
        str[position] = 0;
    } else if (hangul->flag_dotused) {
        int offset = wcslen(hangul->chosung) == 0 ? 1 : 2;
        wcsncpy(str, state->text_buffer, position - offset);
        str[position - offset] = 0;
    } else if (hangul->flag_doubled) {
        wcsncpy(str, state->text_buffer, position - 2);
        str[position - 2] = 0;
    } else {
        wcsncpy(str, state->text_buffer, position - 1);
        str[position - 1] = 0;
    }

    /* Add the unicode character */
    if (unicode != 0) {
        size_t len = wcslen(str);
        str[len] = unicode;
        str[len + 1] = 0;
    }

    /* Add space if needed */
    if (hangul->flag_space) {
        wcscat(str, L" ");
        hangul->flag_space = false;
        spaceflag = true;
    }

    /* Add double final consonant if needed */
    if (doubleflag) {
        if (impossiblejongsungflag) {
            wcscat(str, hangul->jongsung);
        } else {
            wcscat(str, hangul->jongsung2);
        }
    }

    /* Add dot if jungsung is dot */
    if (wcscmp(hangul->jungsung, L"·") == 0) {
        wcscat(str, L"·");
        dotflag = true;
    } else if (wcscmp(hangul->jungsung, L"‥") == 0) {
        wcscat(str, L"‥");
        dotflag = true;
    }

    /* Add the rest of the text after cursor */
    wcscat(str, &state->text_buffer[position]);

    /* Copy back to text buffer */
    wcscpy(state->text_buffer, str);

    /* Adjust cursor position */
    if (dotflag) {
        position++;
    }
    if (doubleflag) {
        if (!hangul->flag_doubled) {
            position++;
        }
        hangul->flag_doubled = true;
    } else {
        if (hangul->flag_doubled) {
            position--;
        }
        hangul->flag_doubled = false;
    }
    if (spaceflag) {
        position++;
    }
    if (unicode == 0 && dotflag == false) {
        position--;
    }
    if (hangul->flag_addcursor) {
        hangul->flag_addcursor = false;
        position++;
    }

    /* Set final cursor position */
    if (hangul->flag_dotused) {
        if (wcslen(hangul->chosung) == 0 && dotflag == false) {
            state->cursor_pos = position;
        } else {
            state->cursor_pos = position - 1;
        }
    } else if (!hangul->flag_writing && dotflag == false) {
        state->cursor_pos = position + 1;
    } else {
        state->cursor_pos = position;
    }

    /* Boundary check */
    CLAMP_CURSOR(state);

    hangul->flag_dotused = false;
    hangul->flag_writing = (unicode == 0 && dotflag == false) ? false : true;
}

void write_engnum(ChunjiinState *state) {
    int position = state->cursor_pos;
    wchar_t str[MAX_TEXT_LEN] = {0};

    /* Build string before cursor */
    if (state->flag_engdelete) {
        wcsncpy(str, state->text_buffer, position - 1);
        str[position - 1] = 0;
    } else {
        wcsncpy(str, state->text_buffer, position);
        str[position] = 0;
    }

    /* Add engnum (uppercase or lowercase) */
    if (state->flag_upper || state->now_mode == MODE_NUMBER) {
        wcscat(str, state->engnum);
    } else {
        /* Convert to lowercase */
        wchar_t lower_engnum[JAMO_BUFFER_SIZE];
        for (size_t i = 0; i < wcslen(state->engnum); i++) {
            lower_engnum[i] = towlower_simple(state->engnum[i]);
        }
        lower_engnum[wcslen(state->engnum)] = 0;
        wcscat(str, lower_engnum);
    }

    /* Handle delete case */
    if (state->flag_engdelete) {
        wcscat(str, &state->text_buffer[position]);
        wcscpy(state->text_buffer, str);
        state->cursor_pos = position;
        state->flag_engdelete = false;
    } else {
        wcscat(str, &state->text_buffer[position]);
        wcscpy(state->text_buffer, str);
        if (wcslen(state->engnum) == 0) {
            state->cursor_pos = position;
        } else {
            state->cursor_pos = position + 1;
        }
    }

    /* Boundary check */
    CLAMP_CURSOR(state);

    /* Initialize engnum if flag is set */
    if (state->flag_initengnum) {
        init_engnum(state);
    }
}
