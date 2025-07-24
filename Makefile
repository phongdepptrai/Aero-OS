# AeroDesk OS Makefile

# Compiler and tools
CC = gcc
AS = as
LD = ld
AR = ar

# Directories
SRC_DIR = src
KERNEL_DIR = $(SRC_DIR)/kernel
SAL_DIR = $(SRC_DIR)/sal
DRIVERS_DIR = $(SRC_DIR)/drivers
SERVICES_DIR = $(SRC_DIR)/services
TEST_DIR = test
INCLUDE_DIR = include
BUILD_DIR = build
ISO_DIR = isodir

# Compiler flags
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -nostdlib -fno-pic -I$(INCLUDE_DIR)
ASFLAGS = --32

# Linker flags
LDFLAGS = -m elf_i386 -T $(KERNEL_DIR)/linker.ld -nostdlib

# Source files
KERNEL_ASM = $(KERNEL_DIR)/boot.S
KERNEL_SRC = $(KERNEL_DIR)/kernel.c
SAL_SRC = $(SAL_DIR)/sal.c
DRIVER_SRCS = $(wildcard $(DRIVERS_DIR)/*.c)
SERVICE_SRCS = $(wildcard $(SERVICES_DIR)/*.c)

# Object files
KERNEL_ASM_OBJ = $(BUILD_DIR)/boot.o
KERNEL_OBJ = $(BUILD_DIR)/kernel.o
SAL_OBJ = $(BUILD_DIR)/sal.o
DRIVER_OBJS = $(patsubst $(DRIVERS_DIR)/%.c,$(BUILD_DIR)/%.o,$(DRIVER_SRCS))
SERVICE_OBJS = $(patsubst $(SERVICES_DIR)/%.c,$(BUILD_DIR)/%.o,$(SERVICE_SRCS))

ALL_OBJS = $(KERNEL_ASM_OBJ) $(KERNEL_OBJ) $(SAL_OBJ) $(DRIVER_OBJS) $(SERVICE_OBJS)

# Target files
KERNEL = aerodesk_kernel.elf
SAL_LIB = libsal.a
ISO = aerodesk.iso

# Default target
all: $(KERNEL) $(SAL_LIB)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build kernel
$(KERNEL): $(BUILD_DIR) $(ALL_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(ALL_OBJS)

# Build SAL library
$(SAL_LIB): $(BUILD_DIR) $(SAL_OBJ)
	$(AR) rcs $@ $(SAL_OBJ)

# Compile assembly files
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.S | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

# Compile kernel C files
$(BUILD_DIR)/kernel.o: $(KERNEL_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile SAL C files
$(BUILD_DIR)/sal.o: $(SAL_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile driver C files
$(BUILD_DIR)/%.o: $(DRIVERS_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile service C files
$(BUILD_DIR)/%.o: $(SERVICES_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Create bootable ISO
iso: $(KERNEL)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/
	cp boot/grub/grub.cfg $(ISO_DIR)/boot/grub/
	grub-mkrescue -o $(ISO) $(ISO_DIR)

# Test with QEMU
qemu: $(KERNEL)
	qemu-system-x86_64 -kernel $(KERNEL) -serial stdio

# Test with ISO
qemu-iso: iso
	qemu-system-x86_64 -cdrom $(ISO) -serial stdio

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(KERNEL) $(SAL_LIB) $(ISO_DIR) $(ISO)

# Install dependencies (Ubuntu/Debian)
deps:
	sudo apt-get update
	sudo apt-get install build-essential nasm qemu-system-x86 grub-pc-bin grub-common xorriso

# Show project structure
tree:
	find . -type f -name "*.c" -o -name "*.h" -o -name "*.S" -o -name "*.ld" -o -name "Makefile" -o -name "*.md" -o -name "*.cfg" | grep -v build | sort

.PHONY: all iso qemu qemu-iso clean deps tree
