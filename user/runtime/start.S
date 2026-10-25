# User exec enters here with argc in a0 and argv in a1.
.section .text.entry
.global _user_start
_user_start:
  call main
  call exit
1:
  j 1b
