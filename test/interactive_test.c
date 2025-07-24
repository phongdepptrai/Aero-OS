#include <stdint.h>
#include <stddef.h>

// Forward declarations of kernel functions
extern void serial_print(const char* str);
extern void serial_write(char c);
extern uint8_t inb(uint16_t port);
extern void outb(uint16_t port, uint8_t val);

// Keyboard input buffer - make it larger for code typing
#define INPUT_BUFFER_SIZE 256
static char input_buffer[INPUT_BUFFER_SIZE];
static int input_pos = 0;
static int input_ready = 0;
static int shift_pressed = 0;

// Simple string to integer conversion
int str_to_int(const char* str) {
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

// Simple integer to string conversion
void int_to_str(int num, char* str) {
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

// Scancode to ASCII mapping for US keyboard layout
static char scancode_to_ascii[256] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,   0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,  'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'','`', 0,   '\\','z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    // Rest initialized to 0
};

// Shifted characters for when shift is pressed
static char scancode_to_ascii_shift[256] = {
    0,   0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,  'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,   '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    // Rest initialized to 0
};

// Handle keyboard input
void handle_keyboard_input_interactive() {
    uint8_t scancode = inb(0x60);
    
    // Handle special keys
    switch (scancode) {
        case 0x2A:  // Left shift pressed
        case 0x36:  // Right shift pressed
            shift_pressed = 1;
            break;
            
        case 0xAA:  // Left shift released
        case 0xB6:  // Right shift released
            shift_pressed = 0;
            break;
            
        case 0x01:  // Escape key
            if (!(scancode & 0x80)) {  // Key press, not release
                serial_print("\nEscape pressed - Exiting interactive mode\n");
                input_ready = 1;  // Exit current input
                return;
            }
            break;
            
        default:
            // Check if this is a key press (not release)
            if (!(scancode & 0x80)) {
                char c;
                if (shift_pressed) {
                    c = scancode_to_ascii_shift[scancode];
                } else {
                    c = scancode_to_ascii[scancode];
                }
                
                if (c != 0) {
                    if (c == '\n') {
                        // Enter pressed - process input
                        serial_write('\n');
                        input_buffer[input_pos] = '\0';
                        input_ready = 1;
                    } else if (scancode == 0x0E) {  // Backspace
                        if (input_pos > 0) {
                            input_pos--;
                            serial_write('\b');
                            serial_write(' ');
                            serial_write('\b');
                        }
                    } else if (input_pos < INPUT_BUFFER_SIZE - 1) {
                        // Accept any printable character
                        input_buffer[input_pos++] = c;
                        serial_write(c);
                    }
                }
            } else {
                // Key release
                uint8_t key = scancode & 0x7F;
                if (key == 0x2A || key == 0x36) {  // Shift release
                    shift_pressed = 0;
                }
            }
            break;
    }
    
    // Send EOI to PIC
    outb(0x20, 0x20);
}

// Enhanced keyboard interrupt handler
void enhanced_irq1() {
    handle_keyboard_input_interactive();
}

// Test function for code typing and analysis
void test_interactive_code_typing() {
    serial_print("\n");
    serial_print("==========================================\n");
    serial_print("      INTERACTIVE CODE TYPING TEST\n");
    serial_print("==========================================\n");
    serial_print("Type C code and press Enter to analyze.\n");
    serial_print("Type 'quit' or 'exit' to quit.\n");
    serial_print("Examples to try:\n");
    serial_print("  int x = 42;\n");
    serial_print("  if (x > 0)\n");
    serial_print("  for (int i = 0; i < 10; i++)\n");
    serial_print("  printf(\"Hello World\");\n");
    serial_print("==========================================\n");
    
    while (1) {
        serial_print("\n> ");
        
        // Reset input state
        input_pos = 0;
        input_ready = 0;
        
        // Use polling instead of interrupts to avoid page faults
        while (!input_ready) {
            // Check if keyboard data is available
            if (inb(0x64) & 0x01) {  // Output buffer full
                uint8_t scancode = inb(0x60);
                
                // Handle special keys
                switch (scancode) {
                    case 0x2A:  // Left shift
                    case 0x36:  // Right shift
                        shift_pressed = 1;
                        continue;
                    case 0xAA:  // Left shift release
                    case 0xB6:  // Right shift release
                        shift_pressed = 0;
                        continue;
                    case 0x01:  // Escape key
                        serial_print("\nEscape pressed - Exiting\n");
                        return;
                    case 0x1C:  // Enter
                        serial_write('\n');
                        input_buffer[input_pos] = '\0';
                        input_ready = 1;
                        break;
                    case 0x0E:  // Backspace
                        if (input_pos > 0) {
                            input_pos--;
                            serial_write('\b');
                            serial_write(' ');
                            serial_write('\b');
                        }
                        break;
                    default:
                        // Handle key presses (not releases)
                        if (!(scancode & 0x80)) {
                            char c = 0;
                            
                            if (shift_pressed) {
                                c = scancode_to_ascii_shift[scancode];
                            } else {
                                c = scancode_to_ascii[scancode];
                            }
                            
                            if (c != 0 && input_pos < INPUT_BUFFER_SIZE - 1) {
                                input_buffer[input_pos++] = c;
                                serial_write(c);
                            }
                        } else {
                            // Key release
                            uint8_t key = scancode & 0x7F;
                            if (key == 0x2A || key == 0x36) {  // Shift release
                                shift_pressed = 0;
                            }
                        }
                        break;
                }
            }
            
            // Small delay to prevent CPU spinning
            for (volatile int i = 0; i < 1000; i++);
        }
        
        // Check if user wants to quit
        if ((input_buffer[0] == 'q' && input_buffer[1] == 'u' && input_buffer[2] == 'i' && input_buffer[3] == 't') ||
            (input_buffer[0] == 'e' && input_buffer[1] == 'x' && input_buffer[2] == 'i' && input_buffer[3] == 't')) {
            serial_print("Exiting interactive code typing test.\n");
            break;
        }
        
        // Analyze the typed code
        serial_print("Code entered: ");
        serial_print(input_buffer);
        serial_print("\n");
        
        // Simple code analysis
        if (input_pos > 0) {
            // Check for common C keywords
            if (input_buffer[0] == 'i' && input_buffer[1] == 'n' && input_buffer[2] == 't') {
                serial_print("Analysis: Integer variable declaration\n");
            } else if (input_buffer[0] == 'i' && input_buffer[1] == 'f') {
                serial_print("Analysis: If statement\n");
            } else if (input_buffer[0] == 'f' && input_buffer[1] == 'o' && input_buffer[2] == 'r') {
                serial_print("Analysis: For loop\n");
            } else if (input_buffer[0] == 'w' && input_buffer[1] == 'h' && input_buffer[2] == 'i' && input_buffer[3] == 'l' && input_buffer[4] == 'e') {
                serial_print("Analysis: While loop\n");
            } else if (input_buffer[0] == '#') {
                serial_print("Analysis: Preprocessor directive\n");
            } else if (input_buffer[0] == 'p' && input_buffer[1] == 'r' && input_buffer[2] == 'i' && input_buffer[3] == 'n' && input_buffer[4] == 't' && input_buffer[5] == 'f') {
                serial_print("Analysis: Printf function call\n");
            } else if (input_buffer[0] >= '0' && input_buffer[0] <= '9') {
                // Try to parse as number and double it
                int num = str_to_int(input_buffer);
                int doubled = num * 2;
                char result_str[16];
                int_to_str(doubled, result_str);
                serial_print("Analysis: Number detected - ");
                serial_print(input_buffer);
                serial_print(" doubled is ");
                serial_print(result_str);
                serial_print("\n");
            } else {
                serial_print("Analysis: Code statement\n");
            }
        }
    }
    
    serial_print("==========================================\n");
    serial_print("Interactive code typing test completed.\n");
    serial_print("==========================================\n");
}

// Initialize the interactive test module
void init_interactive_test() {
    serial_print("Initializing interactive test module...\n");
    
    // Clear input buffer
    for (int i = 0; i < INPUT_BUFFER_SIZE; i++) {
        input_buffer[i] = 0;
    }
    input_pos = 0;
    input_ready = 0;
    
    serial_print("Interactive test module ready!\n");
}

// Run all interactive tests
void run_interactive_tests() {
    init_interactive_test();
    test_interactive_code_typing();
}
