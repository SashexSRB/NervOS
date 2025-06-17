/* kernel/linker.ld */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(kernel_main)

SECTIONS
{
    . = 0x100000; /* Load kernel at 1 MB (0x100000) */
    .text : ALIGN(0x1000) {
        *(.text)
        *(.text.*)
    }
    .rodata : ALIGN(0x1000) {
        *(.rodata)
        *(.rodata.*)
    }
    .data : ALIGN(0x1000) {
        *(.data)
        *(.data.*)
    }
    .bss : ALIGN(0x1000) {
        *(.bss)
        *(.bss.*)
    }
    /DISCARD/ : {
        *(.eh_frame)
        *(.comment)
    }
}