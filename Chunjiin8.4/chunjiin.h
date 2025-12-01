#define CLAMP_CURSOR(state) do { \
    if ((state)->cursor_pos > MAX_TEXT_LEN) (state)->cursor_pos = MAX_TEXT_LEN; \
    if ((state)->cursor_pos < 0) (state)->cursor_pos = 0; \
} while(0)
#ifndef CHUNJIIN_H
#define CHUNJIIN_H

#include <stddef.h>
#include <stdbool.h>
#include <wchar.h>

#define MAX_TEXT_LEN 1024

typedef enum {
    MODE_HANGUL = 0,
    MODE_UPPER_ENGLISH = 1,
    MODE_ENGLISH = 2,
    MODE_NUMBER = 3,
    MODE_SPECIAL = 4
} InputMode;

typedef struct {
    wchar_t chosung[16];      // 초성
    wchar_t jungsung[16];     // 중성
    wchar_t jongsung[16];     // 종성
    wchar_t jongsung2[16];    // 종성2 (겹받침)
    int step;                 // 현재 단계 (0:초성, 1:중성, 2:종성, 3:겹받침)
    bool flag_writing;        // 작성 중 플래그
    bool flag_dotused;        // 점(·, ‥) 사용 플래그
    bool flag_doubled;        // 겹받침 플래그
    bool flag_addcursor;      // 커서 추가 플래그
    bool flag_space;          // 스페이스 플래그
} HangulState;

typedef struct {
    HangulState hangul;
    InputMode now_mode;

    wchar_t engnum[16];       // 영문/숫자 버퍼
    bool flag_initengnum;     // 영문/숫자 초기화 플래그
    bool flag_engdelete;      // 영문 삭제 플래그
    bool flag_upper;          // 대문자 플래그

    wchar_t text_buffer[MAX_TEXT_LEN];  // 텍스트 버퍼
    int cursor_pos;           // 커서 위치
} ChunjiinState;

// wchar_t to UTF-8 conversion
char* wchar_to_utf8(const wchar_t *wstr, size_t max_len);

// 초기화 함수
void chunjiin_init(ChunjiinState *state);
void hangul_init(HangulState *hangul);
void init_engnum(ChunjiinState *state);

// 입력 처리 함수
void chunjiin_process_input(ChunjiinState *state, int input);
void hangul_make(ChunjiinState *state, int input);
void eng_make(ChunjiinState *state, int input);
void num_make(ChunjiinState *state, int input);
void special_make(ChunjiinState *state, int input);

// 텍스트 처리 함수
void write_hangul(ChunjiinState *state);
void write_engnum(ChunjiinState *state);
void delete_char(ChunjiinState *state);

// 유니코드 처리 함수
int get_unicode(HangulState *hangul, const wchar_t *real_jong);
void check_double(const wchar_t *jong, const wchar_t *jong2, wchar_t *result);

// 모드 변경
void change_mode(ChunjiinState *state);

// 버튼 텍스트 가져오기
const wchar_t* get_button_text(InputMode mode, int button_num);

#endif // CHUNJIIN_H
