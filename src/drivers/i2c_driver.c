#include <stdint.h>

// I2C controller definitions for hardware I2C
#define I2C_BASE_ADDR   0xFE804000  // Example BCM2835 BSC1 base address  
#define I2C_C_REG       (I2C_BASE_ADDR + 0x00)  // Control register
#define I2C_S_REG       (I2C_BASE_ADDR + 0x04)  // Status register
#define I2C_DLEN_REG    (I2C_BASE_ADDR + 0x08)  // Data length register
#define I2C_A_REG       (I2C_BASE_ADDR + 0x0C)  // Slave address register
#define I2C_FIFO_REG    (I2C_BASE_ADDR + 0x10)  // Data FIFO register
#define I2C_DIV_REG     (I2C_BASE_ADDR + 0x14)  // Clock divider register
#define I2C_DEL_REG     (I2C_BASE_ADDR + 0x18)  // Data delay register
#define I2C_CLKT_REG    (I2C_BASE_ADDR + 0x1C)  // Clock stretch timeout

// Control register bits
#define I2C_C_I2CEN     (1 << 15)  // I2C enable
#define I2C_C_INTR      (1 << 10)  // Interrupt on RX
#define I2C_C_INTT      (1 << 9)   // Interrupt on TX  
#define I2C_C_INTD      (1 << 8)   // Interrupt on DONE
#define I2C_C_ST        (1 << 7)   // Start transfer
#define I2C_C_CLEAR     (3 << 4)   // Clear FIFO
#define I2C_C_READ      (1 << 0)   // Read transfer

// Status register bits
#define I2C_S_CLKT      (1 << 9)   // Clock stretch timeout
#define I2C_S_ERR       (1 << 8)   // ACK error
#define I2C_S_RXF       (1 << 7)   // RX FIFO full
#define I2C_S_TXE       (1 << 6)   // TX FIFO empty
#define I2C_S_RXD       (1 << 5)   // RX FIFO contains data
#define I2C_S_TXD       (1 << 4)   // TX FIFO can accept data
#define I2C_S_RXR       (1 << 3)   // RX FIFO needs reading
#define I2C_S_TXW       (1 << 2)   // TX FIFO needs writing
#define I2C_S_DONE      (1 << 1)   // Transfer done
#define I2C_S_TA        (1 << 0)   // Transfer active

// Memory-mapped I/O functions
static inline void mmio_write(uint32_t addr, uint32_t value) {
    volatile uint32_t* ptr = (volatile uint32_t*)addr;
    *ptr = value;
}

static inline uint32_t mmio_read(uint32_t addr) {
    volatile uint32_t* ptr = (volatile uint32_t*)addr;
    return *ptr;
}

// Initialize I2C controller
void i2c_init() {
    // Set clock divider for ~100kHz (assuming 250MHz core clock)
    // Divider = core_clock / desired_i2c_clock
    mmio_write(I2C_DIV_REG, 2500);
    
    // Set data delay and clock stretch timeout
    mmio_write(I2C_DEL_REG, (48 << 16) | 48);  // Rising/falling edge delays
    mmio_write(I2C_CLKT_REG, 64);  // Clock stretch timeout
    
    // Clear FIFO and enable I2C
    mmio_write(I2C_C_REG, I2C_C_I2CEN | I2C_C_CLEAR);
}

// Wait for transfer to complete
static int i2c_wait_done() {
    int timeout = 1000000;
    
    while (timeout-- > 0) {
        uint32_t status = mmio_read(I2C_S_REG);
        
        if (status & I2C_S_DONE) {
            // Clear status flags
            mmio_write(I2C_S_REG, I2C_S_DONE | I2C_S_ERR | I2C_S_CLKT);
            
            if (status & I2C_S_ERR) {
                return -1;  // ACK error
            }
            if (status & I2C_S_CLKT) {
                return -2;  // Clock stretch timeout
            }
            return 0;  // Success
        }
    }
    
    return -3;  // Timeout
}

// Read register from I2C device
int i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data) {
    // Clear FIFO and status
    mmio_write(I2C_C_REG, I2C_C_I2CEN | I2C_C_CLEAR);
    mmio_write(I2C_S_REG, I2C_S_DONE | I2C_S_ERR | I2C_S_CLKT);
    
    // Set slave address and data length for write
    mmio_write(I2C_A_REG, dev_addr);
    mmio_write(I2C_DLEN_REG, 1);
    
    // Write register address
    mmio_write(I2C_FIFO_REG, reg);
    
    // Start write transfer
    mmio_write(I2C_C_REG, I2C_C_I2CEN | I2C_C_ST);
    
    // Wait for write to complete
    if (i2c_wait_done() != 0) {
        return -1;
    }
    
    // Set up for read
    mmio_write(I2C_DLEN_REG, 1);
    
    // Start read transfer
    mmio_write(I2C_C_REG, I2C_C_I2CEN | I2C_C_ST | I2C_C_READ);
    
    // Wait for read to complete
    if (i2c_wait_done() != 0) {
        return -1;
    }
    
    // Read data from FIFO
    *data = mmio_read(I2C_FIFO_REG) & 0xFF;
    
    return 0;
}

// Write register to I2C device
int i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data) {
    // Clear FIFO and status
    mmio_write(I2C_C_REG, I2C_C_I2CEN | I2C_C_CLEAR);
    mmio_write(I2C_S_REG, I2C_S_DONE | I2C_S_ERR | I2C_S_CLKT);
    
    // Set slave address and data length
    mmio_write(I2C_A_REG, dev_addr);
    mmio_write(I2C_DLEN_REG, 2);  // Register address + data
    
    // Write register address and data to FIFO
    mmio_write(I2C_FIFO_REG, reg);
    mmio_write(I2C_FIFO_REG, data);
    
    // Start transfer
    mmio_write(I2C_C_REG, I2C_C_I2CEN | I2C_C_ST);
    
    // Wait for completion
    return i2c_wait_done();
}

// Read multiple bytes from I2C device
int i2c_read_bytes(uint8_t dev_addr, uint8_t reg, uint8_t *data, int len) {
    // Clear FIFO and status
    mmio_write(I2C_C_REG, I2C_C_I2CEN | I2C_C_CLEAR);
    mmio_write(I2C_S_REG, I2C_S_DONE | I2C_S_ERR | I2C_S_CLKT);
    
    // Set slave address and data length for register write
    mmio_write(I2C_A_REG, dev_addr);
    mmio_write(I2C_DLEN_REG, 1);
    
    // Write register address
    mmio_write(I2C_FIFO_REG, reg);
    
    // Start write transfer
    mmio_write(I2C_C_REG, I2C_C_I2CEN | I2C_C_ST);
    
    // Wait for write to complete
    if (i2c_wait_done() != 0) {
        return -1;
    }
    
    // Set up for multi-byte read
    mmio_write(I2C_DLEN_REG, len);
    
    // Start read transfer
    mmio_write(I2C_C_REG, I2C_C_I2CEN | I2C_C_ST | I2C_C_READ);
    
    // Read data bytes as they become available
    for (int i = 0; i < len; i++) {
        // Wait for data in RX FIFO
        int timeout = 1000000;
        while (!(mmio_read(I2C_S_REG) & I2C_S_RXD) && timeout-- > 0);
        
        if (timeout <= 0) {
            return -1;  // Timeout
        }
        
        data[i] = mmio_read(I2C_FIFO_REG) & 0xFF;
    }
    
    // Wait for transfer completion
    if (i2c_wait_done() != 0) {
        return -1;
    }
    
    return len;
}

// Write multiple bytes to I2C device
int i2c_write_bytes(uint8_t dev_addr, uint8_t reg, const uint8_t *data, int len) {
    // Clear FIFO and status
    mmio_write(I2C_C_REG, I2C_C_I2CEN | I2C_C_CLEAR);
    mmio_write(I2C_S_REG, I2C_S_DONE | I2C_S_ERR | I2C_S_CLKT);
    
    // Set slave address and data length (register + data bytes)
    mmio_write(I2C_A_REG, dev_addr);
    mmio_write(I2C_DLEN_REG, len + 1);
    
    // Write register address first
    mmio_write(I2C_FIFO_REG, reg);
    
    // Write data bytes
    for (int i = 0; i < len; i++) {
        // Wait for space in TX FIFO
        int timeout = 1000000;
        while (!(mmio_read(I2C_S_REG) & I2C_S_TXD) && timeout-- > 0);
        
        if (timeout <= 0) {
            return -1;  // Timeout
        }
        
        mmio_write(I2C_FIFO_REG, data[i]);
    }
    
    // Start transfer
    mmio_write(I2C_C_REG, I2C_C_I2CEN | I2C_C_ST);
    
    // Wait for completion
    return i2c_wait_done();
}

// Set I2C clock speed
void i2c_set_clock(uint32_t clock_hz) {
    // Assuming 250MHz core clock
    uint32_t divider = 250000000 / clock_hz;
    mmio_write(I2C_DIV_REG, divider);
}
