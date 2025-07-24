# AeroDesk Kernel Testing Guide

## Overview
This document describes how to test the AeroDesk kernel using both automated test modules and interactive testing features.

## Test Module Features

The kernel includes comprehensive testing capabilities:

### 1. **Automated Test Suite** (`kernel_test.c`)
- Serial output functionality
- Basic arithmetic operations  
- String operations and memory access
- Array operations and bounds checking
- Memory allocation (`kmalloc`)
- I/O port operations (`inb`/`outb`)
- Timer functionality and interrupt handling
- Stack operations and pointer arithmetic

### 2. **Interactive Test Module** (`interactive_test.c`)
- **Integer Doubling Test**: Type an integer, get double the value
- Real-time keyboard input processing
- ASCII conversion and string parsing
- Interactive user interface via serial console

## Running Tests

### Method 1: Interactive Testing (Recommended)
```bash
./test_interactive.sh
```
This will:
1. Build the kernel if needed
2. Create bootable ISO
3. Launch QEMU with keyboard support
4. Run automated tests first
5. Start interactive integer doubling test

### Method 2: Quick Automated Test Only
```bash
./test_kernel.sh
```

### Method 3: Manual Testing
```bash
# Build kernel
make clean && make

# Create ISO
make iso

# Run in QEMU with full interface
qemu-system-i386 -cdrom aerodesk.iso -serial stdio
```

## Test Output

The test suite provides detailed output showing:
- Individual test results (PASS/FAIL)
- Test summary with total counts
- Kernel initialization progress
- System component status

### Example Output:
```
==========================================
    AERODESK KERNEL TEST SUITE
==========================================
TEST 1: Serial Output - PASS - Serial output functional
TEST 2: Basic Arithmetic - PASS - Addition works
...
==========================================
Test Summary:
Total tests: 10
Passed: 33
Failed: 0
ALL TESTS PASSED!
==========================================
```

## Test Components

### Test Framework (`test/kernel_test.c`)
- **`test_start()`** - Initialize a new test
- **`test_assert()`** - Assert conditions and report results
- **`test_end()`** - Generate final test summary

### Individual Test Functions:
1. **`test_serial()`** - Serial communication validation
2. **`test_memory_allocation()`** - Dynamic memory allocation
3. **`test_io_ports()`** - Hardware I/O operations
4. **`test_timer()`** - Timer interrupt functionality
5. **`test_arithmetic()`** - Basic math operations
6. **`test_string_operations()`** - String handling
7. **`test_arrays()`** - Array access and manipulation
8. **`test_bit_operations()`** - Bitwise operations
9. **`test_pointers()`** - Pointer arithmetic and dereferencing
10. **`test_stack()`** - Stack operations and local variables

## Adding New Tests

To add new tests:

1. Create a new test function in `test/kernel_test.c`:
```c
void test_new_feature() {
    test_start("New Feature");
    
    // Your test code here
    test_assert(condition, "Test description");
}
```

2. Add the function call to `run_kernel_tests()`:
```c
void run_kernel_tests() {
    // ... existing tests ...
    test_new_feature();
    test_end();
}
```

3. Rebuild the kernel:
```bash
make clean && make
```

## Debugging Failed Tests

If tests fail:

1. **Check Serial Output**: All test output goes to serial console
2. **Review Error Messages**: Failed assertions show specific failure reasons
3. **Use QEMU Monitor**: Add `-monitor stdio` for debugging
4. **Enable Graphics**: Remove `-display none` to see kernel output

## QEMU Options

Common QEMU testing options:
- `-serial stdio` - Redirect serial output to console
- `-display none` - Run headless (no graphics)
- `-monitor stdio` - Enable QEMU monitor
- `-m 512M` - Set memory size
- `-smp 1` - Set number of CPU cores

## Dependencies

Required for testing:
- `qemu-system-i386` - 32-bit x86 emulator
- `grub-mkrescue` - For ISO creation
- `xorriso` - ISO creation dependency
- Standard build tools (`gcc`, `ld`, `as`)

Install on Ubuntu/Debian:
```bash
sudo apt-get install qemu-system-x86 grub-pc-bin grub-common xorriso build-essential
```

## Test Coverage

Current test coverage includes:
- ✅ Basic kernel initialization
- ✅ Memory management (basic allocation)
- ✅ I/O operations
- ✅ Timer functionality
- ✅ Arithmetic and logic operations
- ✅ Data structure operations
- ✅ Hardware interface validation

Future test additions could include:
- [ ] Process management
- [ ] Filesystem operations
- [ ] Network stack
- [ ] Advanced memory management
- [ ] Device driver validation
- [ ] Security and authentication
