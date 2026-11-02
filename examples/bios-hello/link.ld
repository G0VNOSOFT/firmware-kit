ENTRY(_start)

SECTIONS {
    . = 0x10000;

    .text : {
        *(.text.entry)
        *(.text)
        *(.text.*)
    }

    .rdata : {
        *(.rdata)
        *(.rdata.*)
        *(.rodata)
        *(.rodata.*)
    }

    .data : {
        *(.data)
        *(.data.*)
    }

    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }

    /DISCARD/ : {
        *(.comment)
        *(.pdata)
        *(.xdata)
        *(.eh_frame)
        *(.debug*)
    }
}
