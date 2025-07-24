#ifndef SAFE_TEST_H
#define SAFE_TEST_H

// Safe test function declarations (no interrupts required)
void run_safe_interactive_tests(void);
void test_simple_input_output(void);

// Utility functions (safe versions to avoid conflicts)
int safe_str_to_int(const char* str);
void safe_int_to_str(int num, char* str);

#endif // SAFE_TEST_H
