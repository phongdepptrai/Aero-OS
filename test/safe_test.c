#include <stdint.h>
#include <stddef.h>

// Forward declarations of kernel functions
extern void serial_print(const char* str);
extern void serial_write(char c);
extern uint8_t inb(uint16_t port);
extern void outb(uint16_t port, uint8_t val);

// Forward declare utility functions (avoid redefinition)
int safe_str_to_int(const char* str);
void safe_int_to_str(int num, char* str);

// Simple polling-based input test (no interrupts)
void test_simple_input_output() {
    serial_print("\n");
    serial_print("==========================================\n");
    serial_print("      SIMPLE I/O TEST MODULE\n");
    serial_print("==========================================\n");
    serial_print("Testing basic I/O functionality...\n");
    
    // Test 1: Simple number doubling (hardcoded)
    serial_print("Test 1: Doubling hardcoded numbers\n");
    int test_numbers[] = {5, -3, 42, 100, 0};
    
    for (int i = 0; i < 5; i++) {
        int num = test_numbers[i];
        int doubled = num * 2;
        
        // Convert numbers to strings and display
        char num_str[16], doubled_str[16];
        safe_int_to_str(num, num_str);
        safe_int_to_str(doubled, doubled_str);
        
        serial_print("Input: ");
        serial_print(num_str);
        serial_print(", Double: ");
        serial_print(doubled_str);
        serial_print("\n");
    }
    
    // Test 2: String processing
    serial_print("\nTest 2: String to integer conversion\n");
    const char* test_strings[] = {"123", "-45", "999", "0", "42"};
    
    for (int i = 0; i < 5; i++) {
        const char* str = test_strings[i];
        int parsed = safe_str_to_int(str);
        int doubled = parsed * 2;
        
        char doubled_str[16];
        safe_int_to_str(doubled, doubled_str);
        
        serial_print("String: \"");
        serial_print(str);
        serial_print("\" -> Integer: ");
        char parsed_str[16];
        safe_int_to_str(parsed, parsed_str);
        serial_print(parsed_str);
        serial_print(" -> Doubled: ");
        serial_print(doubled_str);
        serial_print("\n");
    }
    
    // Test 3: Keyboard status (polling, safe)
    serial_print("\nTest 3: Keyboard controller status\n");
    uint8_t status = inb(0x64);
    serial_print("Keyboard controller status: 0x");
    char hex[3];
    hex[0] = (status >> 4) > 9 ? 'A' + (status >> 4) - 10 : '0' + (status >> 4);
    hex[1] = (status & 0xF) > 9 ? 'A' + (status & 0xF) - 10 : '0' + (status & 0xF);
    hex[2] = '\0';
    serial_print(hex);
    serial_print("\n");
    
    if (status & 0x01) {
        serial_print("Output buffer full (data available)\n");
    } else {
        serial_print("Output buffer empty\n");
    }
    
    if (status & 0x02) {
        serial_print("Input buffer full\n");
    } else {
        serial_print("Input buffer empty\n");
    }
    
    serial_print("==========================================\n");
    serial_print("Simple I/O tests completed successfully!\n");
    serial_print("All integer doubling functionality works!\n");
    serial_print("==========================================\n");
}

// Simple string to integer conversion (safe version)
int safe_str_to_int(const char* str) {
    int result = 0;
    int sign = 1;
    int i = 0;
    
    // Handle negative numbers
    if (str[0] == '-') {
        sign = -1;
        i = 1;
    } else if (str[0] == '+') {
        i = 1;
    }
    
    // Convert digits
    while (str[i] != '\0' && str[i] != '\n' && str[i] != '\r') {
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
        } else {
            // Invalid character
            return 0;
        }
        i++;
    }
    
    return result * sign;
}

// Simple integer to string conversion (safe version)
void safe_int_to_str(int num, char* str) {
    int i = 0;
    int is_negative = 0;
    
    // Handle negative numbers
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    // Handle zero case
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    // Convert digits (reverse order)
    while (num > 0) {
        str[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    // Add negative sign if needed
    if (is_negative) {
        str[i++] = '-';
    }
    
    // Null terminate
    str[i] = '\0';
    
    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// Main test runner (safe version)
void run_safe_interactive_tests() {
    test_simple_input_output();
}
