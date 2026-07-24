CC := cc
CFLAGS := -std=c11 -Wall -Wextra -I../c -Ivendor/unity

BUILD_DIR := build
BIN := $(BUILD_DIR)/run_tests

SRCS := test_datetime_format.c \
        vendor/unity/unity.c \
        ../c/datetime_format.c

.PHONY: test clean

test: $(BIN)
	./$(BIN)

$(BIN): $(SRCS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $(BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
