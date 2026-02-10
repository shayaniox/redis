CC      = gcc
CFLAGS  = -Wall -Wextra -g
LDFLAGS = -Iinclude

BIN_FILE = main
SRC_DIR = src
BUILD_DIR = build
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

$(BIN_FILE): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(BUILD_DIR)/$@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | build
	$(CC) $(CFLAGS) $< -c -o $@ $(LDFLAGS)

build:
	@mkdir -p $(BUILD_DIR)
.PHONY: build

clean:
	rm -rf $(BUILD_DIR)/$(BIN_FILE) $(BUILD_DIR)/*.o
