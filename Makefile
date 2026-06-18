CC := /opt/amiga/bin/m68k-amigaos-gcc
CFLAGS ?= -Os -Wall -Wextra -fomit-frame-pointer -mcrt=nix13 -fno-builtin
LDFLAGS ?= -mcrt=nix13
CPPFLAGS += -I. -Isrc -Iinclude -DAMITCP13_OS13
BUILD_DIR := build
TARGET := $(BUILD_DIR)/ISSTracker
SRCS := src/main.c src/gui.c src/draw.c src/map_image.c src/config.c src/http_client.c src/iss_api.c src/json_min.c src/map_projection.c src/citydb.c src/funfacts.c src/astro.c src/worldmask.c src/fixedmath.c
OBJS := $(SRCS:src/%.c=$(BUILD_DIR)/%.o)
.PHONY: all clean
all: $(TARGET)
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@
clean:
	rm -rf $(BUILD_DIR)
