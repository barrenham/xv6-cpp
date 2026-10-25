        # qemu -kernel loads the kernel at 0x80000000
        # and causes each hart (i.e. CPU) to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.
.section .text.entry
.global _entry
_entry:
        # set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + ((hartid + 1) * 4096)
        la sp, stack0
        li t0, 1024*4
        csrr a0, mhartid
        addi t1, a0, 1
        mul t0, t0, t1
        add sp, sp, t0
        # a0 is hartid; preserve firmware a1 (device tree address).
        # jump to start() in start.c
        call start
spin:
        j spin
