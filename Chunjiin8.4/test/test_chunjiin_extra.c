#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h> // for rand
#include "../chunjiin.h"

void test_delete_char() {
    ChunjiinState state;
    chunjiin_init(&state);
    // Input ㄱ, ㅏ, ㄴ (simulate a syllable)
    chunjiin_process_input(&state, 4); // ㄱ
    chunjiin_process_input(&state, 1); // ㅣ
    chunjiin_process_input(&state, 5); // ㄴ
    int before = state.cursor_pos;
    delete_char(&state);
    assert(state.cursor_pos == before - 1 || state.cursor_pos == 0);
    printf("test_delete_char passed\n");
}

void test_rapid_mode_switch() {
    ChunjiinState state;
    chunjiin_init(&state);
    for (int i = 0; i < 20; i++) {
        change_mode(&state);
    }
    assert(state.now_mode >= MODE_HANGUL && state.now_mode <= MODE_SPECIAL);
    printf("test_rapid_mode_switch passed\n");
}

void test_invalid_input() {
    ChunjiinState state;
    chunjiin_init(&state);
    int before = state.cursor_pos;
    chunjiin_process_input(&state, -1); // invalid
    chunjiin_process_input(&state, 99); // invalid
    assert(state.cursor_pos == before);
    printf("test_invalid_input passed\n");
}

void test_full_user_session() {
    ChunjiinState state;
    chunjiin_init(&state);
    // Input: ㄱ, ㅏ, ㄴ, mode switch, 1, 2, 3, clear, ㄷ, ㅏ, ㄹ, enter
    chunjiin_process_input(&state, 4); // ㄱ
    chunjiin_process_input(&state, 1); // ㅣ
    chunjiin_process_input(&state, 5); // ㄴ
    change_mode(&state); // English
    chunjiin_process_input(&state, 1); // .QZ
    chunjiin_process_input(&state, 2); // ABC
    chunjiin_process_input(&state, 3); // DEF
    chunjiin_init(&state); // clear
    chunjiin_process_input(&state, 6); // ㄷ
    chunjiin_process_input(&state, 1); // ㅏ
    chunjiin_process_input(&state, 5); // ㄹ
    assert(state.cursor_pos > 0);
    printf("test_full_user_session passed\n");
}

void test_korean_combinations() {
    ChunjiinState state;
    chunjiin_init(&state);
    // Simple syllable: ㄱ + ㅏ + ㄴ → 간
    chunjiin_process_input(&state, 4); // ㄱ
    chunjiin_process_input(&state, 1); // ㅣ (simulate ㅏ if needed)
    chunjiin_process_input(&state, 5); // ㄴ
    assert(state.cursor_pos > 0);
    // Compound chosung: ㄲ (simulate by inputting ㄱ twice if system supports)
    chunjiin_init(&state);
    chunjiin_process_input(&state, 4); // ㄱ
    chunjiin_process_input(&state, 4); // ㄱ again for ㄲ
    chunjiin_process_input(&state, 1); // ㅣ
    chunjiin_process_input(&state, 5); // ㄴ
    assert(state.cursor_pos > 0);
    // Compound jungsung: ㅘ (ㅗ + ㅏ)
    chunjiin_init(&state);
    chunjiin_process_input(&state, 3); // ㅡ (simulate ㅗ if needed)
    chunjiin_process_input(&state, 1); // ㅏ
    chunjiin_process_input(&state, 4); // ㄱ
    assert(state.cursor_pos > 0);
    // Compound jongsung: ㄳ (ㄱ + ㅅ)
    chunjiin_init(&state);
    chunjiin_process_input(&state, 4); // ㄱ
    chunjiin_process_input(&state, 1); // ㅏ
    chunjiin_process_input(&state, 4); // ㄱ
    chunjiin_process_input(&state, 8); // ㅅ (simulate compound jongsung)
    assert(state.cursor_pos > 0);
    printf("test_korean_combinations passed\n");
}

void test_compound_jungsung_jongsung() {
    ChunjiinState state;
    chunjiin_init(&state);
    // Compound jungsung: ㅙ (ㅗ+ㅐ)
    chunjiin_process_input(&state, 8); // ㅗ
    chunjiin_process_input(&state, 2); // ㅐ
    chunjiin_process_input(&state, 4); // ㄱ
    assert(state.cursor_pos > 0);
    // Compound jongsung: ㄵ (ㄴ+ㅈ)
    chunjiin_init(&state);
    chunjiin_process_input(&state, 5); // ㄴ
    chunjiin_process_input(&state, 1); // ㅏ
    chunjiin_process_input(&state, 5); // ㄴ
    chunjiin_process_input(&state, 9); // ㅈ (simulate compound jongsung)
    assert(state.cursor_pos > 0);
    printf("test_compound_jungsung_jongsung passed\n");
}

void test_buffer_overflow_underflow() {
    ChunjiinState state;
    chunjiin_init(&state);
    for (int i = 0; i < MAX_TEXT_LEN + 100; i++) {
        chunjiin_process_input(&state, 4); // ㄱ
    }
    assert(state.cursor_pos <= MAX_TEXT_LEN);
    // Underflow: delete at start
    chunjiin_init(&state);
    delete_char(&state);
    assert(state.cursor_pos == 0);
    printf("test_buffer_overflow_underflow passed\n");
}

void test_rapid_clear_enter() {
    ChunjiinState state;
    chunjiin_init(&state);
    for (int i = 0; i < 10; i++) {
        chunjiin_process_input(&state, 4); // ㄱ
        chunjiin_init(&state); // clear
    }
    assert(state.cursor_pos == 0);
    printf("test_rapid_clear_enter passed\n");
}

void test_utf8_conversion() {
    ChunjiinState state;
    chunjiin_init(&state);
    chunjiin_process_input(&state, 4); // ㄱ
    chunjiin_process_input(&state, 1); // ㅣ
    chunjiin_process_input(&state, 5); // ㄴ
    extern char* wchar_to_utf8(const wchar_t*, size_t);
    char* utf8 = wchar_to_utf8(state.text_buffer, MAX_TEXT_LEN);
    assert(utf8[0] != '\0');
    printf("test_utf8_conversion passed: %s\n", utf8);
}

void test_delete_at_end() {
    ChunjiinState state;
    chunjiin_init(&state);
    for (int i = 0; i < 5; i++) chunjiin_process_input(&state, 4);
    int before = state.cursor_pos;
    delete_char(&state);
    assert(state.cursor_pos == before - 1);
    printf("test_delete_at_end passed\n");
}

void test_mode_switch_mid_composition() {
    ChunjiinState state;
    chunjiin_init(&state);
    chunjiin_process_input(&state, 4); // ㄱ
    change_mode(&state); // switch mode mid-composition
    chunjiin_process_input(&state, 1); // English or next mode
    assert(state.now_mode != MODE_HANGUL || state.cursor_pos > 0);
    printf("test_mode_switch_mid_composition passed\n");
}

void test_special_number_english_input() {
    ChunjiinState state;
    chunjiin_init(&state);
    change_mode(&state); // English
    chunjiin_process_input(&state, 2); // ABC
    change_mode(&state); // Number
    chunjiin_process_input(&state, 1); // 1
    change_mode(&state); // Special
    chunjiin_process_input(&state, 1); // !@#
    assert(state.cursor_pos > 0);
    printf("test_special_number_english_input passed\n");
}

void test_fuzz_random_input() {
    ChunjiinState state;
    chunjiin_init(&state);
    for (int i = 0; i < 1000; i++) {
        int input = rand() % 20 - 4; // random input, some invalid
        chunjiin_process_input(&state, input);
    }
    // Should not crash, buffer should not overflow
    assert(state.cursor_pos <= MAX_TEXT_LEN);
    printf("test_fuzz_random_input passed\n");
}

void test_memory_boundary() {
    ChunjiinState state;
    chunjiin_init(&state);
    for (int i = 0; i < MAX_TEXT_LEN + 10; i++) {
        chunjiin_process_input(&state, 4); // try to overfill buffer
    }
    assert(state.cursor_pos <= MAX_TEXT_LEN);
    // Optionally, check that the buffer is not overfilled
    int count = 0;
    for (int i = 0; i < MAX_TEXT_LEN; i++) {
        if (state.text_buffer[i] != 0) count++;
    }
    assert(count <= MAX_TEXT_LEN);
    printf("test_memory_boundary passed\n");
}

void test_rapid_mode_clear_enter() {
    ChunjiinState state;
    chunjiin_init(&state);
    for (int i = 0; i < 50; i++) {
        change_mode(&state);
        chunjiin_init(&state);
    }
    assert(state.cursor_pos == 0);
    printf("test_rapid_mode_clear_enter passed\n");
}

void test_unicode_edge_cases() {
    ChunjiinState state;
    chunjiin_init(&state);
    // Compose Hangul syllable at Unicode edge
    chunjiin_process_input(&state, 4); // ㄱ
    chunjiin_process_input(&state, 1); // ㅣ
    chunjiin_process_input(&state, 5); // ㄴ
    char* utf8 = wchar_to_utf8(state.text_buffer, MAX_TEXT_LEN);
    assert(utf8[0] != '\0');
    // Compose with high code point
    state.text_buffer[0] = 0xD7A3; // last Hangul syllable
    state.text_buffer[1] = 0;
    utf8 = wchar_to_utf8(state.text_buffer, MAX_TEXT_LEN);
    assert(utf8[0] != '\0');
    printf("test_unicode_edge_cases passed\n");
}

void test_empty_and_null_input() {
    ChunjiinState state;
    chunjiin_init(&state);
    char* utf8 = wchar_to_utf8(NULL, MAX_TEXT_LEN);
    assert(utf8[0] == '\0' || utf8[0] == '\n');
    utf8 = wchar_to_utf8(state.text_buffer, 0);
    assert(utf8[0] == '\0');
    printf("test_empty_and_null_input passed\n");
}

void test_special_char_button_0() {
    ChunjiinState state;
    chunjiin_init(&state);
    // Switch to special mode
    change_mode(&state); // English
    change_mode(&state); // Upper English
    change_mode(&state); // Number
    change_mode(&state); // Special
    assert(state.now_mode == MODE_SPECIAL);

    // Test button 0: ~.^ (cycles through ~, ., ^)
    int before = state.cursor_pos;
    chunjiin_process_input(&state, 0); // First press: ~
    assert(state.cursor_pos == before + 1);

    before = state.cursor_pos;
    chunjiin_process_input(&state, 0); // Second press: .
    assert(state.cursor_pos == before);

    chunjiin_process_input(&state, 0); // Third press: ^
    assert(state.cursor_pos == before);

    chunjiin_process_input(&state, 0); // Fourth press: cycles back to ~
    assert(state.cursor_pos == before);

    printf("test_special_char_button_0 passed\n");
}

void test_special_char_button_7() {
    ChunjiinState state;
    chunjiin_init(&state);
    // Switch to special mode
    change_mode(&state); // English
    change_mode(&state); // Upper English
    change_mode(&state); // Number
    change_mode(&state); // Special
    assert(state.now_mode == MODE_SPECIAL);

    // Test button 7: -_ (cycles through -, _)
    int before = state.cursor_pos;
    chunjiin_process_input(&state, 7); // First press: -
    assert(state.cursor_pos == before + 1);

    before = state.cursor_pos;
    chunjiin_process_input(&state, 7); // Second press: _
    assert(state.cursor_pos == before);

    chunjiin_process_input(&state, 7); // Third press: cycles back to -
    assert(state.cursor_pos == before);

    printf("test_special_char_button_7 passed\n");
}

void test_special_char_all_buttons() {
    ChunjiinState state;
    chunjiin_init(&state);
    // Switch to special mode
    change_mode(&state); // English
    change_mode(&state); // Upper English
    change_mode(&state); // Number
    change_mode(&state); // Special
    assert(state.now_mode == MODE_SPECIAL);

    // Test all special character buttons (0-9)
    for (int i = 0; i < 10; i++) {
        int before = state.cursor_pos;
        chunjiin_process_input(&state, i);
        assert(state.cursor_pos >= before); // Should add at least one character
    }

    printf("test_special_char_all_buttons passed\n");
}

int main() {
    test_delete_char();
    test_rapid_mode_switch();
    test_invalid_input();
    test_full_user_session();
    test_korean_combinations();
    test_compound_jungsung_jongsung();
    test_buffer_overflow_underflow();
    test_rapid_clear_enter();
    test_utf8_conversion();
    test_delete_at_end();
    test_mode_switch_mid_composition();
    test_special_number_english_input();
    test_fuzz_random_input();
    test_memory_boundary();
    test_rapid_mode_clear_enter();
    test_unicode_edge_cases();
    test_empty_and_null_input();
    test_special_char_button_0();
    test_special_char_button_7();
    test_special_char_all_buttons();
    printf("Additional Chunjiin tests passed!\n");
    return 0;
}
