CC=gcc
CFLAGS=-I..
SRC_DIR=..

TESTS=test_chunjiin test_chunjiin_extra test_regression

all: $(TESTS)

# Build each test

test_chunjiin: test_chunjiin.c $(SRC_DIR)/chunjiin.c $(SRC_DIR)/chunjiin_hangul.c $(SRC_DIR)/chunjiin.h
	$(CC) $(CFLAGS) -o $@ test_chunjiin.c $(SRC_DIR)/chunjiin.c $(SRC_DIR)/chunjiin_hangul.c

test_chunjiin_extra: test_chunjiin_extra.c $(SRC_DIR)/chunjiin.c $(SRC_DIR)/chunjiin_hangul.c $(SRC_DIR)/chunjiin.h
	$(CC) $(CFLAGS) -o $@ test_chunjiin_extra.c $(SRC_DIR)/chunjiin.c $(SRC_DIR)/chunjiin_hangul.c

test_regression: test_regression.c $(SRC_DIR)/chunjiin.c $(SRC_DIR)/chunjiin_hangul.c $(SRC_DIR)/chunjiin.h
	$(CC) $(CFLAGS) -o $@ test_regression.c $(SRC_DIR)/chunjiin.c $(SRC_DIR)/chunjiin_hangul.c

clean:
	rm -f $(TESTS)
