.PHONY: all clean

CC      ?= zig cc
CFLAGS  ?= -std=c23

BUILD := build

C_SRCS := $(wildcard *.c)
C_BINS := $(patsubst %.c,$(BUILD)/%,$(C_SRCS))

all: $(C_BINS)

clean:
	rm -rf $(BUILD)

$(BUILD)/%: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<
