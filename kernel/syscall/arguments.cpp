module;

#include "kernel/support/types.h"

module xv.syscall;

import :arguments;
import xv.process;
import xv.print;

namespace xv::syscall
{
// Fetch the uint64 at addr from the current process.
int fetch_address(uint64 addr, uint64 *ip)
{
  xv::process::Process *p = xv::process::current();
  if (addr >= p->memory().size() ||
      addr + sizeof(uint64) >
          p->memory().size()) // both tests needed, in case of overflow
  {
    return -1;
  }
  if (p->memory().copy_in(ip, addr, sizeof(*ip)) != 0)
  {
    return -1;
  }
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int fetch_string(uint64 addr, char *buf, int max)
{
  xv::process::Process *p = xv::process::current();
  if (p->memory().copy_string(buf, addr, max) < 0)
  {
    return -1;
  }
  int length = 0;
  while (buf[length])
  {
    ++length;
  }
  return length;
}

static uint64 argument_raw(int n)
{
  xv::process::Process *p = xv::process::current();
  switch (n)
  {
  case 0:
    return p->registers()->a0;
  case 1:
    return p->registers()->a1;
  case 2:
    return p->registers()->a2;
  case 3:
    return p->registers()->a3;
  case 4:
    return p->registers()->a4;
  case 5:
    return p->registers()->a5;
  }
  xv::print::panic("argument_raw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
void argument_integer(int n, int *ip)
{
  *ip = argument_raw(n);
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
void argument_address(int n, uint64 *ip)
{
  *ip = argument_raw(n);
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (not including nul), -1 if error.
int argument_string(int n, char *buf, int max)
{
  uint64 addr;
  argument_address(n, &addr);
  return fetch_string(addr, buf, max);
}

} // namespace xv::syscall
