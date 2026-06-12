/* linker.ld - memory layout for the kernel on the QEMU 'virt' machine.
 *
 * OpenSBI occupies the start of DRAM (0x80000000) and hands control to the
 * kernel at 0x80200000, so that is where we link our image.
 */

OUTPUT_ARCH("riscv")
ENTRY(_start)

/* Load address handed to us by OpenSBI on the QEMU 'virt' machine. */
BASE_ADDRESS = 0x80200000;

SECTIONS
{
    . = BASE_ADDRESS;

    .text : ALIGN(8) {
        KEEP(*(.text.boot))     /* _start must be first */
        *(.text .text.*)
    }

    .rodata : ALIGN(8) {
        *(.rodata .rodata.*)
        *(.srodata .srodata.*)
    }

    .data : ALIGN(8) {
        *(.data .data.*)
        *(.sdata .sdata.*)
    }

    .bss (NOLOAD) : ALIGN(8) {
        __bss_start = .;
        *(.sbss .sbss.*)
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end = .;
    }

    /* 64 KiB boot stack, placed right after BSS. */
    . = ALIGN(16);
    . = . + 0x10000;
    __stack_top = .;

    /DISCARD/ : {
        *(.comment)
        *(.note .note.*)
        *(.eh_frame .eh_frame_hdr)
        *(.riscv.attributes)
    }
}
