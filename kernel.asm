section .multiboot_header
header_start:
    ; magic number
    dd 0xe85250d6                ; multiboot2
    ; architecture
    dd 0                         ; protected mode i386
    ; header length
    dd header_end - header_start
    ; checksum
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    ; required end tag
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
header_end:

section .bss
align 16
stack_bottom:
resb 16384 ; 16 KB stack
stack_top:

section .text
global _start
extern kernel_main

_start:
    ; Встановлюємо стек
    mov esp, stack_top

    ; Очищуємо EFLAGS
    push 0
    popf

    ; Викликаємо головну функцію ядра
    call kernel_main

    ; Якщо kernel_main повертається, зупиняємо CPU
    cli
.hang:
    hlt
    jmp .hang

; Функція для включення переривань
global enable_interrupts
enable_interrupts:
    sti
    ret

; Функція для вимкнення переривань
global disable_interrupts
disable_interrupts:
    cli
    ret

; Обробник переривання клавіатури
global keyboard_interrupt_handler
extern keyboard_handler
keyboard_interrupt_handler:
    pushad              ; Зберігаємо всі регістри
    
    call keyboard_handler
    
    ; Відправляємо EOI до PIC
    mov al, 0x20
    out 0x20, al
    
    popad              ; Відновлюємо всі регістри
    iret               ; Повертаємося з переривання

; Функція для перезавантаження
global reboot_system
reboot_system:
    ; Перезавантаження через клавіатурний контролер
    mov al, 0xFE
    out 0x64, al
    ; Якщо не спрацювало, використовуємо triple fault
    mov eax, cr0
    and eax, 0x7FFFFFFF ; Вимикаємо paging
    mov cr0, eax
    
    ; Неправильний дескриптор для triple fault
    lgdt [invalid_gdt]
    ret

; Функція для вимкнення системи
global shutdown_system
shutdown_system:
    ; Спробуємо ACPI shutdown
    mov dx, 0x604  ; QEMU ACPI shutdown port
    mov ax, 0x2000
    out dx, ax
    
    ; Якщо ACPI не працює, просто зупиняємо CPU
    cli
.loop:
    hlt
    jmp .loop

; Завантаження IDT
global load_idt
load_idt:
    mov eax, [esp + 4]  ; Отримуємо адресу IDT descriptor
    lidt [eax]
    ret

section .data
; Неправильний GDT для triple fault
invalid_gdt:
    dd 0x00000000
    dd 0x00000000