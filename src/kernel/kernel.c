#include <stdint.h>
#include <stddef.h>
#include "../include/sal/sal.h"
#include "../include/auth.h"

// Basic I/O functions - make them non-static for testing
void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Serial port initialization for early debug output
void init_serial() {
    outb(0x3F8 + 1, 0x00);    // Disable all interrupts
    outb(0x3F8 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(0x3F8 + 0, 0x01);    // Set divisor to 1 (lo byte) 115200 baud
    outb(0x3F8 + 1, 0x00);    //                  (hi byte)
    outb(0x3F8 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(0x3F8 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(0x3F8 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

void serial_write(char c) {
    while ((inb(0x3F8 + 5) & 0x20) == 0);
    outb(0x3F8, c);
}

void serial_print(const char* str) {
    while (*str) {
        serial_write(*str++);
    }
}

// Basic GDT setup
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct gdt_entry gdt[3];
static struct gdt_ptr gdt_ptr;

void init_gdt() {
    // Null descriptor
    gdt[0].limit_low = 0;
    gdt[0].base_low = 0;
    gdt[0].base_middle = 0;
    gdt[0].access = 0;
    gdt[0].granularity = 0;
    gdt[0].base_high = 0;
    
    // Kernel code segment
    gdt[1].limit_low = 0xFFFF;
    gdt[1].base_low = 0;
    gdt[1].base_middle = 0;
    gdt[1].access = 0x9A;
    gdt[1].granularity = 0xCF;  // 32-bit, 4KB granularity
    gdt[1].base_high = 0;
    
    // Kernel data segment
    gdt[2].limit_low = 0xFFFF;
    gdt[2].base_low = 0;
    gdt[2].base_middle = 0;
    gdt[2].access = 0x92;
    gdt[2].granularity = 0xCF;  // 32-bit, 4KB granularity
    gdt[2].base_high = 0;
    
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint32_t)&gdt;
    
    asm volatile ("lgdt %0" :: "m"(gdt_ptr));
}

// IDT setup structures
struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr idt_ptr;

// Interrupt handler declarations
extern void isr0();
extern void isr1();
extern void isr8();
extern void isr13();
extern void isr14();
extern void irq0();
extern void irq1();

// Timer variables
volatile uint32_t timer_ticks = 0;

// Forward declaration
void timer_handler();

// Timer interrupt handler
void timer_handler() {
    timer_ticks++;
    // Send EOI to PIC
    outb(0x20, 0x20);
}

// Interrupt handler implementations (basic stubs)
void isr0() {
    serial_print("Division by zero exception!\n");
    while(1) asm volatile ("hlt");
}

void isr1() {
    serial_print("Debug exception!\n");
    while(1) asm volatile ("hlt");
}

void isr8() {
    serial_print("Double fault exception!\n");
    while(1) asm volatile ("hlt");
}

void isr13() {
    serial_print("General protection fault!\n");
    while(1) asm volatile ("hlt");
}

void isr14() {
    uint32_t fault_addr;
    asm volatile ("mov %%cr2, %0" : "=r"(fault_addr));
    
    serial_print("Page fault exception!\n");
    serial_print("Fault address: 0x");
    
    // Print fault address in hex
    char hex[9];
    for (int i = 7; i >= 0; i--) {
        int digit = (fault_addr >> (i * 4)) & 0xF;
        hex[7-i] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
    }
    hex[8] = '\0';
    serial_print(hex);
    serial_print("\n");
    
    // Get error code from stack
    uint32_t error_code;
    asm volatile ("mov 4(%%esp), %0" : "=r"(error_code));
    
    serial_print("Error code: 0x");
    for (int i = 7; i >= 0; i--) {
        int digit = (error_code >> (i * 4)) & 0xF;
        hex[7-i] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
    }
    hex[8] = '\0';
    serial_print(hex);
    serial_print("\n");
    
    if (error_code & 1) {
        serial_print("Page protection violation\n");
    } else {
        serial_print("Page not present\n");
    }
    
    if (error_code & 2) {
        serial_print("Write operation\n");
    } else {
        serial_print("Read operation\n");
    }
    
    if (error_code & 4) {
        serial_print("User mode access\n");
    } else {
        serial_print("Kernel mode access\n");
    }
    
    while(1) asm volatile ("hlt");
}

void irq0() {
    timer_handler();
}

void irq1() {
    serial_print("Keyboard interrupt\n");
    uint8_t scancode = inb(0x60); // Read scancode
    (void)scancode; // Suppress unused warning
    outb(0x20, 0x20); // Send EOI
}

// Set an IDT entry
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].offset_low = base & 0xFFFF;
    idt[num].offset_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
}

void init_idt() {
    serial_print("IDT initialization...\n");
    
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint32_t)&idt;
    
    // Clear IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    // Set up exception handlers (0-31)
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);   // Division by zero
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);   // Debug exception
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);   // Double fault
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E); // General protection fault
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E); // Page fault
    
    // Set up IRQ handlers (32-47)
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);  // Timer
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);  // Keyboard
    
    // Load IDT
    asm volatile ("lidt %0" :: "m"(idt_ptr));
    serial_print("IDT loaded\n");
}

// Paging structures
#define PAGE_SIZE 4096
#define PAGE_TABLE_ENTRIES 1024
#define PAGE_DIRECTORY_ENTRIES 1024

static uint32_t page_directory[PAGE_DIRECTORY_ENTRIES] __attribute__((aligned(4096)));
static uint32_t first_page_table[PAGE_TABLE_ENTRIES] __attribute__((aligned(4096)));

void enable_paging() {
    serial_print("Paging setup...\n");
    
    // Clear page directory
    for (int i = 0; i < PAGE_DIRECTORY_ENTRIES; i++) {
        page_directory[i] = 0x00000002; // Not present, writable, supervisor
    }
    
    // Map first 8MB of memory (identity mapping) - more generous mapping
    for (int i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        first_page_table[i] = (i * PAGE_SIZE) | 3; // Present, writable
    }
    
    // Set first page directory entry to point to our page table
    page_directory[0] = ((uint32_t)first_page_table) | 3; // Present, writable
    
    // Map additional page table for higher memory if needed
    static uint32_t second_page_table[PAGE_TABLE_ENTRIES] __attribute__((aligned(4096)));
    for (int i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        second_page_table[i] = ((PAGE_TABLE_ENTRIES + i) * PAGE_SIZE) | 3; // Present, writable
    }
    page_directory[1] = ((uint32_t)second_page_table) | 3; // Present, writable
    
    // Load page directory into CR3
    asm volatile ("mov %0, %%cr3" :: "r"(page_directory));
    
    // Enable paging by setting PG bit in CR0
    uint32_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set PG bit
    asm volatile ("mov %0, %%cr0" :: "r"(cr0));
    
    serial_print("Paging enabled with 8MB identity mapping\n");
}

void init_timer_interrupt() {
    serial_print("Timer interrupt setup...\n");
    
    // Program the PIT (Programmable Interval Timer)
    // Set frequency to 100Hz (10ms intervals)
    uint32_t divisor = 1193180 / 100;
    
    outb(0x43, 0x36); // Command byte: channel 0, lobyte/hibyte, rate generator
    outb(0x40, divisor & 0xFF);        // Low byte
    outb(0x40, (divisor >> 8) & 0xFF); // High byte
    
    serial_print("Timer configured for 100Hz\n");
}

void init_devices() {
    serial_print("Device initialization...\n");
    
    // Initialize keyboard controller
    serial_print("Initializing keyboard...\n");
    outb(0x64, 0xAE); // Enable keyboard
    
    // Initialize mouse/PS2
    serial_print("Initializing PS/2 controller...\n");
    outb(0x64, 0xA8); // Enable auxiliary device (mouse)
    
    // Initialize RTC
    serial_print("Initializing RTC...\n");
    outb(0x70, 0x8B); // Select register B
    uint8_t prev = inb(0x71);
    outb(0x70, 0x8B);
    outb(0x71, prev | 0x40); // Enable update-ended interrupts
    
    // Initialize CMOS
    serial_print("Reading CMOS...\n");
    outb(0x70, 0x00); // Seconds
    uint8_t sec = inb(0x71);
    (void)sec; // Suppress unused variable warning
    
    serial_print("Basic devices initialized\n");
}

void init_interrupt_controller() {
    serial_print("Interrupt controller setup...\n");
    
    // Remap PIC (Programmable Interrupt Controller)
    // ICW1 - Initialize PIC
    outb(0x20, 0x11); // Master PIC
    outb(0xA0, 0x11); // Slave PIC
    
    // ICW2 - Set interrupt vector offsets
    outb(0x21, 0x20); // Master PIC starts at interrupt 32
    outb(0xA1, 0x28); // Slave PIC starts at interrupt 40
    
    // ICW3 - Set up cascading
    outb(0x21, 0x04); // Master PIC has slave on IRQ2
    outb(0xA1, 0x02); // Slave PIC is cascade identity 2
    
    // ICW4 - Set mode
    outb(0x21, 0x01); // 8086 mode
    outb(0xA1, 0x01); // 8086 mode
    
    // Mask all interrupts initially except timer and keyboard
    outb(0x21, 0xFC); // Enable IRQ0 (timer) and IRQ1 (keyboard)
    outb(0xA1, 0xFF); // Mask all slave PIC interrupts
    
    serial_print("PIC remapped and configured\n");
}

// Syscall numbers
#define SYS_EXIT    1
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_GETPID  20

// Syscall handler
uint32_t syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    (void)arg1; // Suppress unused warning for now
    (void)arg2; // Suppress unused warning for now
    
    switch (syscall_num) {
        case SYS_EXIT:
            serial_print("Process exit called\n");
            // TODO: Terminate current process
            return 0;
        
        case SYS_READ:
            serial_print("Read syscall\n");
            // TODO: Implement read
            return 0;
        
        case SYS_WRITE:
            serial_print("Write syscall\n");
            // TODO: Implement write
            return arg3; // Return bytes written
        
        case SYS_GETPID:
            serial_print("GetPID syscall\n");
            // TODO: Return current process ID
            return 1; // Dummy PID
        
        default:
            serial_print("Unknown syscall: ");
            // Simple number output
            char num[10];
            int i = 0;
            uint32_t n = syscall_num;
            if (n == 0) {
                num[i++] = '0';
            } else {
                while (n > 0) {
                    num[i++] = '0' + (n % 10);
                    n /= 10;
                }
            }
            // Reverse the string
            for (int j = 0; j < i/2; j++) {
                char temp = num[j];
                num[j] = num[i-1-j];
                num[i-1-j] = temp;
            }
            num[i] = '\0';
            serial_print(num);
            serial_print("\n");
            return -1;
    }
}

void init_syscall_handler() {
    serial_print("Syscall handler setup...\n");
    
    // Set up syscall interrupt (int 0x80)
    idt_set_gate(0x80, (uint32_t)syscall_handler, 0x08, 0xEE); // User-accessible
    
    serial_print("Syscall handler registered at interrupt 0x80\n");
}

// Process states
enum ProcessState {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
};

// Process control block
struct Process {
    uint32_t pid;
    char name[32];
    enum ProcessState state;
    uint32_t esp;     // Stack pointer
    uint32_t ebp;     // Base pointer
    uint32_t eip;     // Instruction pointer
    uint32_t page_dir; // Page directory
    struct Process* next;
};

// Simple process list
static struct Process* process_list = NULL;
static struct Process* current_process = NULL;
static uint32_t next_pid = 1;

void init_scheduler() {
    serial_print("Scheduler initialization...\n");
    
    // Initialize process list
    process_list = NULL;
    current_process = NULL;
    
    serial_print("Scheduler ready\n");
}

// Idle thread function
void idle_thread() {
    while (1) {
        asm volatile ("hlt"); // Halt until next interrupt
    }
}

void create_idle_thread() {
    serial_print("Creating idle thread...\n");
    
    // Allocate memory for idle process
    static struct Process idle_process;
    
    idle_process.pid = 0; // Special PID for idle
    for (int i = 0; i < 32; i++) {
        idle_process.name[i] = "idle_thread"[i];
        if (idle_process.name[i] == '\0') break;
    }
    idle_process.state = PROCESS_READY;
    idle_process.eip = (uint32_t)idle_thread;
    idle_process.page_dir = (uint32_t)page_directory;
    idle_process.next = NULL;
    
    // Add to process list
    if (process_list == NULL) {
        process_list = &idle_process;
    } else {
        struct Process* proc = process_list;
        while (proc->next != NULL) {
            proc = proc->next;
        }
        proc->next = &idle_process;
    }
    
    serial_print("Idle thread created with PID 0\n");
}

// Simple process memory allocation (static for now)
static struct Process processes[16];
static int process_count = 0;

void create_user_process(const char* name) {
    serial_print("Creating user process: ");
    serial_print(name);
    serial_print("\n");
    
    if (process_count >= 16) {
        serial_print("ERROR: Too many processes\n");
        return;
    }
    
    struct Process* proc = &processes[process_count++];
    
    proc->pid = next_pid++;
    
    // Copy name
    int i;
    for (i = 0; i < 31 && name[i] != '\0'; i++) {
        proc->name[i] = name[i];
    }
    proc->name[i] = '\0';
    
    proc->state = PROCESS_READY;
    proc->page_dir = (uint32_t)page_directory; // Share kernel page directory for now
    proc->next = NULL;
    
    // Set up basic entry point based on process name
    if (i >= 4 && proc->name[0] == 'i' && proc->name[1] == 'n' && proc->name[2] == 'i' && proc->name[3] == 't') {
        // Init process - basic system initialization
        proc->eip = (uint32_t)idle_thread; // Placeholder
    } else {
        // Other processes
        proc->eip = (uint32_t)idle_thread; // Placeholder
    }
    
    // Add to process list
    if (process_list == NULL) {
        process_list = proc;
    } else {
        struct Process* p = process_list;
        while (p->next != NULL) {
            p = p->next;
        }
        p->next = proc;
    }
    
    serial_print("Process created with PID ");
    // Simple number output
    char pid_str[10];
    int j = 0;
    uint32_t pid = proc->pid;
    if (pid == 0) {
        pid_str[j++] = '0';
    } else {
        while (pid > 0) {
            pid_str[j++] = '0' + (pid % 10);
            pid /= 10;
        }
    }
    // Reverse
    for (int k = 0; k < j/2; k++) {
        char temp = pid_str[k];
        pid_str[k] = pid_str[j-1-k];
        pid_str[j-1-k] = temp;
    }
    pid_str[j] = '\0';
    serial_print(pid_str);
    serial_print("\n");
}

void schedule() {
    // Simple round-robin scheduler
    if (process_list == NULL) {
        // No processes, just halt
        asm volatile ("hlt");
        return;
    }
    
    // Find next ready process
    struct Process* next = NULL;
    
    if (current_process == NULL) {
        // Start with first process
        next = process_list;
    } else {
        // Look for next ready process after current
        next = current_process->next;
        if (next == NULL) {
            next = process_list; // Wrap around
        }
    }
    
    // Find a ready process
    struct Process* start = next;
    while (next != NULL && next->state != PROCESS_READY) {
        next = next->next;
        if (next == NULL) {
            next = process_list; // Wrap around
        }
        if (next == start) {
            break; // Avoid infinite loop
        }
    }
    
    if (next == NULL || next->state != PROCESS_READY) {
        // No ready process found, halt
        asm volatile ("hlt");
        return;
    }
    
    // Switch to next process
    if (current_process != next) {
        current_process = next;
        current_process->state = PROCESS_RUNNING;
        
        // For now, just yield CPU since we don't have full context switching
        asm volatile ("pause");
    }
    
    // Yield CPU
    asm volatile ("hlt");
}

// Simple memory management
static uint32_t kernel_heap_ptr = 0x100000;

void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    
    // Align to 4 bytes
    size = (size + 3) & ~3;
    
    void* ptr = (void*)kernel_heap_ptr;
    kernel_heap_ptr += size;
    
    return ptr;
}

// Authentication gating logic
void wait_for_auth() {
    struct AuthMsg msg;
    serial_print("Waiting for biometric authentication...\n");
    
    // Initialize auth message
    msg.type = AUTH_VERIFY;
    msg.user_id = 1;
    msg.timestamp = timer_ticks;
    
    // Clear security token
    for (int i = 0; i < 32; i++) {
        msg.security_token[i] = 0;
    }
    
    while (1) {
        // TODO: Replace with actual SAL implementation
        // if (sal_recv(AUTH_CHANNEL, &msg, sizeof(msg)) == sizeof(msg)) {
        //     if (msg.type == AUTH_SUCCESS) {
        //         serial_print("Authentication successful!\n");
        //         break;
        //     }
        // }
        
        // Temporary: simulate authentication after some time
        static int counter = 0;
        counter++;
        if (counter > 1000000) {
            serial_print("Simulating authentication success...\n");
            // Simulate successful auth message
            msg.type = AUTH_SUCCESS;
            msg.timestamp = timer_ticks;
            // Use msg to suppress warning
            (void)msg.user_id;
            serial_print("User authenticated, proceeding with desktop launch\n");
            break;
        }
        
        // Could yield CPU or show waiting UI
        asm volatile ("pause");
    }
}

void kernel_init() {
    serial_print("AeroDesk Kernel Initialization\n");
    
    init_syscall_handler();
    init_scheduler();
    
    // Launch initial user process (init/manager)
    create_idle_thread();
    create_user_process("init");
    
    // Start biometric auth service
    create_user_process("auth_service");
    
    // Wait for authentication before launching desktop
    wait_for_auth();
    
    // Launch desktop shell after authentication
    create_user_process("desktop_shell");
    
    serial_print("Entering kernel main loop...\n");
    
    // Kernel main loop
    while (1) {
        schedule();
    }
}

// Kernel entry point called from assembly
void kernel_main(uint32_t magic, uint32_t multiboot_addr) {
    (void)multiboot_addr; // Suppress unused warning for now
    
    // Initialize serial for early debug output
    init_serial();
    
    serial_print("AeroDesk OS Starting in 32-bit mode...\n");
    
    // Verify multiboot2 magic number
    if (magic != 0x36d76289) {
        serial_print("ERROR: Invalid multiboot2 magic number: 0x");
        // Simple hex output for debugging
        char hex[9];
        for (int i = 7; i >= 0; i--) {
            int digit = (magic >> (i * 4)) & 0xF;
            hex[7-i] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        }
        hex[8] = '\0';
        serial_print(hex);
        serial_print("\n");
        while (1) asm volatile ("hlt");
    }
    
    serial_print("Multiboot2 magic verified\n");
    
    // Disable interrupts initially
    asm volatile ("cli");
    serial_print("Interrupts disabled\n");
    
    // Set up basic kernel environment
    serial_print("Initializing GDT...\n");
    init_gdt();
    serial_print("GDT initialized\n");
    
    serial_print("Initializing IDT...\n");
    init_idt();
    serial_print("IDT initialized\n");
    
    serial_print("Setting up paging...\n");
    enable_paging();
    serial_print("Paging setup complete\n");
    
    serial_print("Initializing timer...\n");
    init_timer_interrupt();
    serial_print("Timer initialized\n");
    
    serial_print("Initializing devices...\n");
    init_devices();
    serial_print("Devices initialized\n");
    
    // Call higher-level kernel initialization
    serial_print("Calling kernel_init()...\n");
    kernel_init();
    
    // Should never reach here
    while (1) {
        asm volatile ("hlt");
    }
}
