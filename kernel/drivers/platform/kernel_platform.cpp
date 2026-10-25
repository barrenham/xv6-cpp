module;

#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"

extern "C" volatile int panicking;
extern "C" volatile int panicked;

module xv.drivers;

import :platform;
import xv.memory;

namespace xv::drivers::detail
{
void prepare_sleep(void *channel)
{
  sleep_prepare(channel);
}

void sleep_process()
{
  sleep();
}

void wake_processes(void *channel)
{
  wakeup(channel);
}

bool process_killed()
{
  return killed(myproc());
}

void dump_processes()
{
  procdump();
}

int copy_in(void *destination, bool user, unsigned long source, unsigned count)
{
  return either_copyin(destination, user, source, count);
}

int copy_out(bool user, unsigned long destination, void *source, unsigned count)
{
  return either_copyout(user, destination, source, count);
}

void push_interrupts_off()
{
  push_off();
}

void pop_interrupts_off()
{
  pop_off();
}

bool panicking()
{
  return ::panicking != 0;
}

bool panicked()
{
  return ::panicked != 0;
}

void io_barrier()
{
  io_fence();
}

void *allocate_dma_page()
{
  return xv::memory::allocator().allocate();
}

void zero_page(void *page)
{
  memset(page, 0, xv::memory::PageSize);
}

[[noreturn]] void fail(const char *message)
{
  panic(const_cast<char *>(message));
}
} // namespace xv::drivers::detail
