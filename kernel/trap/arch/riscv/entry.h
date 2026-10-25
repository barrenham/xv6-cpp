#pragma once

// Only the entry functions and assembly labels cross the C/assembly ABI.
extern "C"
{
unsigned long usertrap();
void kerneltrap();
void kernelvec();
extern char trampoline[], uservec[], userret[];
}
