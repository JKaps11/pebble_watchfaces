CC := cc
CFLAGS := -std=c11 -Wall -Wextra -I../c -Ivendor/unity

BUILD_DIR := build
UNITY := vendor/unity/unity.c

# One binary per suite: Unity gives each test file its own main, so they cannot
# be linked together.
SUITES := datetime_format golf_perspective
BINS := $(addprefix $(BUILD_DIR)/test_,$(SUITES))

.PHONY: test clean

test: $(BINS)
	@for bin in $(BINS); do echo "--- $$bin"; ./$$bin || exit 1; done

$(BUILD_DIR)/test_%: test_%.c ../c/%.c $(UNITY) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
