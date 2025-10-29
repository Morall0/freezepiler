CC = gcc
CFLAGS = -Wall -g -Isrc/main

# linker flags
LDFLAGS = 

SRC_DIR = src/main
BUILD_DIR = build
BIN_DIR = bin
TARGET_NAME = lexer
TARGET = $(BIN_DIR)/$(TARGET_NAME)
SRCS = $(SRC_DIR)/lexer.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "Linking executable: $@"
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling: $<"
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "Cleaning project..."
	rm -f $(BIN_DIR)/*
	rm -f $(BUILD_DIR)/*

.PHONY: all clean
