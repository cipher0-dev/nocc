.PHONY: all clean

CC      ?= zig cc
CFLAGS  ?= -std=c23

BUILD := build

C_SRCS := $(wildcard *.c)
C_BINS := $(patsubst %.c,$(BUILD)/%,$(C_SRCS))

all: $(C_BINS) $(BUILD)/out

clean:
	rm -rf $(BUILD)

$(BUILD)/%: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD)/main: main.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -static -nostdlib -Wl,-e,_start -o $@ $<

$(BUILD)/main.text: $(BUILD)/main
	objcopy -O binary --only-section=.text $< $@

$(BUILD)/out: $(BUILD)/write_elf $(BUILD)/main.text
	$^ $@

disass_text: $(BUILD)/main.text
	objdump -D -b binary -m i386:x86-64 $<
