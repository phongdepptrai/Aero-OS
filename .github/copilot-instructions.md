# AeroDesk OS - AI Coding Assistant Instructions

## Project Overview
AeroDesk is a microkernel-based OS for x86_64 with mandatory biometric authentication. The kernel provides only essential services (scheduling, IPC, memory management) while all other functionality runs in isolated user-space servers communicating via the Service Abstraction Layer (SAL).

## Architecture Principles

### Microkernel Design
- **Kernel responsibilities**: Scheduling, threads, basic memory management, IPC primitives, interrupt handling
- **User-space servers**: File systems, drivers, biometric analysis, UI rendering
- **Communication**: All inter-process communication goes through SAL message passing
- **Isolation**: Services cannot directly access each other's memory - only via controlled IPC

### Critical Boot Flow
1. GRUB Multiboot2 → Kernel init → User-space service startup
2. **Authentication gate**: Desktop shell launch is blocked until biometric auth succeeds
3. Biometric services (HRV, EEG) → Auth service → SAL message to kernel → Shell launch

## Project Structure

### Directory Organization
```
src/
├── kernel/     # Microkernel implementation
├── sal/        # Service Abstraction Layer
├── drivers/    # Hardware drivers (user-space)
└── services/   # User-space services
include/        # Header files
boot/grub/      # Boot configuration
```

### Key Components & Patterns

#### Service Abstraction Layer (SAL)
- **API Functions**: `sal_send()`, `sal_recv()`, `sal_publish()`, `sal_subscribe()`
- **Implementation**: Located in `src/sal/sal.c`, wraps kernel IPC syscalls
- **Usage Pattern**: Services publish data (e.g., "heart_rate" topic), others subscribe
- **Header**: `include/sal/sal.h` contains all API definitions

#### Biometric Authentication System
- **HRV Server**: `src/services/biometric_services.c` - processes heart rate variability
- **EEG Server**: Same file - handles brainwave data processing and feature extraction  
- **Auth Service**: `src/services/auth_service.c` - validates against stored profiles
- **Security Gate**: Kernel waits for `AUTH_SUCCESS` message before launching UI

#### Hardware Drivers (User-Space)
- **UART Driver**: `src/drivers/uart_driver.c` - memory-mapped I/O for modern controllers
- **SPI Driver**: `src/drivers/spi_driver.c` - hardware SPI controller interface
- **I2C Driver**: `src/drivers/i2c_driver.c` - hardware I2C controller with FIFO support
- **Integration**: All use memory-mapped I/O (`mmio_read/write`) not port I/O

## Development Patterns

### Build System
- **Makefile**: Organized build with separate object directories
- **Kernel**: Compile with `-m64 -ffreestanding -nostdlib -I include/`
- **Linking**: Use `src/kernel/linker.ld` for memory layout
- **SAL Library**: Built as `libsal.a` for linking with user processes

### Memory Management
- **Higher-half mapping**: Kernel loaded at 0x100000 (1MiB)
- **Headers**: All includes use relative paths from `include/` directory
- **Isolation**: Each service runs in separate address space

### Hardware Access Patterns
- **Modern Approach**: Use memory-mapped I/O with `mmio_read/write` functions
- **Register Definitions**: Hardware controllers have base addresses and register offsets
- **Error Handling**: Functions return status codes and use timeouts
- **FIFO Management**: Hardware drivers handle TX/RX FIFOs properly

## Critical Implementation Details

### Authentication Flow
```c
// Kernel gating logic in src/kernel/kernel.c
void wait_for_auth() {
    struct AuthMsg msg;
    while (1) {
        if (sal_recv(AUTH_CHANNEL, &msg, sizeof(msg)) == sizeof(msg)) {
            if (msg.type == AUTH_SUCCESS) break;
        }
    }
}
```

### SAL Usage Pattern
```c
// Publishing data (in biometric services)
sal_publish("heart_rate", &hrv_data, sizeof(hrv_data));

// Subscribing (in auth service)
sal_subscribe("heart_rate", hrv_callback);
```

### Hardware Driver Pattern
```c
// Memory-mapped I/O (not port I/O)
#define SPI_BASE_ADDR 0xFE204000
#define SPI_CS_REG    (SPI_BASE_ADDR + 0x00)

static inline void mmio_write(uint32_t addr, uint32_t value) {
    volatile uint32_t* ptr = (volatile uint32_t*)addr;
    *ptr = value;
}
```

### File Organization Rules
- **Headers**: Place in `include/` with subdirectories matching functionality
- **Implementation**: Match directory structure in `src/`
- **Includes**: Always use relative paths from project root
- **Services**: Each major service gets its own file in `src/services/`

## Build & Test Commands
```bash
make            # Build kernel and SAL library
make iso        # Create bootable ISO
make qemu-iso   # Test in QEMU with ISO
make clean      # Clean build artifacts
make tree       # Show project structure
```

## Key Differences from Standard OS Development
- **User-space drivers**: Hardware drivers run as services, not kernel modules
- **SAL messaging**: Use pub/sub instead of direct function calls
- **Memory-mapped I/O**: Modern hardware controllers, not legacy port I/O
- **Authentication gating**: Unique boot flow with biometric blocking
- **Microkernel isolation**: Services are completely isolated processes
