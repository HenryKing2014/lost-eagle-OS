# boot.S - RISC-V (rv64) S-mode entry point for the QEMU 'virt' machine.
#
# QEMU loads OpenSBI (the default -bios) at 0x80000000, which runs in M-mode
# and then jumps to our kernel at 0x80200000 in S-mode with:
#   a0 = hartid of the boot hart
#   a1 = physical address of the device tree blob (FDT)

    .section .text.boot, "ax"
    .global _start
_start:
    # (QEMU 'virt' boots with -smp 1 by default, so usually only hart 0 runs.)
    mv      s0, a0              # save hartid
    mv      s1, a1              # save FDT pointer

    # Set up the stack (defined at the end of the BSS in linker.ld).
    la      sp, __stack_top

    # Zero the .bss section.
    la      t0, __bss_start
    la      t1, __bss_end
1:
    bgeu    t0, t1, 2f
    sd      zero, 0(t0)
    addi    t0, t0, 8
    j       1b
2:
    # Restore arguments and jump into C.
    mv      a0, s0              # hartid
    mv      a1, s1              # FDT pointer
    call    kmain

    # kmain should never return; if it does, halt.
halt:
    wfi
    j       halt
