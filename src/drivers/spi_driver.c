#include <stdint.h>

// SPI pin definitions for hardware SPI controller
#define SPI_BASE_ADDR   0xFE204000  // Example BCM2835 SPI0 base address
#define SPI_CS_REG      (SPI_BASE_ADDR + 0x00)  // Control and Status
#define SPI_FIFO_REG    (SPI_BASE_ADDR + 0x04)  // TX and RX FIFOs
#define SPI_CLK_REG     (SPI_BASE_ADDR + 0x08)  // Clock divider
#define SPI_DLEN_REG    (SPI_BASE_ADDR + 0x0C)  // Data length
#define SPI_LTOH_REG    (SPI_BASE_ADDR + 0x10)  // LOSSI mode TOH
#define SPI_DC_REG      (SPI_BASE_ADDR + 0x14)  // DMA DREQ controls

// SPI Control and Status register bits
#define SPI_CS_LEN_LONG     (1 << 25)  // Enable long data word in LOSSI mode
#define SPI_CS_DMA_LEN      (1 << 24)  // Enable DMA mode in LOSSI mode  
#define SPI_CS_CSPOL2       (1 << 23)  // Chip select 2 polarity
#define SPI_CS_CSPOL1       (1 << 22)  // Chip select 1 polarity
#define SPI_CS_CSPOL0       (1 << 21)  // Chip select 0 polarity
#define SPI_CS_RXF          (1 << 20)  // RX FIFO full
#define SPI_CS_RXR          (1 << 19)  // RX FIFO needs reading
#define SPI_CS_TXD          (1 << 18)  // TX FIFO can accept data
#define SPI_CS_RXD          (1 << 17)  // RX FIFO contains data
#define SPI_CS_DONE         (1 << 16)  // Transfer done
#define SPI_CS_LEN          (1 << 13)  // Enable LOSSI mode
#define SPI_CS_REN          (1 << 12)  // Read enable
#define SPI_CS_ADCS         (1 << 11)  // Automatically deassert chip select
#define SPI_CS_INTR         (1 << 10)  // Interrupt on RXR
#define SPI_CS_INTD         (1 << 9)   // Interrupt on DONE
#define SPI_CS_DMAEN        (1 << 8)   // DMA enable
#define SPI_CS_TA           (1 << 7)   // Transfer active
#define SPI_CS_CSPOL        (1 << 6)   // Chip select polarity
#define SPI_CS_CLEAR_RX     (1 << 5)   // Clear RX FIFO
#define SPI_CS_CLEAR_TX     (1 << 4)   // Clear TX FIFO
#define SPI_CS_CPOL         (1 << 3)   // Clock polarity
#define SPI_CS_CPHA         (1 << 2)   // Clock phase
#define SPI_CS_CS           (3 << 0)   // Chip select mask

// Memory-mapped I/O functions
static inline void mmio_write(uint32_t addr, uint32_t value) {
    volatile uint32_t* ptr = (volatile uint32_t*)addr;
    *ptr = value;
}

static inline uint32_t mmio_read(uint32_t addr) {
    volatile uint32_t* ptr = (volatile uint32_t*)addr;
    return *ptr;
}

// Initialize SPI controller
void spi_init() {
    // Clear FIFOs and reset SPI controller
    mmio_write(SPI_CS_REG, SPI_CS_CLEAR_RX | SPI_CS_CLEAR_TX);
    
    // Set clock divider (example: 250MHz / 256 = ~976kHz)
    mmio_write(SPI_CLK_REG, 256);
    
    // Configure SPI mode (CPOL=0, CPHA=0, chip select 0)
    mmio_write(SPI_CS_REG, 0);
}

// Transfer one byte over SPI
uint8_t spi_transfer(uint8_t data_out) {
    // Wait for any previous transfer to complete
    while (!(mmio_read(SPI_CS_REG) & SPI_CS_DONE));
    
    // Clear FIFOs
    mmio_write(SPI_CS_REG, SPI_CS_CLEAR_RX | SPI_CS_CLEAR_TX);
    
    // Write data to TX FIFO
    mmio_write(SPI_FIFO_REG, data_out);
    
    // Start transfer
    mmio_write(SPI_CS_REG, SPI_CS_TA);
    
    // Wait for transfer to complete
    while (!(mmio_read(SPI_CS_REG) & SPI_CS_DONE));
    
    // Read received data
    uint8_t data_in = mmio_read(SPI_FIFO_REG) & 0xFF;
    
    // Clear TA bit to end transfer
    mmio_write(SPI_CS_REG, 0);
    
    return data_in;
}

// Write multiple bytes
int spi_write(const uint8_t* data, int len) {
    for (int i = 0; i < len; i++) {
        spi_transfer(data[i]);
    }
    return len;
}

// Read multiple bytes
int spi_read(uint8_t* data, int len) {
    for (int i = 0; i < len; i++) {
        data[i] = spi_transfer(0xFF);  // Send dummy byte to read
    }
    return len;
}

// Write then read (common SPI pattern)
int spi_write_read(const uint8_t* write_data, int write_len, uint8_t* read_data, int read_len) {
    // Write phase
    for (int i = 0; i < write_len; i++) {
        spi_transfer(write_data[i]);
    }
    
    // Read phase
    for (int i = 0; i < read_len; i++) {
        read_data[i] = spi_transfer(0xFF);
    }
    
    return read_len;
}

// Set chip select
void spi_set_chip_select(int cs) {
    uint32_t cs_reg = mmio_read(SPI_CS_REG);
    cs_reg &= ~SPI_CS_CS;  // Clear current CS
    cs_reg |= (cs & 3);    // Set new CS (0, 1, or 2)
    mmio_write(SPI_CS_REG, cs_reg);
}

// Set SPI mode (CPOL and CPHA)
void spi_set_mode(int mode) {
    uint32_t cs_reg = mmio_read(SPI_CS_REG);
    cs_reg &= ~(SPI_CS_CPOL | SPI_CS_CPHA);  // Clear mode bits
    
    switch (mode) {
        case 0: /* CPOL=0, CPHA=0 */ break;
        case 1: cs_reg |= SPI_CS_CPHA; break;         // CPOL=0, CPHA=1
        case 2: cs_reg |= SPI_CS_CPOL; break;         // CPOL=1, CPHA=0  
        case 3: cs_reg |= SPI_CS_CPOL | SPI_CS_CPHA; break; // CPOL=1, CPHA=1
    }
    
    mmio_write(SPI_CS_REG, cs_reg);
}

// Set clock divider (speed)
void spi_set_clock_divider(uint16_t divider) {
    mmio_write(SPI_CLK_REG, divider);
}
