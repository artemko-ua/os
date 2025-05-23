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

    ; end tag
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
    ret

; Функція для вимкнення системи
global shutdown_system
shutdown_system:
    ; Вимкнення через ACPI (якщо підтримується)
    ; Або просто зупинка CPU
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
; IDT для обробки переривань
align 8
idt:
    times 256 dd 0, 0   ; 256 записів IDT (8 байт кожен)

idt_descriptor:
    dw 256*8 - 1        ; Розмір IDT - 1
    dd idt              ; Адреса IDT

global idt_descriptor

; Виправлення попередження про виконуваний стек
section .note.GNU-stack noalloc noexec nowrite progbits 