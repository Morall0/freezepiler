CC = gcc
CFLAGS = -Wall -g -Isrc/main

# linker flags
LDFLAGS = 

SRC_DIR = src/main
BUILD_DIR = build
BIN_DIR = bin
TARGET_NAME = main
TARGET = $(BIN_DIR)/$(TARGET_NAME)
SRCS = $(SRC_DIR)/main.c \
	   $(SRC_DIR)/lexer.c \
	   $(SRC_DIR)/parser.tab.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

PARSER_Y = $(SRC_DIR)/parser.y
PARSER_C = $(SRC_DIR)/parser.tab.c
PARSER_H = $(SRC_DIR)/parser.tab.h

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "Linking executable: $@"
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(PARSER_H)
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling: $<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(PARSER_C) $(PARSER_H): $(PARSER_Y)
	@echo "Running Bison on $<"
	bison -d -o $(PARSER_C) $(PARSER_Y)

clean:
	@echo "Cleaning project..."
	rm -f $(BIN_DIR)/*
	rm -f $(BUILD_DIR)/*

.PHONY: all clean
