# Plain C99, no dependencies. `make test` runs the unit tests; `make test-san`
# rebuilds everything with AddressSanitizer and UndefinedBehaviorSanitizer.
CC ?= cc
CFLAGS ?= -O2 -g
WARN = -std=c99 -Wall -Wextra -Wpedantic -Wconversion -Wno-sign-conversion
ALL_CFLAGS = $(WARN) -fvisibility=hidden -Iinclude $(CFLAGS)

LIB_SRC = $(wildcard src/*.c)
TESTS = $(patsubst tests/%.c,%,$(wildcard tests/test_*.c))
BUILD ?= build

.PHONY: all test test-san clean

all: $(BUILD)/libdtc.a

$(BUILD)/%.o: src/%.c $(wildcard src/*.h) include/texturecodec/texturecodec.h
	@mkdir -p $(BUILD)
	$(CC) $(ALL_CFLAGS) -c $< -o $@

$(BUILD)/libdtc.a: $(patsubst src/%.c,$(BUILD)/%.o,$(LIB_SRC))
	$(AR) rcs $@ $^

$(BUILD)/test_%: tests/test_%.c tests/check.h $(BUILD)/libdtc.a
	$(CC) $(ALL_CFLAGS) -Isrc -Itests $< $(BUILD)/libdtc.a -o $@

test: $(patsubst %,$(BUILD)/%,$(TESTS))
	@set -e; for t in $^; do echo "== $$t"; ./$$t; done

test-san:
	$(MAKE) BUILD=build-san CFLAGS='-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer' test

clean:
	rm -rf build build-san
