# Makefile for Lost Eagle OS - a minimal RISC-V (rv64) kernel for QEMU 'virt'.

CROSS_COMPILE ?= riscv-none-elf-
CC      := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy

# rv64gc, S-mode kernel loaded by OpenSBI. medany is required because the
# kernel is linked high in the address space (0x80200000).
ARCHFLAGS := -march=rv64gc -mabi=lp64d -mcmodel=medany
CFLAGS  := $(ARCHFLAGS) -ffreestanding -nostdlib -nostartfiles -mno-relax \
           -fno-stack-protector -fno-pic -Wall -Wextra -O2 -g -Isrc
LDFLAGS := -Wl,-T,linker.ld -Wl,--build-id=none -Wl,--no-warn-rwx-segments
# libgcc provides compiler runtime helpers (byteswap, 64-bit math, ...).
LDLIBS := -lgcc

SRCDIR := src
BUILD  := build

CSRCS := $(wildcard $(SRCDIR)/*.c)
ASRCS := $(wildcard $(SRCDIR)/*.S)
OBJS  := $(patsubst $(SRCDIR)/%.c,$(BUILD)/%.o,$(CSRCS)) \
         $(patsubst $(SRCDIR)/%.S,$(BUILD)/%.o,$(ASRCS))

KERNEL := $(BUILD)/kernel.elf

QEMU      ?= qemu-system-riscv64
QEMUFLAGS := -machine virt -m 256M -bios default -kernel $(KERNEL) \
             -device ramfb -serial mon:stdio

all: $(KERNEL)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(SRCDIR)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: $(SRCDIR)/%.S | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(OBJS) linker.ld | $(BUILD)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

# Run with a graphical window (ramfb display).
run: $(KERNEL)
	$(QEMU) $(QEMUFLAGS)

# Run headless (no window); useful for CI / serial-only checks.
run-headless: $(KERNEL)
	$(QEMU) -machine virt -m 256M -bios default -kernel $(KERNEL) \
	        -device ramfb -serial stdio -display none

clean:
	rm -rf $(BUILD)

.PHONY: all run run-headless clean
