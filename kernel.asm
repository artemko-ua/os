; Nexus OS Kernel
; x86_64 Kernel with Multiboot2 support

[BITS 64]

; Multiboot2 header
section .multiboot
align 8
multiboot_header:
    dd 0xe85250d6             ; magic number (multiboot2)
    dd 0                      ; architecture 0 (protected mode i386)
    dd multiboot_header_end - multiboot_header  ; header length
    
    ; checksum
    dd -(0xe85250d6 + 0 + (multiboot_header_end - multiboot_header))
    
    ; tags
    ; end tag
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
multiboot_header_end:

section .text
global _start

extern kmain  ; C kernel main function

_start:
    ; Clear interrupts
    cli
    
    ; Set up stack
    mov rsp, stack_top
    
    ; Set up segment registers
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Call the C kernel main function
    call kmain
    
    ; If kmain returns, halt the CPU
.halt:
    hlt
    jmp .halt

section .bss
align 16
stack_bottom:
    resb 16384  ; 16 KB stack
stack_top:

; Pad to ensure the kernel meets multiboot requirements
times 512 - ($ - $$) db 0 
