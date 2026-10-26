module;
#include "kernel/support/types.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.fs;

import xv.print;

import xv.runtime;
import :platform;
import xv.process;
import xv.memory;
import xv.trap;

namespace xv::fs::detail
{
bool process_after(int after, ProcessRecord &result)
{
  xv::process::ProcessInfo value;
  if (!xv::process::snapshot_after(after, value)) return false;
  result.pid = value.pid; result.parent = value.parent;
  result.group = value.group; result.session = value.session;
  result.bytes = value.bytes; result.state = value.state;
  xv::runtime::move(result.name, value.name, sizeof(result.name));
  return true;
}
unsigned long uptime_ticks() { return xv::trap::clock().uptime(); }
void memory_info(unsigned long &total_kib, unsigned long &free_kib)
{
  auto &allocator = xv::memory::allocator();
  total_kib = 0;
  for (unsigned i = 0; i < allocator.layout().usable_count(); ++i)
    total_kib += allocator.layout().usable(i).page_count() * 4;
  free_kib = allocator.free_pages() * 4;
}
void prepare_interruptible_sleep(void *channel)
{
  xv::process::prepare_interruptible_sleep(channel);
}

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
  xv::runtime::move(destination, source, count);
}

void fill(void *destination, int value, unsigned count)
{
  xv::runtime::fill(destination, value, count);
}

int compare(const char *first, const char *second, unsigned count)
{
  return xv::runtime::compare_text(first, second, count);
}

void copy_fixed(char *destination, const char *source, unsigned count)
{
  xv::runtime::copy_fixed(destination, source, count);
}

void report(const char *message, int first, int second)
{
  xv::print::printk(message, first, second);
}

[[noreturn]] void fail(const char *message)
{
  xv::print::panic(message);
}
} // namespace xv::fs::detail
