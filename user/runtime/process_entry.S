.section .text
.globl xv_user_enter
.type xv_user_enter, @function
xv_user_enter:
  mv sp, a0
  andi sp, sp, -16
  mv t0, a1
  mv a0, a2
  li s0, 0
  li ra, 0
  jr t0
.size xv_user_enter, .-xv_user_enter
