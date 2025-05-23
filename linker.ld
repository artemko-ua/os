ENTRY(_start)

SECTIONS {
    . = 1M;
    
    .multiboot : {
        *(.multiboot)
    }
    
    .text : {
        *(.text)
        *(.text.*)
    }
    
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }
    
    .data : {
        *(.data)
        *(.data.*)
    }
    
    .bss : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
    
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
        *(.note.*)
    }
}
