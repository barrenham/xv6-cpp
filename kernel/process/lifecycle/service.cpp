module;
#include "kernel/support/types.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.process;

import xv.runtime;
import :manager;
import :paged_store;
import :fifo_queue;
import :round_robin;
import :wait_channels;

namespace xv::process
{
namespace
{
constinit PagedProcessStore store;
constinit FifoReadyQueue ready;
constinit RoundRobinScheduler active_scheduler(ready);
constinit WaitChannels waits(active_scheduler);
} // namespace

constinit ProcessManager process_manager(store, active_scheduler, waits);

WaitChannels &wait_channels()
{
  return waits;
}

Scheduler &scheduler()
{
  return active_scheduler;
}

void initialize()
{
  store.initialize();
  ready.initialize();
  waits.initialize();
  process_manager.initialize();
}

void create_initial_process()
{
  process_manager.create_initial_process();
}

int fork_process()
{
  return process_manager.fork_process();
}

[[noreturn]] void exit_process(int status)
{
  process_manager.exit_process(status);
}

int wait(unsigned long address)
{
  return process_manager.wait(address);
}

int kill(int pid)
{
  return process_manager.kill(pid);
}

void wakeup(void *channel)
{
  process_manager.wakeup(channel);
}

void dump()
{
  process_manager.dump();
}

int copy_in(void *destination, bool user, unsigned long source, unsigned long count)
{
  if (user)
  {
    return current()->memory().copy_in(destination, source, count);
  }
  xv::runtime::move(destination, reinterpret_cast<const void *>(source), count);
  return 0;
}

int copy_out(bool user, unsigned long destination, const void *source,
             unsigned long count)
{
  if (user)
  {
    return current()->memory().copy_out(destination, source, count);
  }
  xv::runtime::move(reinterpret_cast<void *>(destination), source, count);
  return 0;
}
} // namespace xv::process
