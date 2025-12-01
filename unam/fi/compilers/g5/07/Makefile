CC = gcc
# LLVM flags extraídos automáticamente
LLVM_CFLAGS := $(shell llvm-config --cflags)
LLVM_LDFLAGS := $(shell llvm-config --ldflags --libs core mcjit native --system-libs)
CFLAGS = -Wall -g -Isrc/main $(LLVM_CFLAGS)

# Linker flags
LDFLAGS = $(LLVM_LDFLAGS)

# Vars
SRC_DIR = src/main
BUILD_DIR = build
BIN_DIR = bin
TARGET_NAME = main
TARGET = $(BIN_DIR)/$(TARGET_NAME)
SRCS = $(SRC_DIR)/main.c \
		$(SRC_DIR)/lexer.c \
		$(SRC_DIR)/ast.c \
		$(SRC_DIR)/parser.tab.c \
		$(SRC_DIR)/codegen.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Bison files
PARSER_Y = $(SRC_DIR)/parser.y
PARSER_C = $(SRC_DIR)/parser.tab.c
PARSER_H = $(SRC_DIR)/parser.tab.h

# Headers
HDRS = $(SRC_DIR)/ast.h $(SRC_DIR)/lexer.h $(SRC_DIR)/codegen.h

all: $(TARGET)

# Linking rule
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "Linking executable: $@"
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

# Compilation rule
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(PARSER_H) $(HDRS)
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling: $<"
	$(CC) $(CFLAGS) -c -o $@ $<

# Bison files generation
$(PARSER_C) $(PARSER_H): $(PARSER_Y) $(SRC_DIR)/ast.h
	@echo "Running Bison on $<"
	bison -d -o $(PARSER_C) $(PARSER_Y)

clean:
	@echo "Cleaning project..."
	rm -f $(BIN_DIR)/*
	rm -f $(BUILD_DIR)/*
	rm -f $(PARSER_C)
	rm -f $(PARSER_H)

.PHONY: all clean
