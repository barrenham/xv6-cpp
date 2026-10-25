#include "kernel/syscall/syscall.h"
.text
.global signal_register_probe
signal_register_probe:
 addi sp, sp, -128
 sd ra, 0(sp)
 sd s0, 8(sp)
 sd s1, 16(sp)
 sd s2, 24(sp)
 sd s3, 32(sp)
 sd s4, 40(sp)
 sd s5, 48(sp)
 sd s6, 56(sp)
 sd s7, 64(sp)
 sd s8, 72(sp)
 sd s9, 80(sp)
 sd s10, 88(sp)
 sd s11, 96(sp)
 sd gp, 104(sp)
 sd tp, 112(sp)
 li s0, 30
 li s1, 31
 li s2, 32
 li s3, 33
 li s4, 34
 li s5, 35
 li s6, 36
 li s7, 37
 li s8, 38
 li s9, 39
 li s10, 40
 li s11, 41
 li gp, 42
 li tp, 43
 li t0, 11
 li t1, 12
 li t2, 13
 li t3, 14
 li t4, 15
 li t5, 16
 li t6, 17
 li a1, 10
 li a2, 22
 li a3, 23
 li a4, 24
 li a5, 25
 li a6, 26
 li a7, SYS_sigsend
 ecall
 bnez a0, 1f
 li a0, 11
 bne t0, a0, 1f
 li a0, 12
 bne t1, a0, 1f
 li a0, 13
 bne t2, a0, 1f
 li a0, 14
 bne t3, a0, 1f
 li a0, 15
 bne t4, a0, 1f
 li a0, 16
 bne t5, a0, 1f
 li a0, 17
 bne t6, a0, 1f
 li a0, 10
 bne a1, a0, 1f
 li a0, 22
 bne a2, a0, 1f
 li a0, 23
 bne a3, a0, 1f
 li a0, 24
 bne a4, a0, 1f
 li a0, 25
 bne a5, a0, 1f
 li a0, 26
 bne a6, a0, 1f
 li a0, SYS_sigsend
 bne a7, a0, 1f
 ld a0, 0(sp)
 bne ra, a0, 1f
 li a0, 30
 bne s0, a0, 1f
 li a0, 31
 bne s1, a0, 1f
 li a0, 32
 bne s2, a0, 1f
 li a0, 33
 bne s3, a0, 1f
 li a0, 34
 bne s4, a0, 1f
 li a0, 35
 bne s5, a0, 1f
 li a0, 36
 bne s6, a0, 1f
 li a0, 37
 bne s7, a0, 1f
 li a0, 38
 bne s8, a0, 1f
 li a0, 39
 bne s9, a0, 1f
 li a0, 40
 bne s10, a0, 1f
 li a0, 41
 bne s11, a0, 1f
 li a0, 42
 bne gp, a0, 1f
 li a0, 43
 bne tp, a0, 1f
 li a0, 0
 j 2f
1:
 li a0, -1
2:
 ld ra, 0(sp)
 ld s0, 8(sp)
 ld s1, 16(sp)
 ld s2, 24(sp)
 ld s3, 32(sp)
 ld s4, 40(sp)
 ld s5, 48(sp)
 ld s6, 56(sp)
 ld s7, 64(sp)
 ld s8, 72(sp)
 ld s9, 80(sp)
 ld s10, 88(sp)
 ld s11, 96(sp)
 ld gp, 104(sp)
 ld tp, 112(sp)
 addi sp, sp, 128
 ret

# An asynchronous handler may clobber all user registers; sigreturn restores them.
.global signal_register_handler
signal_register_handler:
 li gp, -1
 li tp, -1
 li s0, -1
 li s1, -1
 li s2, -1
 li s3, -1
 li s4, -1
 li s5, -1
 li s6, -1
 li s7, -1
 li s8, -1
 li s9, -1
 li s10, -1
 li s11, -1
 li t0, -1
 li t1, -1
 li t2, -1
 li t3, -1
 li t4, -1
 li t5, -1
 li t6, -1
 li a0, -1
 li a1, -1
 li a2, -1
 li a3, -1
 li a4, -1
 li a5, -1
 li a6, -1
 li a7, -1
 ret

# Malformed frame must never replace the trusted trapframe's kernel fields.
.global signal_corrupt_handler
signal_corrupt_handler:
 sd zero, 0(sp)
 ret
.global signal_badpc_handler
signal_badpc_handler:
 li t0, -1
 sd t0, 16(sp)
 ret
.global signal_badsp_handler
signal_badsp_handler:
 li t0, -1
 sd t0, 32(sp)
 ret
.global signal_stack_exhaust
signal_stack_exhaust:
 li sp, 0
 li a7, SYS_sigsend
 li a1, 10
 ecall
 ebreak
