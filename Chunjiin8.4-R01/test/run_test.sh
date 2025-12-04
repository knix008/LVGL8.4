#!/bin/bash
# Run all Chunjiin test cases in the test directory
# This script compiles and executes all test suites for the Chunjiin input system
#
# Usage:
#   ./run_test.sh          # Run all tests
#   ./run_test.sh --clean  # Clean and run tests
#   ./run_test.sh --help   # Show this help message

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Help function
show_help() {
    echo "Chunjiin Input System Test Runner"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  (no args)   Run all tests"
    echo "  --clean     Clean build artifacts before testing"
    echo "  --help      Show this help message"
    echo ""
    echo "Test Suites:"
    echo "  test_chunjiin           Core functionality tests (5 tests)"
    echo "  test_chunjiin_extra     Advanced and edge-case tests (20 tests)"
    echo "  test_regression         Regression and safety tests (17 tests)"
    exit 0
}

# Parse arguments
CLEAN_BUILD=0
case "${1:-}" in
    --help)
        show_help
        ;;
    --clean)
        CLEAN_BUILD=1
        ;;
    "")
        CLEAN_BUILD=0
        ;;
    *)
        echo "Unknown option: $1" >&2
        show_help
        ;;
esac

# Clean if requested
if [ $CLEAN_BUILD -eq 1 ]; then
    echo -e "${BLUE}Cleaning test artifacts...${NC}"
    make clean
fi

# Build tests
echo -e "${BLUE}Building tests...${NC}"
make

# Run core tests
echo -e "\n${BLUE}Running test_chunjiin...${NC}"
if ./test_chunjiin; then
    echo -e "${GREEN}✓ Core tests passed${NC}"
else
    echo -e "${RED}✗ Core tests failed${NC}"
    exit 1
fi

# Run advanced tests
echo -e "\n${BLUE}Running test_chunjiin_extra...${NC}"
if ./test_chunjiin_extra; then
    echo -e "${GREEN}✓ Advanced tests passed${NC}"
else
    echo -e "${RED}✗ Advanced tests failed${NC}"
    exit 1
fi

# Run regression tests
echo -e "\n${BLUE}Running test_regression...${NC}"
if ./test_regression; then
    echo -e "${GREEN}✓ Regression tests passed${NC}"
else
    echo -e "${RED}✗ Regression tests failed${NC}"
    exit 1
fi

# Summary
echo -e "\n${GREEN}All tests completed successfully!${NC}"
echo ""
echo "Test Summary:"
echo "  Core tests:             5 test functions"
echo "  Advanced tests:         20 test functions"
echo "  Regression tests:       17 test functions"
echo "  ────────────────────────────────────────"
echo "  Total:                  42 comprehensive test cases"
echo ""
echo "For more information, see README.md"
