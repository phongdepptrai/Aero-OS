#ifndef KERNEL_TEST_H
#define KERNEL_TEST_H

// Test function declarations
void run_kernel_tests(void);

// Individual test functions
void test_serial(void);
void test_memory_allocation(void);
void test_io_ports(void);
void test_timer(void);
void test_arithmetic(void);
void test_string_operations(void);
void test_arrays(void);
void test_bit_operations(void);
void test_pointers(void);
void test_stack(void);

// Test framework functions
void test_start(const char* test_name);
void test_assert(int condition, const char* message);
void test_end(void);

#endif // KERNEL_TEST_H
