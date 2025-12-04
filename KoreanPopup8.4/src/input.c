#include "../include/chunjiin.h"
#include "../include/config.h"
#include <string.h>
#include <wchar.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Helper function to convert wide char to lowercase
static wchar_t towlower_simple(wchar_t ch) {
    if (ch >= L'A' && ch <= L'Z') {
        return ch + (L'a' - L'A');
    }
    return ch;
}

// ============================================================================
// NUMBER INPUT PROCESSING
// ============================================================================

// num_make function (lines 381-391 in Java)
void num_make(ChunjiinState *state, int input) {
    if (state->cursor_pos >= MAX_TEXT_LEN) return;

    if (input == CHUNJIIN_SPACE_KEY) { // Space
        wcscpy(state->engnum, L" ");
    } else if (input == CHUNJIIN_DELETE_KEY) { // Delete
        delete_char(state);
    } else {
        swprintf(state->engnum, 16, L"%d", input);
    }

    state->flag_initengnum = true;
}

// ============================================================================
// SPECIAL CHARACTER INPUT PROCESSING
// ============================================================================

// special_make function for special characters
void special_make(ChunjiinState *state, int input) {
    if (state->cursor_pos >= MAX_TEXT_LEN) return;

    if (input == CHUNJIIN_SPACE_KEY) { // Space
        if (wcslen(state->engnum) == 0) {
            wcscpy(state->engnum, L" ");
        } else {
            state->engnum[0] = 0;
        }
        state->flag_initengnum = true;
    } else if (input == CHUNJIIN_DELETE_KEY) { // Delete
        delete_char(state);
        init_engnum(state);
    } else {
        const wchar_t *str = L"";
        switch (input) {
            case 0: str = L"~.^"; break;
            case 1: str = L"!@#"; break;
            case 2: str = L"$%&"; break;
            case 3: str = L"*()"; break;
            case 4: str = L"+{}"; break;
            case 5: str = L"[]="; break;
            case 6: str = L"<>|"; break;
            case 7: str = L"-_"; break;
            case 8: str = L":;"; break;
            case 9: str = L"\"'/"; break;
            default: return;
        }

        wchar_t ch[4];
        ch[0] = str[0];
        ch[1] = str[1];
        ch[2] = str[2];
        ch[3] = 0;

        size_t str_len = wcslen(str);

        if (wcslen(state->engnum) == 0) {
            state->engnum[0] = ch[0];
            state->engnum[1] = 0;
        } else if (state->engnum[0] == ch[0]) {
            state->engnum[0] = ch[1];
            state->engnum[1] = 0;
            state->flag_engdelete = true;
        } else if (state->engnum[0] == ch[1] && str_len >= 3) {
            state->engnum[0] = ch[2];
            state->engnum[1] = 0;
            state->flag_engdelete = true;
        } else if (state->engnum[0] == ch[1] && str_len == 2) {
            // For 2-character buttons, cycle back to first character
            state->engnum[0] = ch[0];
            state->engnum[1] = 0;
            state->flag_engdelete = true;
        } else if (state->engnum[0] == ch[2] && str_len >= 3) {
            state->engnum[0] = ch[0];
            state->engnum[1] = 0;
            state->flag_engdelete = true;
        } else {
            state->engnum[0] = ch[0];
            state->engnum[1] = 0;
        }
    }
}

// ============================================================================
// ENGLISH INPUT PROCESSING
// ============================================================================

// eng_make function (lines 325-380 in Java)
void eng_make(ChunjiinState *state, int input) {
    if (state->cursor_pos >= MAX_TEXT_LEN) return;

    if (input == CHUNJIIN_SPACE_KEY) { // Space
        if (wcslen(state->engnum) == 0) {
            wcscpy(state->engnum, L" ");
        } else {
            state->engnum[0] = 0;
        }
        state->flag_initengnum = true;
    } else if (input == CHUNJIIN_DELETE_KEY) { // Delete
        delete_char(state);
        init_engnum(state);
    } else {
        const wchar_t *str = L"";
        switch (input) {
            case 0: str = L"@?!"; break;
            case 1: str = L"ABC"; break;
            case 2: str = L"DEF"; break;
            case 3: str = L"GHI"; break;
            case 4: str = L"JKL"; break;
            case 5: str = L"MNO"; break;
            case 6: str = L"PQR"; break;
            case 7: str = L"STU"; break;
            case 8: str = L"VWX"; break;
            case 9: str = L"YZ."; break;
            default: return;
        }

        wchar_t ch[4];
        ch[0] = str[0];
        ch[1] = str[1];
        ch[2] = str[2];
        ch[3] = 0;

        size_t str_len = wcslen(str);

        if (wcslen(state->engnum) == 0) {
            state->engnum[0] = ch[0];
            state->engnum[1] = 0;
        } else if (state->engnum[0] == ch[0]) {
            state->engnum[0] = ch[1];
            state->engnum[1] = 0;
            state->flag_engdelete = true;
        } else if (state->engnum[0] == ch[1] && str_len >= 3) {
            state->engnum[0] = ch[2];
            state->engnum[1] = 0;
            state->flag_engdelete = true;
        } else if (state->engnum[0] == ch[1] && str_len == 2) {
            // For 2-character buttons, cycle back to first character
            state->engnum[0] = ch[0];
            state->engnum[1] = 0;
            state->flag_engdelete = true;
        } else if (state->engnum[0] == ch[2] && str_len >= 3) {
            state->engnum[0] = ch[0];
            state->engnum[1] = 0;
            state->flag_engdelete = true;
        } else {
            state->engnum[0] = ch[0];
            state->engnum[1] = 0;
        }
    }
}

// ============================================================================
// HANGUL INPUT PROCESSING
// ============================================================================

// hangul_make function (lines 392-788 in Java)
void hangul_make(ChunjiinState *state, int input) {
    if (state->cursor_pos >= MAX_TEXT_LEN) return;

    HangulState *hangul = &state->hangul;
    wchar_t beforedata[16] = {0};
    wchar_t nowdata[16] = {0};
    wchar_t overdata[16] = {0};

    if (input == CHUNJIIN_SPACE_KEY) { // Space
        if (hangul->flag_writing) {
            hangul_init(hangul);
        } else {
            hangul->flag_space = true;
        }
    } else if (input == CHUNJIIN_DELETE_KEY) { // Delete
        if (hangul->step == 0) {
            if (wcslen(hangul->chosung) == 0) {
                delete_char(state);
                hangul->flag_writing = false;
            } else {
                hangul->chosung[0] = 0;
            }
        } else if (hangul->step == 1) {
            if (wcscmp(hangul->jungsung, L"·") == 0 || wcscmp(hangul->jungsung, L"‥") == 0) {
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
    } else if (input == 1 || input == 2 || input == 3) { // Vowels
        bool batchim = false;
        if (hangul->step == 2) {
            delete_char(state);
            wchar_t s[16];
            wcscpy(s, hangul->jongsung);
            // Bug fixed, 16.4.22
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
            wchar_t s[16];
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

        if (input == 1) { // ㅣ ㅓ ㅕ ㅐ ㅔ ㅖㅒ ㅚ ㅟ ㅙ ㅝ ㅞ ㅢ
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
        } else if (input == 2) { // ·,‥,ㅏ,ㅑ,ㅜ,ㅠ,ㅘ
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
        } else if (input == 3) { // ㅡ, ㅗ, ㅛ
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
    } else { // Consonants
        if (hangul->step == 1) {
            if (wcscmp(hangul->jungsung, L"·") == 0 || wcscmp(hangul->jungsung, L"‥") == 0) {
                hangul_init(hangul);
            } else {
                hangul->step = 2;
            }
        }

        if (hangul->step == 0) {
            wcscpy(beforedata, hangul->chosung);
        } else if (hangul->step == 2) {
            wcscpy(beforedata, hangul->jongsung);
        } else if (hangul->step == 3) {
            wcscpy(beforedata, hangul->jongsung2);
        }

        if (input == 4) { // ㄱ, ㅋ, ㄲ, ㄺ
            if (wcslen(beforedata) == 0) {
                if (hangul->step == 2) {
                    if (wcslen(hangul->chosung) == 0) {
                        wcscpy(overdata, L"ㄱ");
                    } else {
                        wcscpy(nowdata, L"ㄱ");
                    }
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
        } else if (input == 5) { // ㄴ ㄹ
            if (wcslen(beforedata) == 0) {
                if (hangul->step == 2) {
                    if (wcslen(hangul->chosung) == 0) {
                        wcscpy(overdata, L"ㄴ");
                    } else {
                        wcscpy(nowdata, L"ㄴ");
                    }
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
        } else if (input == 6) { // ㄷ, ㅌ, ㄸ, ㄾ
            if (wcslen(beforedata) == 0) {
                if (hangul->step == 2) {
                    if (wcslen(hangul->chosung) == 0) {
                        wcscpy(overdata, L"ㄷ");
                    } else {
                        wcscpy(nowdata, L"ㄷ");
                    }
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
        } else if (input == 7) { // ㅂ, ㅍ, ㅃ, ㄼ, ㄿ
            if (wcslen(beforedata) == 0) {
                if (hangul->step == 2) {
                    if (wcslen(hangul->chosung) == 0) {
                        wcscpy(overdata, L"ㅂ");
                    } else {
                        wcscpy(nowdata, L"ㅂ");
                    }
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
        } else if (input == 8) { // ㅅ, ㅎ, ㅆ, ㄳ, ㄶ, ㄽ, ㅀ, ㅄ
            if (wcslen(beforedata) == 0) {
                if (hangul->step == 2) {
                    if (wcslen(hangul->chosung) == 0) {
                        wcscpy(overdata, L"ㅅ");
                    } else {
                        wcscpy(nowdata, L"ㅅ");
                    }
                } else {
                    wcscpy(nowdata, L"ㅅ");
                }
            } else if (wcscmp(beforedata, L"ㅅ") == 0) {
                wcscpy(nowdata, L"ㅎ");
            } else if (wcscmp(beforedata, L"ㅎ") == 0) {
                wcscpy(nowdata, L"ㅆ");
            } else if (wcscmp(beforedata, L"ㅆ") == 0) {
                wcscpy(nowdata, L"ㅅ");
            } else if (wcscmp(beforedata, L"ㄱ") == 0 && hangul->step == 2) {
                hangul->step = 3;
                wcscpy(nowdata, L"ㅅ");
            } else if (wcscmp(beforedata, L"ㄴ") == 0 && hangul->step == 2) {
                hangul->step = 3;
                wcscpy(nowdata, L"ㅅ");
            } else if (wcscmp(beforedata, L"ㄹ") == 0 && hangul->step == 2) {
                hangul->step = 3;
                wcscpy(nowdata, L"ㅅ");
            } else if (wcscmp(beforedata, L"ㅂ") == 0 && hangul->step == 2) {
                hangul->step = 3;
                wcscpy(nowdata, L"ㅅ");
            } else {
                wcscpy(overdata, L"ㅅ");
            }
        } else if (input == 9) { // ㅈ, ㅊ, ㅉ, ㄵ
            if (wcslen(beforedata) == 0) {
                if (hangul->step == 2) {
                    if (wcslen(hangul->chosung) == 0) {
                        wcscpy(overdata, L"ㅈ");
                    } else {
                        wcscpy(nowdata, L"ㅈ");
                    }
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
        } else if (input == 0) { // ㅇ, ㅁ, ㄻ
            if (wcslen(beforedata) == 0) {
                if (hangul->step == 2) {
                    if (wcslen(hangul->chosung) == 0) {
                        wcscpy(overdata, L"ㅇ");
                    } else {
                        wcscpy(nowdata, L"ㅇ");
                    }
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
        }

        if (wcslen(nowdata) > 0) {
            if (hangul->step == 0) {
                wcscpy(hangul->chosung, nowdata);
            } else if (hangul->step == 2) {
                wcscpy(hangul->jongsung, nowdata);
            } else { // if (hangul->step == 3)
                wcscpy(hangul->jongsung2, nowdata);
            }
        }
        if (wcslen(overdata) > 0) {
            hangul->flag_writing = false;
            hangul_init(hangul);
            wcscpy(hangul->chosung, overdata);
        }
    }
}

// ============================================================================
// TEXT WRITING FUNCTIONS
// ============================================================================

// write_hangul function (lines 151-254 in Java, the hangul mode branch)
void write_hangul(ChunjiinState *state) {
    HangulState *hangul = &state->hangul;
    int position = state->cursor_pos;
    wchar_t str[MAX_TEXT_LEN] = {0};
    wchar_t real_jongsung[16] = {0};

    bool dotflag = false;
    bool doubleflag = false;
    bool spaceflag = false;
    bool impossiblejongsungflag = false;
    wchar_t unicode;

    // Check for double jongsung
    check_double(hangul->jongsung, hangul->jongsung2, real_jongsung);
    if (wcslen(real_jongsung) == 0) {
        wcscpy(real_jongsung, hangul->jongsung);
        if (wcslen(hangul->jongsung2) != 0) {
            doubleflag = true;
        }
    }

    // Bug fixed, 16.4.22: added impossible jongsungflag
    if (wcscmp(hangul->jongsung, L"ㅃ") == 0 ||
        wcscmp(hangul->jongsung, L"ㅉ") == 0 ||
        wcscmp(hangul->jongsung, L"ㄸ") == 0) {
        doubleflag = true;
        impossiblejongsungflag = true;
        unicode = (wchar_t)get_unicode(hangul, L"");
    } else {
        unicode = (wchar_t)get_unicode(hangul, real_jongsung);
    }

    // Build the string before cursor
    if (!hangul->flag_writing) {
        wcsncpy(str, state->text_buffer, position);
        str[position] = 0;
    } else if (hangul->flag_dotused) {
        if (wcslen(hangul->chosung) == 0) {
            wcsncpy(str, state->text_buffer, position - 1);
            str[position - 1] = 0;
        } else {
            wcsncpy(str, state->text_buffer, position - 2);
            str[position - 2] = 0;
        }
    } else if (hangul->flag_doubled) {
        wcsncpy(str, state->text_buffer, position - 2);
        str[position - 2] = 0;
    } else {
        wcsncpy(str, state->text_buffer, position - 1);
        str[position - 1] = 0;
    }

    // Add the unicode character
    if (unicode != 0) {
        size_t len = wcslen(str);
        str[len] = unicode;
        str[len + 1] = 0;
    }

    // Add space if needed
    if (hangul->flag_space) {
        wcscat(str, L" ");
        hangul->flag_space = false;
        spaceflag = true;
    }

    // Add double jongsung if needed
    if (doubleflag) {
        if (impossiblejongsungflag) {
            wcscat(str, hangul->jongsung);
        } else {
            wcscat(str, hangul->jongsung2);
        }
    }

    // Add dot if jungsung is dot
    if (wcscmp(hangul->jungsung, L"·") == 0) {
        wcscat(str, L"·");
        dotflag = true;
    } else if (wcscmp(hangul->jungsung, L"‥") == 0) {
        wcscat(str, L"‥");
        dotflag = true;
    }

    // Add the rest of the text after cursor
    wcscat(str, &state->text_buffer[position]);

    // Copy back to text buffer
    wcscpy(state->text_buffer, str);

    // Adjust cursor position
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

    // Set final cursor position
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
    // Boundary check
    CLAMP_CURSOR(state);

    hangul->flag_dotused = false;
    hangul->flag_writing = (unicode == 0 && dotflag == false) ? false : true;
}

// write_engnum function (lines 256-309 in Java, the english/number mode branch)
void write_engnum(ChunjiinState *state) {
    int position = state->cursor_pos;
    wchar_t str[MAX_TEXT_LEN] = {0};

    // Build string before cursor
    if (state->flag_engdelete) {
        wcsncpy(str, state->text_buffer, position - 1);
        str[position - 1] = 0;
    } else {
        wcsncpy(str, state->text_buffer, position);
        str[position] = 0;
    }

    // Add engnum (uppercase or lowercase)
    if (state->flag_upper || state->now_mode == MODE_NUMBER) {
        wcscat(str, state->engnum);
    } else {
        // Convert to lowercase
        wchar_t lower_engnum[16];
        size_t i;
        for (i = 0; i < wcslen(state->engnum); i++) {
            lower_engnum[i] = towlower_simple(state->engnum[i]);
        }
        lower_engnum[i] = 0;
        wcscat(str, lower_engnum);
    }

    // Handle delete case
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
    // Boundary check
    CLAMP_CURSOR(state);

    // Initialize engnum if flag is set
    if (state->flag_initengnum) {
        init_engnum(state);
    }
}
