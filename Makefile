# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
INCLUDE_DIR = include

# Source and object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Output binary
TARGET = $(BIN_DIR)/fat16_reader

# Default target: Compile everything
all: $(TARGET)

# Compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create the final executable
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Create necessary directories if they don't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean compiled files
clean:
	rm -rf $(BUILD_DIR)/*.o $(BIN_DIR)/fat16_reader

# Run the program
run: all
	$(TARGET) fat16.img

# Rebuild everything from scratch
rebuild: clean all
