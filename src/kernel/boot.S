# AeroDesk OS - Multiboot2 Bootstrap
# Assembly entry point with Multiboot2 header

.section .multiboot2_header, "a"
.align 8
multiboot_header:
    .long 0xE85250D6                    # Multiboot2 magic number
    .long 0                             # Architecture: i386 (0)
    .long multiboot_header_end - multiboot_header    # Header length
    .long -(0xE85250D6 + 0 + (multiboot_header_end - multiboot_header))  # Checksum

    # Information request tag
    .align 8
    .word 1    # type: information request
    .word 0    # flags
    .long 12   # size
    .long 4    # mbi_tag_basic_meminfo
    
    # End tag - required
    .align 8
    .word 0    # type: end tag
    .word 0    # flags  
    .long 8    # size
multiboot_header_end:

.section .bss
.align 16
stack_bottom:
.skip 16384  # 16 KiB stack
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    # Set up 32-bit stack
    mov $stack_top, %esp
    
    # Clear direction flag
    cld
    
    # Save multiboot2 magic and info pointer
    push %ebx   # Multiboot information structure
    push %eax   # Magic value
    
    # Call the kernel main function
    call kernel_main
    
    # If kernel_main returns, halt the system
    cli
1:  hlt
    jmp 1b

.size _start, . - _start
