module;
#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"

module xv.process;
import :table;

namespace xv::process
{
void initialize()
{
  process_table.initialize();
}

void map_kernel_stacks(PageTable table)
{
  process_table.map_kernel_stacks(table);
}

void create_initial_process()
{
  process_table.create_initial_process();
}

int fork_process()
{
  return process_table.fork_process();
}

[[noreturn]] void exit_process(int status)
{
  process_table.exit_process(status);
}

int wait(unsigned long address)
{
  return process_table.wait(address);
}

int kill(int pid)
{
  return process_table.kill(pid);
}

void wakeup(void *channel)
{
  process_table.wakeup(channel);
}

void dump()
{
  process_table.dump();
}

int copy_in(void *destination, bool user, unsigned long source, unsigned long count)
{
  if (user)
  {
    return current()->memory().copy_in(destination, source, count);
  }
  memmove(destination, reinterpret_cast<const void *>(source), count);
  return 0;
}

int copy_out(bool user, unsigned long destination, const void *source,
             unsigned long count)
{
  if (user)
  {
    return current()->memory().copy_out(destination, source, count);
  }
  memmove(reinterpret_cast<void *>(destination), source, count);
  return 0;
}
} // namespace xv::process
