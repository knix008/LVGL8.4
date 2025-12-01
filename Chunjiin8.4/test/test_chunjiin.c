#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../chunjiin.h"

void test_init() {
    ChunjiinState state;
    chunjiin_init(&state);
    assert(state.now_mode == MODE_HANGUL);
    assert(state.cursor_pos == 0);
    for (int i = 0; i < MAX_TEXT_LEN; i++) {
        assert(state.text_buffer[i] == 0);
    }
    printf("test_init passed\n");
}

void test_hangul_input() {
    ChunjiinState state;
    chunjiin_init(&state);
    chunjiin_process_input(&state, 4); // ㄱ
    chunjiin_process_input(&state, 1); // ㅣ
    chunjiin_process_input(&state, 5); // ㄴ
    // Should have composed a syllable (e.g., 긴)
    assert(state.cursor_pos > 0);
    printf("test_hangul_input passed\n");
}

void test_mode_switch() {
    ChunjiinState state;
    chunjiin_init(&state);
    change_mode(&state);
    assert(state.now_mode == MODE_UPPER_ENGLISH);
    change_mode(&state);
    assert(state.now_mode == MODE_ENGLISH);
    change_mode(&state);
    assert(state.now_mode == MODE_NUMBER);
    change_mode(&state);
    assert(state.now_mode == MODE_SPECIAL);
    change_mode(&state);
    assert(state.now_mode == MODE_HANGUL);
    printf("test_mode_switch passed\n");
}

void test_buffer_limit() {
    ChunjiinState state;
    chunjiin_init(&state);
    for (int i = 0; i < MAX_TEXT_LEN + 10; i++) {
        chunjiin_process_input(&state, 4); // ㄱ
    }
    assert(state.cursor_pos <= MAX_TEXT_LEN);
    printf("test_buffer_limit passed\n");
}

void test_clear_and_enter() {
    ChunjiinState state;
    chunjiin_init(&state);
    chunjiin_process_input(&state, 4); // ㄱ
    chunjiin_init(&state); // simulate clear
    assert(state.cursor_pos == 0);
    for (int i = 0; i < MAX_TEXT_LEN; i++) {
        assert(state.text_buffer[i] == 0);
    }
    printf("test_clear_and_enter passed\n");
}

int main() {
    test_init();
    test_hangul_input();
    test_mode_switch();
    test_buffer_limit();
    test_clear_and_enter();
    printf("All Chunjiin tests passed!\n");
    return 0;
}
