#include "code_editor_test.h"
#include <stdint.h>

// Keyboard scancode to ASCII mapping for US keyboard layout
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

// External functions from kernel
extern void outb(uint16_t port, uint8_t val);
extern uint8_t inb(uint16_t port);
extern void serial_print(const char* str);
extern void serial_write(char c);

// Global state for editor
static char code_buffer[4096];  // Buffer to store typed code
static int buffer_pos = 0;      // Current position in buffer
static int shift_pressed = 0;   // Track shift key state
static int ctrl_pressed = 0;    // Track ctrl key state

// Display the current line being typed
void display_current_line() {
    serial_print("\n> ");
    
    // Display current buffer content
    for (int i = 0; i < buffer_pos; i++) {
        serial_write(code_buffer[i]);
    }
}

// Process a character and add to buffer
void process_character(char c) {
    if (c == 0) return;  // Ignore null characters
    
    if (c == '\n') {
        // Enter pressed - execute/display the line
        code_buffer[buffer_pos] = '\0';  // Null terminate
        
        serial_print("\nCode entered: ");
        serial_print(code_buffer);
        serial_print("\n");
        
        // Simple code analysis
        if (buffer_pos > 0) {
            // Check for common C keywords
            if (code_buffer[0] == 'i' && code_buffer[1] == 'n' && code_buffer[2] == 't') {
                serial_print("Detected: Integer variable declaration\n");
            } else if (code_buffer[0] == 'i' && code_buffer[1] == 'f') {
                serial_print("Detected: If statement\n");
            } else if (code_buffer[0] == 'f' && code_buffer[1] == 'o' && code_buffer[2] == 'r') {
                serial_print("Detected: For loop\n");
            } else if (code_buffer[0] == 'w' && code_buffer[1] == 'h' && code_buffer[2] == 'i' && code_buffer[3] == 'l' && code_buffer[4] == 'e') {
                serial_print("Detected: While loop\n");
            } else if (code_buffer[0] == '#') {
                serial_print("Detected: Preprocessor directive\n");
            } else {
                serial_print("Detected: Code statement\n");
            }
        }
        
        // Reset buffer for next line
        buffer_pos = 0;
        display_current_line();
        
    } else if (c == '\b' || c == 127) {  // Backspace
        if (buffer_pos > 0) {
            buffer_pos--;
            serial_print("\b \b");  // Erase character on screen
        }
        
    } else if (buffer_pos < sizeof(code_buffer) - 1) {
        // Add character to buffer
        code_buffer[buffer_pos++] = c;
        serial_write(c);  // Echo character
    }
}

// Handle keyboard scancode
void handle_keyboard_input() {
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
            
        case 0x1D:  // Ctrl pressed
            ctrl_pressed = 1;
            break;
            
        case 0x9D:  // Ctrl released
            ctrl_pressed = 0;
            break;
            
        case 0x01:  // Escape key
            if (!(scancode & 0x80)) {  // Key press, not release
                serial_print("\nEscape pressed - Exiting code editor\n");
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
                
                // Handle Ctrl combinations
                if (ctrl_pressed) {
                    switch (c) {
                        case 'c':
                            serial_print("\n^C - Interrupt\n");
                            buffer_pos = 0;
                            display_current_line();
                            break;
                        case 'l':
                            serial_print("\n^L - Clear screen\n");
                            // Could clear screen here
                            display_current_line();
                            break;
                        default:
                            // Ignore other ctrl combinations for now
                            break;
                    }
                } else {
                    process_character(c);
                }
            }
            break;
    }
    
    // Send EOI to interrupt controller
    outb(0x20, 0x20);
}

// Enhanced keyboard interrupt handler that calls our input handler
void enhanced_keyboard_interrupt() {
    handle_keyboard_input();
}

// Set up enhanced keyboard handling
void setup_enhanced_keyboard() {
    serial_print("Setting up enhanced keyboard input...\n");
    
    // We'll need to update the IDT entry for keyboard interrupt
    // This would normally be done by modifying the kernel's IRQ1 handler
    
    // For now, we'll use polling mode to avoid the page fault issues
    serial_print("Using polling mode for keyboard input\n");
}

void run_code_editor_test() {
    serial_print("\n");
    serial_print("=========================================\n");
    serial_print("     INTERACTIVE CODE EDITOR TEST       \n");
    serial_print("=========================================\n");
    serial_print("Type C code and press Enter to analyze\n");
    serial_print("Use Escape to exit\n");
    serial_print("Examples to try:\n");
    serial_print("  int x = 5;\n");
    serial_print("  if (x > 0)\n");
    serial_print("  for (int i = 0; i < 10; i++)\n");
    serial_print("  #include <stdio.h>\n");
    serial_print("=========================================\n");
    
    // Initialize buffer
    buffer_pos = 0;
    shift_pressed = 0;
    ctrl_pressed = 0;
    
    setup_enhanced_keyboard();
    display_current_line();
    
    // Main input loop using polling
    serial_print("\nPolling for keyboard input (type and press Enter)...\n");
    
    // Simple polling loop for demonstration
    int timeout_counter = 0;
    while (timeout_counter < 1000000) {
        // Check if keyboard data is available
        if (inb(0x64) & 0x01) {  // Output buffer full
            uint8_t scancode = inb(0x60);
            
            // Process the scancode similar to interrupt handler
            if (scancode == 0x01) {  // Escape key
                serial_print("\nEscape pressed - Exiting code editor\n");
                break;
            }
            
            // Handle key presses (not releases)
            if (!(scancode & 0x80)) {
                char c = 0;
                
                // Handle special keys
                switch (scancode) {
                    case 0x2A:  // Left shift
                    case 0x36:  // Right shift
                        shift_pressed = 1;
                        continue;
                    case 0x1C:  // Enter
                        c = '\n';
                        break;
                    case 0x0E:  // Backspace
                        c = '\b';
                        break;
                    case 0x39:  // Space
                        c = ' ';
                        break;
                    default:
                        if (shift_pressed) {
                            c = scancode_to_ascii_shift[scancode];
                        } else {
                            c = scancode_to_ascii[scancode];
                        }
                        break;
                }
                
                if (c != 0) {
                    process_character(c);
                }
            } else {
                // Key release
                uint8_t key = scancode & 0x7F;
                if (key == 0x2A || key == 0x36) {  // Shift release
                    shift_pressed = 0;
                }
            }
            
            timeout_counter = 0;  // Reset timeout when we get input
        } else {
            timeout_counter++;
            // Small delay to prevent CPU spinning
            for (volatile int i = 0; i < 1000; i++);
        }
    }
    
    serial_print("\nCode editor test completed\n");
    serial_print("Buffer contents: ");
    code_buffer[buffer_pos] = '\0';
    serial_print(code_buffer);
    serial_print("\n");
}
