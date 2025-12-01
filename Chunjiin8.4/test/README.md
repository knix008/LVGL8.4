# Chunjiin Input System Test Suite

This directory contains comprehensive automated tests for the Chunjiin (천지인) Korean input system, including unit tests, integration tests, edge cases, and fuzz testing.

## Quick Start

### Run All Tests

```bash
cd test
./run_test.sh
```

This will automatically:
1. Compile all test binaries
2. Run core functionality tests (`test_chunjiin`)
3. Run advanced/edge-case tests (`test_chunjiin_extra`)
4. Report test results

### Expected Output

All tests should pass with output similar to:
```
Building tests...
gcc -I.. -o test_chunjiin test_chunjiin.c ../chunjiin.c ../chunjiin_hangul.c
gcc -I.. -o test_chunjiin_extra test_chunjiin_extra.c ../chunjiin.c ../chunjiin_hangul.c
Running test_chunjiin...
test_init passed
test_hangul_input passed
test_mode_switch passed
test_buffer_limit passed
test_clear_and_enter passed
All Chunjiin tests passed!

Running test_chunjiin_extra...
[Advanced tests output...]
All advanced tests passed!

All tests completed.
```

## Test Files

### Core Tests (`test_chunjiin.c`)
- **test_init**: Verify state initialization and default mode
- **test_hangul_input**: Korean character composition (consonant + vowel + final)
- **test_mode_switch**: Mode cycling (한글 → 영문 → 숫자 → 특수문자 → 한글)
- **test_buffer_limit**: Buffer overflow protection (MAX_TEXT_LEN boundary)
- **test_clear_and_enter**: Clear and Enter key functionality

### Advanced Tests (`test_chunjiin_extra.c`)
- **Hangul character composition**: Complex syllable combinations
- **Consonant handling**: All 19 Korean consonants
- **Vowel handling**: All Korean vowel combinations
- **Double consonants**: ㄲ, ㄸ, ㅃ, ㅆ, ㅉ handling
- **Final consonants**: All 27 possible final consonant combinations
- **English input (T9 Keypad)**:
  - Button layout verification (ABC, DEF, GHI, JKL, MNO, PQR, STU, VWX, YZ.)
  - Multi-tap character cycling
  - Period (.) input on button 9
- **Mode switching**: English (lowercase), English (uppercase), Numbers, Special characters
- **Backspace/Delete**: Character removal and decomposition
- **Buffer management**: Boundary conditions and overflow prevention
- **Unicode output**: UTF-8 encoding verification
- **Edge cases**: Rapid input, state transitions, error conditions
- **Fuzz testing**: Random inputs and boundary testing

## Build System

### Files
- `test_chunjiin.c`: Core functionality tests (~75 lines, 5 test functions)
- `test_chunjiin_extra.c`: Advanced tests (~400+ lines, 25+ test functions)
- `Makefile`: Build rules for all test binaries
- `run_test.sh`: Script to build and run all tests
- `TEST_UPDATES.md`: Documentation of test additions and improvements

### Building Individually

Compile a specific test:
```bash
cd test
gcc -I.. -o test_chunjiin test_chunjiin.c ../chunjiin.c ../chunjiin_hangul.c
./test_chunjiin
```

Or use the Makefile:
```bash
cd test
make test_chunjiin
make test_chunjiin_extra
make clean  # Remove all built test binaries
```

## Test Coverage

### Functionality Coverage
- ✓ State initialization
- ✓ Mode switching (5 modes)
- ✓ Hangul composition (consonants, vowels, finals)
- ✓ English input (lowercase and uppercase)
- ✓ Number input (0-9)
- ✓ Special character input
- ✓ Backspace/Delete functionality
- ✓ Buffer management and overflow protection
- ✓ Clear/Reset functionality
- ✓ Enter/Submit functionality

### Test Categories
- **Unit Tests**: Individual function behavior
- **Integration Tests**: Multi-step input sequences
- **Edge Cases**: Boundary conditions, buffer limits, rapid transitions
- **Fuzz Tests**: Random inputs and stress testing
- **Unicode Tests**: UTF-8 encoding and character conversion

## Adding New Tests

1. Create a new `.c` test file (e.g., `test_new_feature.c`)
2. Include required headers:
   ```c
   #include <stdio.h>
   #include <string.h>
   #include <assert.h>
   #include "../chunjiin.h"
   ```
3. Implement test functions using `assert()` for verification
4. Add a `main()` function that runs all tests
5. Update `Makefile`:
   ```makefile
   test_new_feature: test_new_feature.c $(SRC_DIR)/chunjiin.c $(SRC_DIR)/chunjiin_hangul.c $(SRC_DIR)/chunjiin.h
       $(CC) $(CFLAGS) -o $@ test_new_feature.c $(SRC_DIR)/chunjiin.c $(SRC_DIR)/chunjiin_hangul.c
   ```
6. Update `run_test.sh` to invoke the new test:
   ```bash
   echo "Running test_new_feature..."
   ./test_new_feature
   ```

## Continuous Integration

To integrate tests into your CI/CD pipeline:

```bash
#!/bin/bash
cd test
./run_test.sh || exit 1
echo "All tests passed!"
```

## Troubleshooting

### Tests won't compile
- Verify `chunjiin.c`, `chunjiin_hangul.c`, and `chunjiin.h` exist in parent directory
- Check that CFLAGS `-I..` is correctly set in Makefile
- Ensure gcc or compatible C compiler is installed

### Tests fail
- Check console output for assertion failures
- Review the specific test function that failed
- Verify chunjiin.c implementation matches test expectations
- Run individual test files to isolate failures

### Performance
- Tests should complete in < 1 second total
- If slow, check for infinite loops or very large test iterations

---

# Chunjiin Input System Test Cases

This file lists test cases for the Chunjiin input system. Each test case should be implemented in a corresponding C test file.

## Test Case Ideas

1. Initialization
   - State is zeroed after `chunjiin_init`
   - Mode is set to Hangul by default
2. Hangul Input
   - Single character input (e.g., ㄱ, ㅏ, ㄴ)
   - Compose a full syllable (e.g., ㄱ + ㅏ + ㄴ → 간)
   - Compose multiple syllables in sequence
   - Handle double consonants and vowels
   - Handle special characters (ㆍ, ㅡ, etc.)
3. English/Number/Special Mode
   - Switch to English mode and input letters
   - Switch to Number mode and input numbers
   - Switch to Special mode and input symbols
   - Mode cycling (Hangul → English → Number → Special → Hangul)
4. Buffer Handling
   - Input up to buffer limit (MAX_TEXT_LEN)
   - Input after buffer is full (should not overflow)
   - Clear buffer and check state
   - Enter key after buffer is cleared (should not crash)
   - Enter key with non-empty buffer
5. Cursor Handling
   - Cursor at start, middle, end
   - Delete character at various positions
   - Delete at buffer start (should not crash)
6. Edge Cases
   - Rapid mode switching
   - Rapid clear/enter presses
   - Invalid input values (negative, >11)
   - Null/empty input
7. Unicode/UTF-8 Conversion
   - Correct conversion for Hangul syllables
   - Correct conversion for English/numbers/specials
8. Integration
   - Simulate full user session: input, clear, enter, mode switch, etc.

---

Implement these as C unit tests, e.g. using Unity, CMocka, or a simple assert-based framework.