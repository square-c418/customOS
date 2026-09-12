; boot.asm - Minimal 32-bit x86 Bootloader multiboot header
bits 32                         ; We are operating in 32-bit protected mode
section .text
    align 4
    dd 0x1BADB002               ; Magic number for the multiboot specification
    dd 0x00                     ; Flags
    dd - (0x1BADB002 + 0x00)     ; Checksum to prove we are multiboot compliant

global _start
extern kernel_main              ; This tells Assembly that 'kernel_main' is over in your C++ file

_start:
    cli                         ; Clear interrupts
    call kernel_main            ; Jump execution into your C++ code
    hlt                         ; Halt the CPU if C++ ever returns
