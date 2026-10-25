module;

#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"

extern "C" volatile int panicking;
extern "C" volatile int panicked;

module xv.drivers;

import xv.process;

import :platform;
import xv.memory;

namespace xv::drivers::detail
{
void prepare_sleep(void *channel)
{
  xv::process::prepare_sleep(channel);
}

void sleep_process()
{
  xv::process::sleep();
}

void wake_processes(void *channel)
{
  xv::process::wakeup(channel);
}

bool process_killed()
{
  return xv::process::current()->is_killed();
}

void dump_processes()
{
  xv::process::dump();
}

int copy_in(void *destination, bool user, unsigned long source, unsigned count)
{
  return xv::process::copy_in(destination, user, source, count);
}

int copy_out(bool user, unsigned long destination, void *source, unsigned count)
{
  return xv::process::copy_out(user, destination, source, count);
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
