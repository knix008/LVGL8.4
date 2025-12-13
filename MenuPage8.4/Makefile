CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -D_DEFAULT_SOURCE
LDFLAGS = -lm -lpthread
MAKEFLAGS += -j4

# LVGL directory and library
LVGL_DIR = ./lvgl
LVGL_LIB = $(LVGL_DIR)/build/liblvgl.a

# SDL2 for display driver
SDL_CFLAGS = $(shell pkg-config --cflags sdl2 2>/dev/null || echo "-I/usr/include/SDL2")
SDL_LIBS = $(shell pkg-config --libs sdl2 2>/dev/null || echo "-lSDL2")

# FreeType for TrueType font rendering
FREETYPE_CFLAGS = $(shell pkg-config --cflags freetype2 2>/dev/null || echo "-I/usr/include/freetype2")
FREETYPE_LIBS = $(shell pkg-config --libs freetype2 2>/dev/null || echo "-lfreetype")

# JPEG for JPEG image support
JPEG_LIBS = $(shell pkg-config --libs libjpeg 2>/dev/null || echo "-ljpeg")

# FFmpeg for video playback
FFMPEG_CFLAGS = $(shell pkg-config --cflags libavformat libavcodec libavutil libswscale 2>/dev/null || echo "")
FFMPEG_LIBS = $(shell pkg-config --libs libavformat libavcodec libavutil libswscale 2>/dev/null || echo "-lavformat -lavcodec -lavutil -lswscale")

# Include paths - lv_conf.h is in current directory
INCLUDES = -I. -I$(LVGL_DIR)

# Combine all flags
ALL_CFLAGS = $(CFLAGS) $(INCLUDES) $(SDL_CFLAGS) $(FREETYPE_CFLAGS) $(FFMPEG_CFLAGS)
ALL_LIBS = $(LVGL_LIB) $(SDL_LIBS) $(FREETYPE_LIBS) $(JPEG_LIBS) $(FFMPEG_LIBS) $(LDFLAGS)

# Build directory
BUILD_DIR = build
TARGET = application
SRC_DIR = src

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
STATE_SRCS = $(wildcard $(SRC_DIR)/state/*.c)

# Object files in build directory
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
STATE_OBJS = $(patsubst $(SRC_DIR)/state/%.c,$(BUILD_DIR)/state_%.o,$(STATE_SRCS))

# All objects
ALL_OBJS = $(OBJS) $(STATE_OBJS)

all: check_setup $(BUILD_DIR) $(TARGET)
	@echo "Build complete."

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

check_setup:
	@if [ ! -f "lv_conf.h" ]; then \
		echo "Error: lv_conf.h not found! Run ./setup.sh first."; \
		exit 1; \
	fi
	@if [ ! -f "$(LVGL_LIB)" ]; then \
		echo "Error: LVGL library not found! Run ./setup.sh first."; \
		exit 1; \
	fi

$(TARGET): $(ALL_OBJS)
	@echo "Linking $(TARGET)..."
	@$(CC) -o $(TARGET) $(ALL_OBJS) $(ALL_LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	@$(CC) $(ALL_CFLAGS) -c $< -o $@

$(BUILD_DIR)/state_%.o: $(SRC_DIR)/state/%.c
	@echo "Compiling $<..."
	@$(CC) $(ALL_CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET)
	rm -rf $(BUILD_DIR)

clean-lvgl:
	@echo "Cleaning LVGL build directory..."
	@rm -rf $(LVGL_DIR)/build
	@echo "LVGL build directory cleaned. Run ./setup.sh to rebuild."

run: $(TARGET)
	./$(TARGET)

# Help target to show configuration
help:
	@echo "Title Bar Makefile"
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
