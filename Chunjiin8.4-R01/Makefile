# Makefile for Chunjiin Korean Input Method with LVGL

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I. -Ilvgl $(shell pkg-config --cflags freetype2)
LDFLAGS = -Llvgl/lib -llvgl -lSDL2 -lpthread -lm $(shell pkg-config --libs freetype2)
TARGET = chunjiin
SOURCES = main.c chunjiin.c input.c

# Application object files only
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean install run setup help

all: check_lvgl $(TARGET)

# Check if LVGL library exists
check_lvgl:
	@if [ ! -f "lvgl/lib/liblvgl.a" ]; then \
		echo "Error: LVGL library (lvgl/lib/liblvgl.a) not found!"; \
		echo "Please run './setup.sh' first to build LVGL library."; \
		exit 1; \
	fi

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f *.txt *.log

# Clean everything including LVGL build artifacts
clean-all: clean
	rm -rf lvgl

install: $(TARGET)
	@echo "Installing $(TARGET)..."
	cp $(TARGET) /usr/local/bin/ || cp $(TARGET).exe /usr/local/bin/
	@echo "Installation complete!"

run: $(TARGET)
	./$(TARGET)

# Setup LVGL
setup:
	@echo "Setting up LVGL..."
	@if [ ! -d "lvgl" ]; then \
		echo "Cloning LVGL v8.4 repository..."; \
		git clone --depth 1 --branch release/v8.4 https://github.com/lvgl/lvgl.git; \
	else \
		echo "LVGL directory already exists"; \
	fi
	@echo "Setup complete!"
	@echo ""
	@echo "Next steps:"
	@echo "1. Make sure lv_conf.h is properly configured"
	@echo "2. Make sure SDL2 is installed: sudo apt-get install libsdl2-dev"
	@echo "3. Run 'make' to build the application"

help:
	@echo "Chunjiin Korean Input Method - Makefile (LVGL)"
	@echo ""
	@echo "Available targets:"
	@echo "  ./setup.sh    - Complete setup (clone LVGL, build library, build app)"
	@echo "  make          - Build the application (requires liblvgl.a)"
	@echo "  make clean    - Remove application build files"
	@echo "  make clean-all- Remove all build files including LVGL library"
	@echo "  make run      - Build and run the application"
	@echo "  make install  - Install to /usr/local/bin"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Build process:"
	@echo "  1. Run './setup.sh' once to setup LVGL and build the library"
	@echo "  2. Use 'make' for subsequent builds (much faster!)"
	@echo ""
	@echo "Requirements:"
	@echo "  - SDL2 development libraries (libsdl2-dev)"
	@echo "  - FreeType development libraries (libfreetype6-dev)"
	@echo "  - LVGL v8.4 (cloned and built by setup.sh)"
