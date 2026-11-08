#pragma once

// Only the entry functions and assembly labels cross the C/assembly ABI.
extern "C"
{
struct UserTrapReturn
{
  unsigned long page_table;
  unsigned long trap_frame;
};
UserTrapReturn usertrap();
void kerneltrap();
void kernelvec();
extern char trampoline[], uservec[], userret[];
}
