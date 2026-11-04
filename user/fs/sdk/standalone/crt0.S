.section .text.entry
.global _user_start
_user_start:
  call main
  li a7, 2
  ecall
1:
  j 1b
