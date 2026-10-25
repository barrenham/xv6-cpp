module;
#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"

module xv.fs;
import :platform;
import xv.process;
import xv.memory;

namespace xv::fs::detail
{
void prepare_sleep(void *channel)
{
  xv::process::prepare_sleep(channel);
}

void sleep()
{
  xv::process::sleep();
}

void wakeup(void *channel)
{
  xv::process::wakeup(channel);
}

bool killed()
{
  return xv::process::current()->is_killed();
}

Inode *current_directory()
{
  return xv::process::current()->directory().duplicate();
}

int copy_in(void *destination, bool user, unsigned long source, unsigned count)
{
  return xv::process::copy_in(destination, user, source, count);
}

int copy_out(bool user, unsigned long destination, const void *source, unsigned count)
{
  return xv::process::copy_out(user, destination, source, count);
}

void *allocate_page()
{
  return xv::memory::allocator().allocate();
}

void free_page(void *page)
{
  xv::memory::allocator().deallocate(page);
}

void move(void *destination, const void *source, unsigned count)
{
  memmove(destination, source, count);
}

void fill(void *destination, int value, unsigned count)
{
  memset(destination, value, count);
}

int compare(const char *first, const char *second, unsigned count)
{
  return strncmp(first, second, count);
}

void copy_fixed(char *destination, const char *source, unsigned count)
{
  strncpy(destination, source, count);
}

void report(const char *message, int first, int second)
{
  printk(const_cast<char *>(message), first, second);
}

[[noreturn]] void fail(const char *message)
{
  panic(const_cast<char *>(message));
}
} // namespace xv::fs::detail
