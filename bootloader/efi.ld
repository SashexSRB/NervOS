OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(efi_main)

/*
 * Define a common Image Base for UEFI applications.
 * This is the preferred virtual address where the image expects to be loaded.
 * objcopy will use this to populate the ImageBase in the PE/COFF header
 * and to calculate base relocations.
 */
BASE_ADDRESS = 0x10000; /* A typical base address for UEFI applications */

PHDRS
{
    /*
     * Define a single loadable program segment (PT_LOAD).
     * This segment must start at the BASE_ADDRESS and cover all the
     * loadable sections (text, rodata, data, bss, and linker-generated sections).
     * FLAGS(5) corresponds to PF_R (Read) | PF_X (Execute).
     */
    code PT_LOAD FLAGS(5) ;
}

SECTIONS
{
    /*
     * Start the first section (.text) at the BASE_ADDRESS.
     * All subsequent sections will follow consecutively in virtual memory.
     * By removing explicit AT() for other sections, we let the linker
     * automatically place them, including its own generated sections,
     * within the 'code' PT_LOAD segment, avoiding overlaps.
     */
    .text BASE_ADDRESS : {
        *(.text)
        *(.text.*)
    } : code /* Assign .text to the 'code' segment */

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } : code /* Assign .rodata to the 'code' segment */

    .data : {
        *(.data)
        *(.data.*)
    } : code /* Assign .data to the 'code' segment */

    .bss : {
        *(.bss)
        *(.bss.*)
    } : code /* Assign .bss to the 'code' segment */

    /*
     * The linker will automatically place its own required sections like
     * .interp, .dynstr, .hash, .gnu.hash, .dynamic, .dynsym, .rela.dyn
     * after the explicitly defined sections and before .bss, or in some logical order.
     * By removing explicit AT() for these, we allow it to find non-overlapping space.
     * Ensure they are still implicitly covered by the 'code' segment due to
     * its broad definition in PHDRS.
     */

    .dynamic : { /* Re-add dynamic and related sections for clarity, without AT() */
        *(.dynamic)
    } : code

    .dynsym : {
        *(.dynsym)
    } : code

    .rela.dyn : {
        *(.rela.dyn)
    } : code

    /* Discard sections that are not needed in the final executable. */
    /DISCARD/ : {
        *(.eh_frame) /* Exception handling information */
        *(.comment)  /* Compiler comment sections */
        /* It's also common to discard .interp and other dynamic sections if they
         * are truly not needed after objcopy, but for now, letting the linker
         * handle their placement is safer to avoid overlap errors.
         */
    }
}
