#ifndef INPUT_H
#define INPUT_H

#include "chunjiin.h"

// ============================================================================
// INPUT MODE PROCESSING FUNCTIONS
// ============================================================================

/**
 * Process Hangul input
 * @param state Pointer to ChunjiinState structure
 * @param input Input button number
 */
void hangul_make(ChunjiinState *state, int input);

/**
 * Process English input
 * @param state Pointer to ChunjiinState structure
 * @param input Input button number
 */
void eng_make(ChunjiinState *state, int input);

/**
 * Process number input
 * @param state Pointer to ChunjiinState structure
 * @param input Input button number
 */
void num_make(ChunjiinState *state, int input);

/**
 * Process special character input
 * @param state Pointer to ChunjiinState structure
 * @param input Input button number
 */
void special_make(ChunjiinState *state, int input);

// ============================================================================
// TEXT OPERATIONS
// ============================================================================

/**
 * Write Hangul character to text buffer
 * @param state Pointer to ChunjiinState structure
 */
void write_hangul(ChunjiinState *state);

/**
 * Write English/number character to text buffer
 * @param state Pointer to ChunjiinState structure
 */
void write_engnum(ChunjiinState *state);

/**
 * Delete character at cursor position
 * @param state Pointer to ChunjiinState structure
 */
void delete_char(ChunjiinState *state);

#endif // INPUT_H
