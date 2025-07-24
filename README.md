# AeroDesk OS

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Architecture](https://img.shields.io/badge/arch-x86--32-green.svg)
![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)
![Status](https://img.shields.io/badge/status-development-orange.svg)

A security-focused microkernel operating system for x86 (32-bit) architecture featuring mandatory biometric authentication and a modular service-oriented design.

## 🎯 Overview

AeroDesk OS is a microkernel-based operating system designed from the ground up with security and modularity as core principles. The system enforces biometric authentication before granting access to the desktop environment and implements a strict separation between kernel space and user-space services.

### Key Features

- **🔒 Biometric Security**: Mandatory biometric authentication (HRV, EEG) before system access
- **🧩 Microkernel Architecture**: Minimal kernel with isolated user-space services
- **📡 Service Abstraction Layer (SAL)**: Advanced IPC with pub/sub messaging
- **🏗️ Modular Design**: Easy addition of new services and drivers
- **🛡️ Hardware Isolation**: Device drivers run in protected user-space
- **⚡ Real-time Capabilities**: Optimized for biometric sensor processing

## 🏛️ Architecture

AeroDesk follows a strict microkernel design philosophy where the kernel provides only essential services:

### Kernel Space (Minimal)
- **Process Management**: Thread scheduling and process isolation
- **Memory Management**: Virtual memory and page allocation  
- **IPC Primitives**: Low-level inter-process communication
- **Interrupt Handling**: Hardware interrupt management
- **System Calls**: Secure kernel-userspace interface

### User Space (Everything Else)
- **Device Drivers**: Hardware abstraction and control
- **Biometric Services**: HRV analysis, EEG processing, pattern recognition
- **Authentication Service**: User verification and access control
- **File Systems**: Storage management and file operations
- **UI Services**: Graphics rendering and window management
- **Network Stack**: Protocol implementation and communication

## 📊 Development Status

**Current Release**: `v0.3.0-alpha` - Stable 32-bit kernel with working build system

### ✅ Completed Features
- **Core Kernel**: 32-bit microkernel with multiboot2 support
- **Boot System**: GRUB integration with successful kernel handoff
- **Memory Setup**: GDT initialization and 32-bit addressing
- **Debug Infrastructure**: Serial port debugging (COM1) 
- **Build System**: Complete Makefile with dependencies
- **ISO Generation**: Bootable ISO creation with GRUB
- **QEMU Testing**: Integrated emulation testing workflow

### 🚧 In Development
- **SAL Implementation**: Complete IPC and messaging system
- **User-space Services**: Biometric processing and authentication
- **Hardware Drivers**: I2C, SPI, UART driver implementation  
- **Virtual Memory**: Full paging and memory protection
- **Process Scheduler**: Multi-tasking and thread management

### 📋 Planned Features
- **Real Hardware Support**: Boot on physical x86 systems
- **Biometric Hardware**: Integration with sensors and authentication devices
- **Desktop Environment**: Windowing system and user interface
- **Security Auditing**: Comprehensive security model validation

## 📁 Project Structure

```
AeroDesk_OS/
├── 🗂️ src/                    # Source code
│   ├── 🔧 kernel/             # Microkernel implementation
│   │   ├── boot.S             # Assembly bootstrap + Multiboot2
│   │   ├── kernel.c           # Main kernel (768 lines)
│   │   └── linker.ld          # Memory layout linker script
│   ├── 📡 sal/                # Service Abstraction Layer
│   │   └── sal.c              # IPC and pub/sub messaging
│   ├── 🔌 drivers/            # Hardware drivers (user-space)
│   │   ├── uart_driver.c      # Serial port driver
│   │   ├── spi_driver.c       # SPI bus driver
│   │   └── i2c_driver.c       # I2C bus driver
│   └── 🛠️ services/           # User-space services
│       ├── auth_service.c     # Biometric authentication
│       ├── biometric_services.c # HRV and EEG processing
│       └── render_service.c   # UI/graphics service
├── 📋 include/                # Header files
│   ├── sal/
│   │   └── sal.h              # SAL API definitions
│   └── auth.h                 # Authentication protocol
├── 🥾 boot/                   # Boot configuration
│   └── grub/
│       └── grub.cfg           # GRUB Multiboot2 config
├── 📚 docs/                   # Documentation
│   ├── IMPLEMENTATION.md      # Implementation details
│   ├── SAL_IMPLEMENTATION.md  # SAL architecture
│   └── TESTING.md             # Testing procedures
├── 🧪 test/                   # Test suites
│   ├── kernel_test.c          # Kernel unit tests
│   ├── sal_test.c             # SAL messaging tests
│   └── interactive_test.c     # Integration tests
├── 🏗️ build/                  # Build artifacts (generated)
├── 💿 simple_iso/             # Pre-built ISO directory
├── 🔧 Makefile                # Build system
└── 📖 README.md               # This file
```

### Component Details

| Component | Purpose | Language | Status |
|-----------|---------|----------|---------|
| **Microkernel** | Core OS services, minimal functionality | C + Assembly | ✅ Stable |
| **SAL** | Inter-process communication layer | C | 🚧 In Progress |
| **Drivers** | Hardware abstraction (user-space) | C | 🚧 Skeleton |
| **Services** | Biometric processing, auth, UI | C | 🚧 Planned |
| **Boot System** | GRUB + Multiboot2 initialization | Assembly + Config | ✅ Working |
## 🔧 Quick Start

### Prerequisites

#### Ubuntu/Debian
```bash
# Essential build tools
sudo apt update
sudo apt install build-essential gcc-multilib libc6-dev-i386

# Bootloader and emulation
sudo apt install grub-pc-bin grub-common xorriso qemu-system-x86

# Optional: Development tools
sudo apt install gdb nasm tree
```

#### Using Make (Automated)
```bash
make deps    # Install all dependencies automatically
```

### Building the OS

```bash
# Clone the repository
git clone <repository-url>
cd AeroDesk_OS

# Build everything (kernel + services + ISO)
make all

# Create bootable ISO
make iso

# Test in QEMU (recommended)
make qemu-iso
```

### Build Targets

| Command | Description | Output |
|---------|-------------|---------|
| `make` | Build kernel and all components | `aerodesk_kernel.elf` |
| `make iso` | Create bootable ISO image | `aerodesk.iso` |
| `make qemu` | Test kernel directly in QEMU | Boots kernel |
| `make qemu-iso` | Test ISO in QEMU (recommended) | Boots from ISO |
| `make clean` | Remove all build artifacts | Clean workspace |
| `make deps` | Install system dependencies | - |
| `make tree` | Show project file structure | File listing |

### First Boot

1. **Build the ISO**: `make iso`
2. **Test with QEMU**: `make qemu-iso`
3. **Expected Output**:
   ```
   Loading AeroDesk kernel...
   Starting AeroDesk..
   AeroDesk OS Starting in 32-bit mode...
   Multiboot2 magic verified
   Interrupts disabled
   Initializing GDT...
   GDT initialized
   [... full initialization sequence ...]
   Simulating authentication success...
   Entering kernel main loop...
   ```

## 🛠️ Development

### Build Configuration

The build system is configured for **32-bit x86 architecture**:

```makefile
# Compiler flags
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -nostdlib -fno-pic -I$(INCLUDE_DIR)
ASFLAGS = --32
LDFLAGS = -m elf_i386 -T $(KERNEL_DIR)/linker.ld -nostdlib
```

### Development Workflow


## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Performance improvement

## Testing
- [ ] Builds successfully with `make`
- [ ] Boots in QEMU with `make qemu-iso`
- [ ] Serial output shows expected behavior
- [ ] No regressions in existing functionality

## Checklist
- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] Tests pass
```

## 🔮 Roadmap

### Version 0.4.0 - SAL Implementation
- [ ] Complete IPC message passing
- [ ] Implement pub/sub system
- [ ] Add shared memory support
- [ ] Service discovery mechanism

### Version 0.5.0 - User-Space Services
- [ ] Biometric processing services
- [ ] Authentication service
- [ ] Basic device drivers
- [ ] Process management improvements

### Version 0.6.0 - Hardware Integration
- [ ] Real biometric sensor support
- [ ] Hardware abstraction layer
- [ ] Device driver framework
- [ ] Interrupt handling improvements

### Version 1.0.0 - Production Ready
- [ ] Complete desktop environment
- [ ] Security audit and validation
- [ ] Performance optimization
- [ ] Comprehensive documentation
- [ ] Hardware compatibility testing

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Multiboot2 Specification**: For standardized bootloader interface
- **OSDev Community**: For x86 architecture documentation and guidance
- **QEMU Project**: For providing excellent emulation and testing capabilities
- **GNU Toolchain**: For reliable cross-compilation tools

---

**AeroDesk OS** - Secure, Modular, Biometric-Enabled Operating System
*Built with ❤️ for security-conscious computing*

// Service initialization
void my_service_init() {
    // Initialize any hardware or data structures
    sal_subscribe(MY_SERVICE_CHANNEL);
}

// Main service loop
void my_service_run() {
    struct SalMessage msg;
    
    while (1) {
        // Listen for incoming messages
        if (sal_recv(MY_SERVICE_CHANNEL, &msg, sizeof(msg)) > 0) {
            // Process the message
            handle_message(&msg);
        }
        
        // Do periodic work
        do_service_work();
        
        // Yield CPU to other processes
        asm volatile ("pause");
    }
}

// Message handling
void handle_message(struct SalMessage* msg) {
    switch (msg->type) {
        case MSG_REQUEST:
            // Handle request and send response
            process_request(msg);
            break;
        case MSG_DATA:
            // Process incoming data
            process_data(msg);
            break;
        default:
            // Unknown message type
            break;
    }
}

// Entry point for the service
int main() {
    my_service_init();
    my_service_run();
    return 0;  // Should never reach here
}
```

#### 3. Add Header File (if needed)
```c
// include/my_service.h
#ifndef MY_SERVICE_H
#define MY_SERVICE_H

#include <stdint.h>

// Message types for this service
#define MSG_REQUEST     0x01
#define MSG_RESPONSE    0x02
#define MSG_DATA        0x03

// Service-specific data structures
struct MyServiceData {
    uint32_t sensor_value;
    uint32_t timestamp;
    uint16_t status;
};

// Public API functions
void my_service_init(void);
void my_service_run(void);
int my_service_request(uint32_t request_type, void* data);

#endif // MY_SERVICE_H
```

#### 4. Update Makefile
```makefile
# Add to SERVICES_SRCS variable
SERVICE_SRCS = $(wildcard $(SERVICES_DIR)/*.c) src/services/my_new_service.c

# Or add specific object file
MY_SERVICE_OBJ = $(BUILD_DIR)/my_new_service.o

# Add build rule
$(BUILD_DIR)/my_new_service.o: src/services/my_new_service.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Add to final linking
aerodesk_kernel.elf: ... $(MY_SERVICE_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^
```

#### 5. Register Service in Kernel
```c
// In kernel.c, add to kernel_init()
void kernel_init() {
    // ... existing initialization ...
    
    // Launch your new service
    create_user_process("my_new_service");
    
    // ... rest of initialization ...
}
```

### Module Types & Examples

#### A. Hardware Driver Module
```c
// src/drivers/gpio_driver.c - GPIO hardware control
#include "../include/sal/sal.h"

#define GPIO_BASE_ADDR  0x40020000
#define GPIO_CHANNEL    0x200

void gpio_init() {
    // Memory-map GPIO registers
    // Set up GPIO pins as inputs/outputs
}

void gpio_set_pin(uint8_t pin, uint8_t value) {
    // Direct hardware register access
    volatile uint32_t* gpio_reg = (uint32_t*)(GPIO_BASE_ADDR + pin);
    *gpio_reg = value;
}

void gpio_service_loop() {
    struct SalMessage msg;
    while (1) {
        if (sal_recv(GPIO_CHANNEL, &msg, sizeof(msg)) > 0) {
            if (msg.type == GPIO_SET_PIN) {
                gpio_set_pin(msg.data[0], msg.data[1]);
            }
        }
    }
}
```

#### B. Data Processing Service
```c
// src/services/sensor_processor.c - Process sensor data
#include "../include/sal/sal.h"

#define SENSOR_DATA_CHANNEL 0x300
#define PROCESSED_DATA_CHANNEL 0x301

struct SensorReading {
    float temperature;
    float humidity;
    uint32_t timestamp;
};

void process_sensor_data(struct SensorReading* raw) {
    // Apply calibration, filtering, etc.
    struct SensorReading processed = *raw;
    processed.temperature = raw->temperature * 1.05 + 2.0;  // Calibration
    
    // Publish processed data
    sal_publish(PROCESSED_DATA_CHANNEL, &processed, sizeof(processed));
}

void sensor_processor_main() {
    sal_subscribe(SENSOR_DATA_CHANNEL);
    
    struct SensorReading reading;
    while (1) {
        if (sal_recv(SENSOR_DATA_CHANNEL, &reading, sizeof(reading)) > 0) {
            process_sensor_data(&reading);
        }
    }
}
```

#### C. Communication Protocol Module
```c
// src/services/network_service.c - Network protocol handling
#include "../include/sal/sal.h"

#define NETWORK_IN_CHANNEL  0x400
#define NETWORK_OUT_CHANNEL 0x401

void network_send_packet(uint8_t* data, size_t len) {
    // Format packet and send via hardware driver

