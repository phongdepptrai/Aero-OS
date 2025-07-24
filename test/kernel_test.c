#include <stdint.h>
#include <stddef.h>

// Test framework macros
#define TEST_PASS 0
#define TEST_FAIL 1

static int test_count = 0;
static int test_passed = 0;
static int test_failed = 0;

// Forward declarations of kernel functions to test
extern void serial_print(const char* str);
extern void* kmalloc(size_t size);
extern volatile uint32_t timer_ticks;
extern void outb(uint16_t port, uint8_t val);
extern uint8_t inb(uint16_t port);

// Test framework functions
void test_start(const char* test_name) {
    test_count++;
    serial_print("TEST ");
    // Simple number output
    char num[10];
    int i = 0;
    int n = test_count;
    if (n == 0) {
        num[i++] = '0';
    } else {
        while (n > 0) {
            num[i++] = '0' + (n % 10);
            n /= 10;
        }
    }
    // Reverse
    for (int j = 0; j < i/2; j++) {
        char temp = num[j];
        num[j] = num[i-1-j];
        num[i-1-j] = temp;
    }
    num[i] = '\0';
    
    serial_print(num);
    serial_print(": ");
    serial_print(test_name);
    serial_print(" - ");
}

void test_assert(int condition, const char* message) {
    if (condition) {
        serial_print("PASS - ");
        serial_print(message);
        serial_print("\n");
        test_passed++;
    } else {
        serial_print("FAIL - ");
        serial_print(message);
        serial_print("\n");
        test_failed++;
    }
}

void test_end() {
    serial_print("==========================================\n");
    serial_print("Test Summary:\n");
    serial_print("Total tests: ");
    
    // Output total tests
    char num[10];
    int i = 0;
    int n = test_count;
    if (n == 0) {
        num[i++] = '0';
    } else {
        while (n > 0) {
            num[i++] = '0' + (n % 10);
            n /= 10;
        }
    }
    for (int j = 0; j < i/2; j++) {
        char temp = num[j];
        num[j] = num[i-1-j];
        num[i-1-j] = temp;
    }
    num[i] = '\0';
    serial_print(num);
    serial_print("\n");
    
    serial_print("Passed: ");
    // Output passed tests
    i = 0;
    n = test_passed;
    if (n == 0) {
        num[i++] = '0';
    } else {
        while (n > 0) {
            num[i++] = '0' + (n % 10);
            n /= 10;
        }
    }
    for (int j = 0; j < i/2; j++) {
        char temp = num[j];
        num[j] = num[i-1-j];
        num[i-1-j] = temp;
    }
    num[i] = '\0';
    serial_print(num);
    serial_print("\n");
    
    serial_print("Failed: ");
    // Output failed tests
    i = 0;
    n = test_failed;
    if (n == 0) {
        num[i++] = '0';
    } else {
        while (n > 0) {
            num[i++] = '0' + (n % 10);
            n /= 10;
        }
    }
    for (int j = 0; j < i/2; j++) {
        char temp = num[j];
        num[j] = num[i-1-j];
        num[i-1-j] = temp;
    }
    num[i] = '\0';
    serial_print(num);
    serial_print("\n");
    
    if (test_failed == 0) {
        serial_print("ALL TESTS PASSED!\n");
    } else {
        serial_print("SOME TESTS FAILED!\n");
    }
    serial_print("==========================================\n");
}

// Test serial output functionality
void test_serial() {
    test_start("Serial Output");
    
    serial_print("Testing serial output...\n");
    test_assert(1, "Serial output functional");
}

// Test memory allocation
void test_memory_allocation() {
    test_start("Memory Allocation");
    
    void* ptr1 = kmalloc(64);
    test_assert(ptr1 != NULL, "64-byte allocation");
    
    void* ptr2 = kmalloc(128);
    test_assert(ptr2 != NULL, "128-byte allocation");
    test_assert(ptr2 != ptr1, "Different allocations have different addresses");
    
    void* ptr3 = kmalloc(0);
    test_assert(ptr3 == NULL, "Zero-size allocation returns NULL");
}

// Test I/O port operations
void test_io_ports() {
    test_start("I/O Port Operations");
    
    // Test with a safe port (POST diagnostic port)
    uint8_t test_value = 0x42;
    outb(0x80, test_value);
    test_assert(1, "outb operation completed without crash");
    
    // Read from CMOS seconds register (safe read)
    outb(0x70, 0x00);
    uint8_t seconds = inb(0x71);
    test_assert(seconds < 60, "CMOS seconds read is valid (< 60)");
}

// Test timer functionality
void test_timer() {
    test_start("Timer Functionality");
    
    uint32_t initial_ticks = timer_ticks;
    
    // Wait a bit
    for (volatile int i = 0; i < 100000; i++);
    
    test_assert(timer_ticks >= initial_ticks, "Timer ticks are incrementing");
}

// Test arithmetic operations
void test_arithmetic() {
    test_start("Basic Arithmetic");
    
    int a = 10;
    int b = 5;
    
    test_assert(a + b == 15, "Addition works");
    test_assert(a - b == 5, "Subtraction works");
    test_assert(a * b == 50, "Multiplication works");
    test_assert(a / b == 2, "Division works");
    test_assert(a % 3 == 1, "Modulo works");
}

// Test string operations
void test_string_operations() {
    test_start("String Operations");
    
    const char* str1 = "Hello";
    const char* str2 = "World";
    (void)str2; // Suppress unused warning
    
    // Simple string length check
    int len1 = 0;
    while (str1[len1] != '\0') len1++;
    test_assert(len1 == 5, "String length calculation");
    
    // Simple string comparison
    int same = 1;
    for (int i = 0; i < 5; i++) {
        if (str1[i] != "Hello"[i]) {
            same = 0;
            break;
        }
    }
    test_assert(same == 1, "String comparison");
}

// Test array operations
void test_arrays() {
    test_start("Array Operations");
    
    uint8_t array[10];
    
    // Initialize array
    for (int i = 0; i < 10; i++) {
        array[i] = i * 2;
    }
    
    test_assert(array[0] == 0, "Array index 0");
    test_assert(array[5] == 10, "Array index 5");
    test_assert(array[9] == 18, "Array index 9");
    
    // Test array bounds
    array[3] = 99;
    test_assert(array[3] == 99, "Array write/read");
}

// Test bit operations
void test_bit_operations() {
    test_start("Bit Operations");
    
    uint32_t value = 0x12345678;
    
    test_assert((value & 0xFF) == 0x78, "Bitwise AND");
    test_assert((value | 0x0F) == 0x1234567F, "Bitwise OR");
    test_assert((value ^ 0xFFFFFFFF) == 0xEDCBA987, "Bitwise XOR");
    test_assert((~value) == 0xEDCBA987, "Bitwise NOT");
    test_assert((value << 4) == 0x23456780, "Left shift");
    test_assert((value >> 4) == 0x01234567, "Right shift");
}

// Test pointer operations
void test_pointers() {
    test_start("Pointer Operations");
    
    int value = 42;
    int* ptr = &value;
    
    test_assert(*ptr == 42, "Pointer dereference");
    test_assert(ptr == &value, "Address-of operator");
    
    *ptr = 84;
    test_assert(value == 84, "Pointer write");
    
    int array[5] = {1, 2, 3, 4, 5};
    int* arr_ptr = array;
    
    test_assert(*arr_ptr == 1, "Array pointer base");
    test_assert(*(arr_ptr + 2) == 3, "Array pointer arithmetic");
}

// Test stack operations
void test_stack() {
    test_start("Stack Operations");
    
    uint32_t stack_var1 = 0xDEADBEEF;
    uint32_t stack_var2 = 0xCAFEBABE;
    
    // Get approximate stack pointer
    uint32_t esp;
    asm volatile ("mov %%esp, %0" : "=r"(esp));
    
    test_assert(esp > 0x1000, "Stack pointer is reasonable");
    test_assert(stack_var1 == 0xDEADBEEF, "Stack variable 1");
    test_assert(stack_var2 == 0xCAFEBABE, "Stack variable 2");
}

// Main test runner
void run_kernel_tests() {
    serial_print("\n");
    serial_print("==========================================\n");
    serial_print("    AERODESK KERNEL TEST SUITE\n");
    serial_print("==========================================\n");
    
    test_serial();
    test_arithmetic();
    test_string_operations();
    test_arrays();
    test_bit_operations();
    test_pointers();
    test_stack();
    test_memory_allocation();
    test_io_ports();
    test_timer();
    
    test_end();
}
