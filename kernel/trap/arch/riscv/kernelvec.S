
.macro restore_prepare
        ld t0, 272(sp)
        csrw sepc, t0
        ld t0, 280(sp)
        csrw sstatus, t0
        ld t0, 256(sp)
        csrw scause, t0
        ld t0, 264(sp)
        csrw stval, t0
        addi t0, sp, 320
        csrw sscratch, t0
        ld ra, 0(sp)
        ld gp, 16(sp)
        ld tp, 24(sp)
        ld t0, 32(sp)
        ld t1, 40(sp)
        ld t2, 48(sp)
        ld a0, 72(sp)
        ld a1, 80(sp)
        ld a2, 88(sp)
        ld a3, 96(sp)
        ld a4, 104(sp)
        ld a5, 112(sp)
        ld a6, 120(sp)
        ld a7, 128(sp)
        ld t3, 216(sp)
        ld t4, 224(sp)
        ld t5, 232(sp)
        ld t6, 240(sp)
        ld sp, 8(sp)
.endm
        #
        # interrupts and exceptions while in supervisor
        # mode come here.
        #
        # the current stack is a kernel stack.
        # push registers, call kerneltrap().
        # when kerneltrap() returns, restore registers, return.
        #
.globl kerneltrap
.globl kernelvec
.align 4
kernelvec:
        csrrw sp, sscratch, sp
        addi sp, sp, -320
        sd ra, 0(sp)
        sd gp, 16(sp)
        sd tp, 24(sp)
        sd t0, 32(sp)
        sd t1, 40(sp)
        sd t2, 48(sp)
        sd a0, 72(sp)
        sd a1, 80(sp)
        sd a2, 88(sp)
        sd a3, 96(sp)
        sd a4, 104(sp)
        sd a5, 112(sp)
        sd a6, 120(sp)
        sd a7, 128(sp)
        sd t3, 216(sp)
        sd t4, 224(sp)
        sd t5, 232(sp)
        sd t6, 240(sp)
        csrr a0, sscratch
        sd a0, 8(sp)
        csrr a1, scause
        sd a1, 256(sp)
        csrr a2, stval
        sd a2, 264(sp)
        csrr t0, sepc
        sd t0, 272(sp)
        csrr t0, sstatus
        sd t0, 280(sp)
        li t0, 8192
        sub t0, sp, t0
        csrw sscratch, t0
        call kernel_stack_prepare
        beqz a0, .Lregular_entry
        restore_prepare
        sret
.Lregular_entry:
        restore_prepare

        # make room to save registers.
        addi sp, sp, -256

        # save caller-saved registers.
        sd ra, 0(sp)
        # sd sp, 8(sp)
        sd gp, 16(sp)
        # sd tp, 24(sp)
        sd t0, 32(sp)
        sd t1, 40(sp)
        sd t2, 48(sp)
        sd a0, 72(sp)
        sd a1, 80(sp)
        sd a2, 88(sp)
        sd a3, 96(sp)
        sd a4, 104(sp)
        sd a5, 112(sp)
        sd a6, 120(sp)
        sd a7, 128(sp)
        sd t3, 216(sp)
        sd t4, 224(sp)
        sd t5, 232(sp)
        sd t6, 240(sp)

        # call the thin C ABI trap entry
        call kerneltrap

        # restore registers.
        ld ra, 0(sp)
        # ld sp, 8(sp)
        ld gp, 16(sp)
        # not tp (contains hartid), in case we moved CPUs
        ld t0, 32(sp)
        ld t1, 40(sp)
        ld t2, 48(sp)
        ld a0, 72(sp)
        ld a1, 80(sp)
        ld a2, 88(sp)
        ld a3, 96(sp)
        ld a4, 104(sp)
        ld a5, 112(sp)
        ld a6, 120(sp)
        ld a7, 128(sp)
        ld t3, 216(sp)
        ld t4, 224(sp)
        ld t5, 232(sp)
        ld t6, 240(sp)

        addi sp, sp, 256

        # return to whatever we were doing in the kernel.
        sret
