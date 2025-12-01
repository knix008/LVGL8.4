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

# JPEG for JPEG image support
JPEG_LIBS = $(shell pkg-config --libs libjpeg 2>/dev/null || echo "-ljpeg")

# Include paths - lv_conf.h is in current directory
INCLUDES = -I. -I$(LVGL_DIR)

# Combine all flags
ALL_CFLAGS = $(CFLAGS) $(INCLUDES) $(SDL_CFLAGS) $(FREETYPE_CFLAGS)
ALL_LIBS = $(LVGL_LIB) $(SDL_LIBS) $(FREETYPE_LIBS) $(JPEG_LIBS) $(LDFLAGS)

TARGET = input
SRC_DIR = src
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/home.c $(SRC_DIR)/menu.c $(SRC_DIR)/screen.c $(SRC_DIR)/style.c $(SRC_DIR)/init.c $(SRC_DIR)/info.c $(SRC_DIR)/admin.c $(SRC_DIR)/network.c $(SRC_DIR)/korean_input.c $(SRC_DIR)/chunjiin.c $(SRC_DIR)/chunjiin_hangul.c $(SRC_DIR)/navigation.c $(SRC_DIR)/screen_components.c
OBJS = $(SRC_DIR)/main.o $(SRC_DIR)/home.o $(SRC_DIR)/menu.o $(SRC_DIR)/screen.o $(SRC_DIR)/style.o $(SRC_DIR)/init.o $(SRC_DIR)/info.o $(SRC_DIR)/admin.o $(SRC_DIR)/network.o $(SRC_DIR)/korean_input.o $(SRC_DIR)/chunjiin.o $(SRC_DIR)/chunjiin_hangul.o $(SRC_DIR)/navigation.o $(SRC_DIR)/screen_components.o

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

%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(ALL_CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

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
