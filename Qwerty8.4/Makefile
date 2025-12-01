# Makefile for MAC Address Hex Input with LVGL

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -D_DEFAULT_SOURCE
LDFLAGS = -lm -lpthread

# LVGL directory and library
LVGL_DIR = ./lvgl
LVGL_LIB = $(LVGL_DIR)/build/liblvgl.a

# SDL2 for display driver
SDL_CFLAGS = $(shell pkg-config --cflags sdl2 2>/dev/null || echo "-I/usr/include/SDL2")
SDL_LIBS = $(shell pkg-config --libs sdl2 2>/dev/null || echo "-lSDL2")

# FreeType for TrueType font rendering
FREETYPE_CFLAGS = $(shell pkg-config --cflags freetype2 2>/dev/null || echo "-I/usr/include/freetype2")
FREETYPE_LIBS = $(shell pkg-config --libs freetype2 2>/dev/null || echo "-lfreetype")

# Include paths - lv_conf.h is in current directory
INCLUDES = -I. -I$(LVGL_DIR)

# Combine all flags
ALL_CFLAGS = $(CFLAGS) $(INCLUDES) $(SDL_CFLAGS) $(FREETYPE_CFLAGS)
ALL_LIBS = $(LVGL_LIB) $(SDL_LIBS) $(FREETYPE_LIBS) $(LDFLAGS)

TARGET = qwerty
OBJS = main.o qwerty.o

all: check_setup $(TARGET)

check_setup:
	@if [ ! -f "lv_conf.h" ]; then \
		echo "Error: lv_conf.h not found! Run ./setup.sh first."; \
		exit 1; \
	fi
	@if [ ! -f "$(LVGL_LIB)" ]; then \
		echo "Error: LVGL library not found! Run ./setup.sh first."; \
		exit 1; \
	fi

$(TARGET): $(OBJS)
	@echo "Linking $@..."
	$(CC) -o $(TARGET) $(OBJS) $(ALL_LIBS)

main.o: main.c qwerty.h lv_conf.h
	@echo "Compiling $<..."
	$(CC) $(ALL_CFLAGS) -c main.c

qwerty.o: qwerty.c qwerty.h
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c qwerty.c

clean:
	rm -f $(TARGET) main.o qwerty.o

clean-lvgl:
	@echo "Cleaning LVGL build directory..."
	@rm -rf $(LVGL_DIR)/build
	@echo "LVGL build directory cleaned. Run ./setup.sh to rebuild."

run: $(TARGET)
	./$(TARGET)

# Help target to show configuration
help:
	@echo "MAC Address Hex Input Makefile"
	@echo "=============================="
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build the application (default)"
	@echo "  clean    - Remove built files"
	@echo "  clean-lvgl - Remove LVGL build directory (run ./setup.sh to rebuild)"
	@echo "  run      - Build and run the application"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Dependencies:"
	@echo "  - LVGL (Light and Versatile Graphics Library)"
	@echo "  - SDL2 (for display driver)"
	@echo "  - FreeType (for font rendering)"
	@echo ""
	@echo "Current configuration:"
	@echo "  LVGL_DIR: $(LVGL_DIR)"
	@echo "  LVGL_LIB: $(LVGL_LIB)"
	@echo "  CFLAGS: $(ALL_CFLAGS)"
	@echo "  LIBS: $(ALL_LIBS)"
	@echo ""
	@echo "To install dependencies and build LVGL:"
	@echo "  ./setup.sh"
	@echo ""
	@echo "Then build the application:"
	@echo "  make"

.PHONY: all clean clean-lvgl run help check_setup
