OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(efi_main)
SECTIONS
{
    . = 0x1000;
    .text : {
        *(.text)
        *(.text.*)
    } :text =0x90
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } :rodata
    .data : {
        *(.data)
        *(.data.*)
    } :data
    .bss : {
        *(.bss)
        *(.bss.*)
    } :data
    .dynamic : {
        *(.dynamic)
    } :data
    .dynsym : {
        *(.dynsym)
    } :data
    .rel : {
        *(.rel.*)
    } :data
    .rela : {
        *(.rela.*)
    } :data
    /DISCARD/ : {
        *(.eh_frame)
        *(.comment)
    }
}
PHDRS
{
    text PT_LOAD FLAGS(5); /* PF_R | PF_X */
    rodata PT_LOAD FLAGS(4); /* PF_R */
    data PT_LOAD FLAGS(6); /* PF_R | PF_W */
}