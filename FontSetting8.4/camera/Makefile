# GTK Webcam Application Makefile with FAISS + Deep Learning

CXX := g++
CXXFLAGS := -Wall -Wextra -O2 -std=c++17 -fPIC
INCLUDES := -Iinclude $(shell pkg-config --cflags gtk+-3.0 gdk-pixbuf-2.0 opencv4)

# ONNX Runtime - check if locally installed or use system package
ONNX_LOCAL_DIR := $(wildcard onnxruntime-linux-*/)
ifdef ONNX_LOCAL_DIR
  # Local ONNX Runtime installation (from install_onnxruntime.sh)
  ONNX_INCLUDES := -I$(ONNX_LOCAL_DIR)/include
  ONNX_LIBS := -L$(ONNX_LOCAL_DIR)/lib -lonnxruntime
  INCLUDES += $(ONNX_INCLUDES)
  $(info Using local ONNX Runtime: $(ONNX_LOCAL_DIR))
else
  # System ONNX Runtime installation
  ONNX_LIBS := -lonnxruntime
  $(info Using system ONNX Runtime)
endif

# FAISS - local installation is optional (we use in-memory implementation)
FAISS_LOCAL_DIR := faiss
ifeq ($(wildcard $(FAISS_LOCAL_DIR)/lib/libfaiss.so),)
  $(info FAISS not found - using in-memory index implementation)
  FAISS_LIBS :=
else
  FAISS_INCLUDES := -I$(FAISS_LOCAL_DIR)/include
  FAISS_LIBS := -L$(FAISS_LOCAL_DIR)/lib -lfaiss -Wl,-rpath,$(shell pwd)/$(FAISS_LOCAL_DIR)/lib
  INCLUDES += $(FAISS_INCLUDES)
  $(info Using local FAISS: $(FAISS_LOCAL_DIR))
endif

# Combine all libraries
LIBS := $(shell pkg-config --libs gtk+-3.0 gdk-pixbuf-2.0 opencv4) -pthread -lsqlite3 $(ONNX_LIBS) $(FAISS_LIBS)

# Directories
SRC_DIR := src
OBJ_DIR := build
INCLUDE_DIR := include
CLIENT_DIR := client
CLIENT_SRC_DIR := $(CLIENT_DIR)/src
CLIENT_INCLUDE_DIR := $(CLIENT_DIR)/include
CLIENT_OBJ_DIR := $(OBJ_DIR)/client

# Source and object files
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
# Exclude GTK client GUI files from main build (main app uses socket server directly)
SOURCES := $(filter-out $(SRC_DIR)/gtk_client.cpp $(SRC_DIR)/gtk_client_main.cpp $(SRC_DIR)/socket_client_lib.cpp, $(SOURCES))
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

# Shared protocol object (used by both server and client)
PROTOCOL_OBJ := $(OBJ_DIR)/protocol.o

# Client sources
CLIENT_SOURCES := $(wildcard $(CLIENT_SRC_DIR)/*.cpp)
CLIENT_OBJECTS := $(patsubst $(CLIENT_SRC_DIR)/%.cpp, $(CLIENT_OBJ_DIR)/%.o, $(CLIENT_SOURCES))

TARGET := gtk_webcam
SOCKET_CLIENT := socket_client
GTK_CLIENT := gtk_client

# Default target - only build main application (clients not needed)
all: $(TARGET)

# Create directories
$(OBJ_DIR) $(CLIENT_OBJ_DIR):
	@mkdir -p $@

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile client source files
$(CLIENT_OBJ_DIR)/%.o: $(CLIENT_SRC_DIR)/%.cpp | $(CLIENT_OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I$(CLIENT_INCLUDE_DIR) -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS) $(PROTOCOL_OBJ)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@
	@echo "Build completed: $(TARGET)"

# Build socket client
$(SOCKET_CLIENT): $(CLIENT_OBJ_DIR)/socket_client.o $(PROTOCOL_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build completed: $(SOCKET_CLIENT)"

# Build GTK client
$(GTK_CLIENT): $(CLIENT_OBJ_DIR)/socket_client_lib.o $(CLIENT_OBJ_DIR)/gtk_client.o $(CLIENT_OBJ_DIR)/gtk_client_main.o $(OBJ_DIR)/logger.o $(PROTOCOL_OBJ)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@
	@echo "Build completed: $(GTK_CLIENT)"

# Run the application
run: $(TARGET)
	@echo "Starting GTK Webcam Viewer..."
	./$(TARGET)

# Debug build
debug: CXXFLAGS := -Wall -Wextra -g -std=c++17 -fPIC
debug: clean $(TARGET)
	@echo "Debug build completed"

# Run with GDB debugger
debug-run: debug
	gdb ./$(TARGET)

# Clean build artifacts (keep external dependencies)
clean:
	@rm -rf $(OBJ_DIR) $(TARGET) $(SOCKET_CLIENT) $(GTK_CLIENT)
	@rm -rf *.db *.bin
	@rm -rf dataset/*
	@echo "Cleaned build artifacts"

# Full clean including external dependencies (be careful!)
distclean: clean
	@rm -rf onnxruntime-*/
	@rm -rf faiss/
	@echo "Cleaned all artifacts including external dependencies"

# Help target
help:
	@echo "GTK Webcam Application - Build Targets"
	@echo "======================================"
	@echo "make          - Build the application, socket client, and GTK client"
	@echo "make run      - Build and run the main application"
	@echo "make debug    - Build with debug symbols"
	@echo "make debug-run - Build and run with GDB debugger"
	@echo "make clean    - Remove build artifacts (keeps ONNX Runtime & FAISS)"
	@echo "make distclean - Remove all artifacts including ONNX Runtime & FAISS"
	@echo "make help     - Show this help message"
	@echo ""
	@echo "Executables:"
	@echo "  ./$(TARGET)       - Main GTK face recognition server"
	@echo "  ./$(SOCKET_CLIENT) - Command-line socket client"
	@echo "  ./$(GTK_CLIENT)    - GTK client GUI"

.PHONY: all run debug debug-run clean distclean help
