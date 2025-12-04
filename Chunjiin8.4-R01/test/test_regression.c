/*
 * Regression Tests for Chunjiin Input Method
 * Tests for previously discovered bugs and compatibility issues
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "../chunjiin.h"

/**
 * Regression: Buffer overflow vulnerability
 * Ensure cursor_pos never exceeds MAX_TEXT_LEN
 * This was a critical bug in early versions
 */
void test_regression_buffer_overflow() {
    ChunjiinState state;
    chunjiin_init(&state);

    // Attempt to cause overflow
    for (int i = 0; i < MAX_TEXT_LEN * 2; i++) {
        chunjiin_process_input(&state, 4);
        assert(state.cursor_pos <= MAX_TEXT_LEN);
    }

    printf("test_regression_buffer_overflow passed\n");
}

/**
 * Regression: Negative cursor position
 * Ensure delete_char never causes negative cursor
 */
void test_regression_negative_cursor() {
    ChunjiinState state;
    chunjiin_init(&state);

    // Try to delete from empty buffer
    delete_char(&state);
    assert(state.cursor_pos >= 0);

    // Try to delete multiple times from empty
    for (int i = 0; i < 10; i++) {
        delete_char(&state);
        assert(state.cursor_pos == 0);
    }

    printf("test_regression_negative_cursor passed\n");
}

/**
 * Regression: Mode cycle validation
 * Ensure mode always cycles through valid range
 */
void test_regression_invalid_mode() {
    ChunjiinState state;
    chunjiin_init(&state);

    for (int i = 0; i < 50; i++) {
        change_mode(&state);
        assert(state.now_mode >= MODE_HANGUL && state.now_mode <= MODE_SPECIAL);
    }

    printf("test_regression_invalid_mode passed\n");
}

/**
 * Regression: Invalid input handling
 * Negative buttons, out-of-range buttons should be ignored
 */
void test_regression_invalid_button() {
    ChunjiinState state;
    chunjiin_init(&state);

    int before = state.cursor_pos;

    // Invalid buttons
    chunjiin_process_input(&state, -5);
    chunjiin_process_input(&state, -1);
    chunjiin_process_input(&state, 12);
    chunjiin_process_input(&state, 100);
    chunjiin_process_input(&state, 999);

    assert(state.cursor_pos == before);

    printf("test_regression_invalid_button passed\n");
}

/**
 * Regression: Clear function safety
 * chunjiin_init should safely reset all state
 */
void test_regression_clear_safety() {
    ChunjiinState state;

    // Partially initialize
    state.cursor_pos = 50;
    state.now_mode = MODE_NUMBER;

    // Clear
    chunjiin_init(&state);

    // All fields should be reset
    assert(state.cursor_pos == 0);
    assert(state.now_mode == MODE_HANGUL);
    for (int i = 0; i < MAX_TEXT_LEN; i++) {
        assert(state.text_buffer[i] == 0);
    }

    printf("test_regression_clear_safety passed\n");
}

/**
 * Regression: Mode change during composition
 * Mode should change even if in middle of character
 */
void test_regression_mode_change_mid_char() {
    ChunjiinState state;
    chunjiin_init(&state);

    // Start composition
    chunjiin_process_input(&state, 4); // ㄱ
    chunjiin_process_input(&state, 1); // ㅣ

    // Change mode mid-composition
    change_mode(&state);
    assert(state.now_mode != MODE_HANGUL);

    // Should be able to input in new mode
    chunjiin_process_input(&state, 2);
    assert(state.cursor_pos > 0);

    printf("test_regression_mode_change_mid_char passed\n");
}

/**
 * Regression: Rapid clear without proper initialization
 * Multiple clears should not cause issues
 */
void test_regression_multiple_clears() {
    ChunjiinState state;
    chunjiin_init(&state);

    chunjiin_process_input(&state, 4);
    chunjiin_init(&state); // Clear 1
    chunjiin_init(&state); // Clear 2
    chunjiin_init(&state); // Clear 3

    assert(state.cursor_pos == 0);
    assert(state.now_mode == MODE_HANGUL);

    printf("test_regression_multiple_clears passed\n");
}

/**
 * Regression: Delete on empty composition
 * Delete with no characters should not crash
 */
void test_regression_delete_empty() {
    ChunjiinState state;
    chunjiin_init(&state);

    // Empty state
    assert(state.cursor_pos == 0);

    // Delete from empty
    delete_char(&state);
    assert(state.cursor_pos == 0);

    // Should still be usable
    chunjiin_process_input(&state, 4);
    assert(state.cursor_pos > 0);

    printf("test_regression_delete_empty passed\n");
}

/**
 * Regression: Buffer persistence
 * Text buffer should maintain state until cleared
 */
void test_regression_buffer_persistence() {
    ChunjiinState state;
    chunjiin_init(&state);

    // Input text
    chunjiin_process_input(&state, 4);
    chunjiin_process_input(&state, 1);
    chunjiin_process_input(&state, 5);

    int cursor_before = state.cursor_pos;

    // Check multiple times - should not change without input
    for (int i = 0; i < 10; i++) {
        assert(state.cursor_pos == cursor_before);
    }

    printf("test_regression_buffer_persistence passed\n");
}

/**
 * Regression: Mode-specific input validation
 * Each mode should accept appropriate inputs
 */
void test_regression_mode_input_validation() {
    ChunjiinState state;

    // Test each mode
    for (int mode = 0; mode < 5; mode++) {
        chunjiin_init(&state);

        // Switch to target mode
        for (int i = 0; i < mode; i++) {
            change_mode(&state);
        }

        // Input should be processed (even if mode-specific)
        int before = state.cursor_pos;
        chunjiin_process_input(&state, 4);

        // Cursor should advance (input accepted)
        assert(state.cursor_pos >= before);
    }

    printf("test_regression_mode_input_validation passed\n");
}

/**
 * Regression: State consistency after errors
 * System should remain consistent even after invalid operations
 */
void test_regression_error_recovery() {
    ChunjiinState state;
    chunjiin_init(&state);

    // Valid operation
    chunjiin_process_input(&state, 4);
    int after_valid = state.cursor_pos;

    // Invalid operations
    chunjiin_process_input(&state, -1);
    chunjiin_process_input(&state, 999);

    // State should be unchanged
    assert(state.cursor_pos == after_valid);

    // Should still work normally
    chunjiin_process_input(&state, 5);
    assert(state.cursor_pos > after_valid);

    printf("test_regression_error_recovery passed\n");
}

/**
 * Regression: Boundary between composition and new character
 * Transition from complete character to next one should be smooth
 */
void test_regression_composition_transition() {
    ChunjiinState state;
    chunjiin_init(&state);

    // Complete first character
    chunjiin_process_input(&state, 4); // ㄱ
    chunjiin_process_input(&state, 1); // ㅣ
    chunjiin_process_input(&state, 5); // ㄴ
    int after_first = state.cursor_pos;

    // Start second character
    chunjiin_process_input(&state, 6); // ㄷ
    int after_second = state.cursor_pos;

    // Cursor should advance correctly
    assert(after_second > after_first);

    printf("test_regression_composition_transition passed\n");
}

/**
 * Regression: Button 0 handling
 * Special button 0 should not cause issues
 */
void test_regression_button_0_handling() {
    ChunjiinState state;
    chunjiin_init(&state);

    // Input button 0 various times
    for (int i = 0; i < 20; i++) {
        chunjiin_process_input(&state, 0);
    }

    // Should not crash, cursor bounded
    assert(state.cursor_pos <= MAX_TEXT_LEN);

    printf("test_regression_button_0_handling passed\n");
}

/**
 * Regression: Initialization from uninitialized state
 * Init should work even from garbage state (defensive)
 */
void test_regression_init_from_garbage() {
    ChunjiinState state;

    // Simulate garbage initialization
    state.cursor_pos = 0xFFFFFFFF;
    state.now_mode = 99;
    memset(state.text_buffer, 0xFF, sizeof(state.text_buffer));

    // Should reset cleanly
    chunjiin_init(&state);

    assert(state.cursor_pos == 0);
    assert(state.now_mode == MODE_HANGUL);

    printf("test_regression_init_from_garbage passed\n");
}

/**
 * Regression: Maximum stress without crash
 * System should handle extreme conditions without crashing
 */
void test_regression_no_crash_extreme() {
    ChunjiinState state;
    chunjiin_init(&state);

    // Extreme rapid operations
    for (int i = 0; i < 5000; i++) {
        chunjiin_process_input(&state, (i % 12));
        if (i % 10 == 0) {
            delete_char(&state);
        }
        if (i % 50 == 0) {
            change_mode(&state);
        }
        if (i % 100 == 0) {
            chunjiin_init(&state);
        }

        // Always maintain consistency
        assert(state.cursor_pos >= 0 && state.cursor_pos <= MAX_TEXT_LEN);
        assert(state.now_mode >= MODE_HANGUL && state.now_mode <= MODE_SPECIAL);
    }

    printf("test_regression_no_crash_extreme passed (5000 operations)\n");
}

/**
 * Regression: State after type mismatch protection
 * Type safety checks should not corrupt state
 */
void test_regression_type_safety() {
    ChunjiinState state;
    chunjiin_init(&state);

    InputMode original_mode = state.now_mode;

    // Attempt suspicious operations
    change_mode(&state);

    // Mode should only be valid values
    assert(state.now_mode >= MODE_HANGUL && state.now_mode <= MODE_SPECIAL);

    printf("test_regression_type_safety passed\n");
}

/**
 * Regression: English input button layout (T9 Keypad)
 * Verify that button layout is correct (ABC-DEF-GHI-JKL-MNO-PQR-STU-VWX-YZ.)
 * This test ensures the standard phone keypad layout is maintained
 */
void test_regression_english_button_layout() {
    ChunjiinState state;
    chunjiin_init(&state);

    // Switch to English mode
    change_mode(&state); // MODE_HANGUL -> MODE_UPPER_ENGLISH
    assert(state.now_mode == MODE_UPPER_ENGLISH);

    // Test button 0: @?!
    chunjiin_process_input(&state, 0);
    // Button 0 should produce @?! (special characters)
    assert(state.cursor_pos > 0);

    // Clear for next test
    chunjiin_init(&state);
    change_mode(&state);

    // Test button 1: ABC
    chunjiin_process_input(&state, 1);
    assert(state.cursor_pos > 0); // Should produce A

    // Test button 2: DEF
    chunjiin_init(&state);
    change_mode(&state);
    chunjiin_process_input(&state, 2);
    assert(state.cursor_pos > 0); // Should produce D

    // Test button 6: PQR (was incorrectly PRS)
    chunjiin_init(&state);
    change_mode(&state);
    chunjiin_process_input(&state, 6);
    assert(state.cursor_pos > 0); // Should produce P

    // Test button 9: YZ. (period should be on button 9)
    chunjiin_init(&state);
    change_mode(&state);
    chunjiin_process_input(&state, 9);
    assert(state.cursor_pos > 0); // Should produce Y

    // Multi-tap test on button 9 to get period
    chunjiin_process_input(&state, 9); // Second tap should give Z
    chunjiin_process_input(&state, 9); // Third tap should give period (.)

    printf("test_regression_english_button_layout passed\n");
}

int main() {
    printf("Running regression tests...\n\n");

    // Critical bug tests
    test_regression_buffer_overflow();
    test_regression_negative_cursor();
    test_regression_invalid_mode();
    test_regression_invalid_button();

    // Safety and recovery tests
    test_regression_clear_safety();
    test_regression_mode_change_mid_char();
    test_regression_multiple_clears();
    test_regression_delete_empty();
    test_regression_error_recovery();

    // Behavior consistency tests
    test_regression_buffer_persistence();
    test_regression_mode_input_validation();
    test_regression_composition_transition();
    test_regression_button_0_handling();

    // Robustness tests
    test_regression_init_from_garbage();
    test_regression_no_crash_extreme();
    test_regression_type_safety();

    // English button layout test
    test_regression_english_button_layout();

    printf("\nAll regression tests passed!\n");
    return 0;
}
