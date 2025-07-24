ENTRY(_start)

SECTIONS
{
    . = 0x100000;  /* Load kernel at 1 MiB */
    
    .multiboot2_header ALIGN(8) :
    {
        KEEP(*(.multiboot2_header))
    }
    
    .text ALIGN(4096) :
    {
        *(.text*)
    }
    
    .rodata ALIGN(4096) :
    {
        *(.rodata*)
    }
    
    .data ALIGN(4096) :
    {
        *(.data*)
    }
    
    .bss ALIGN(4096) :
    {
        *(.bss*)
        *(COMMON)
    }
    
    /* Define end of kernel for memory management */
    kernel_end = .;
}
