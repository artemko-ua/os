ENTRY(_start)

SECTIONS
{
    /* Завантажуємо ядро за адресою 1MB */
    . = 0x100000;

    /* Multiboot header має бути на початку */
    .multiboot ALIGN(4K) : {
        *(.multiboot_header)
    }

    /* Текстова секція (код) */
    .text ALIGN(4K) : {
        *(.text)
    }

    /* Readonly дані */
    .rodata ALIGN(4K) : {
        *(.rodata)
    }

    /* Ініціалізовані дані */
    .data ALIGN(4K) : {
        *(.data)
    }

    /* Неініціалізовані дані (BSS) */
    .bss ALIGN(4K) : {
        *(.bss)
        *(COMMON)
    }

    /* Забезпечуємо правильне вирівнювання кінця ядра */
    . = ALIGN(4K);
    end = .;
}