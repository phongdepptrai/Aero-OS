#include <stdint.h>

// UART port definitions
#define UART_PORT 0x3F8   // COM1 base address

// UART register offsets
#define UART_DATA       0  // Data register
#define UART_INT_EN     1  // Interrupt enable
#define UART_INT_ID     2  // Interrupt identification
#define UART_LINE_CTL   3  // Line control
#define UART_MODEM_CTL  4  // Modem control
#define UART_LINE_STAT  5  // Line status
#define UART_MODEM_STAT 6  // Modem status
#define UART_SCRATCH    7  // Scratch register

// Basic I/O functions
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Initialize UART: 115200 baud, 8-N-1
int uart_init() {
    outb(UART_PORT + UART_INT_EN, 0x00);    // Disable all interrupts
    outb(UART_PORT + UART_LINE_CTL, 0x80);  // Enable DLAB (set baud rate divisor)
    outb(UART_PORT + UART_DATA, 0x01);      // Set divisor to 1 (lo byte) 115200 baud
    outb(UART_PORT + UART_INT_EN, 0x00);    //                  (hi byte)
    outb(UART_PORT + UART_LINE_CTL, 0x03);  // 8 bits, no parity, one stop bit
    outb(UART_PORT + UART_INT_ID, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(UART_PORT + UART_MODEM_CTL, 0x0B); // IRQs enabled, RTS/DSR set
    return 0;
}

int uart_write(char c) {
    while ((inb(UART_PORT + UART_LINE_STAT) & 0x20) == 0); // Wait for TX ready
    outb(UART_PORT + UART_DATA, c);
    return 0;
}

int uart_read(char *buf, int len) {
    for (int i = 0; i < len; i++) {
        while ((inb(UART_PORT + UART_LINE_STAT) & 0x01) == 0); // Wait for data ready
        buf[i] = inb(UART_PORT + UART_DATA);
    }
    return len;
}

void uart_print(const char* str) {
    while (*str) {
        uart_write(*str++);
    }
}

// Check if data is available for reading
int uart_data_available() {
    return inb(UART_PORT + UART_LINE_STAT) & 0x01;
}

// Non-blocking read
int uart_read_nonblocking(char* c) {
    if (uart_data_available()) {
        *c = inb(UART_PORT + UART_DATA);
        return 1;
    }
    return 0;
}
