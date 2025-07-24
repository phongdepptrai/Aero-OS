#ifndef INTERACTIVE_TEST_H
#define INTERACTIVE_TEST_H

// Interactive test function declarations
void run_interactive_tests(void);
void init_interactive_test(void);
void test_interactive_code_typing(void);
void handle_keyboard_input_interactive(void);
void enhanced_irq1(void);

// Utility functions
int str_to_int(const char* str);
void int_to_str(int num, char* str);

#endif // INTERACTIVE_TEST_H
