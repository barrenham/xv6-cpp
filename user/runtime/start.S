# User exec enters here with argc in a0 and argv in a1.
.section .text.entry
.global _user_start
_user_start:
  addi sp, sp, -32
  sd a0, 0(sp)
  sd a1, 8(sp)
  sd a2, 16(sp)
  mv a0, a2
  call __environment_init
  ld a0, 0(sp)
  ld a1, 8(sp)
  ld a2, 16(sp)
  addi sp, sp, 32
  call main
  call exit
1:
  j 1b
