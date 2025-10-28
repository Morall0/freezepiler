CC = gcc
CFLAGS = -Wall -g -Isrc/main

# linker flags
LDFLAGS = 

SRC_DIR = src/main
BUILD_DIR = build
TARGET_NAME = lexer
TARGET = $(BUILD_DIR)/$(TARGET_NAME)
SRCS = $(SRC_DIR)/lexer.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking executable: $@"
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling: $<"
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "Cleaning project..."
	rm -f $(BUILD_DIR)/*
	# rm -rf $(BUILD_DIR)

.PHONY: all clean
