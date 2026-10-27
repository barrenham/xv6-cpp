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
constinit ProcessGroups groups;
} // namespace

constinit ProcessManager process_manager(store, active_scheduler, waits, groups);

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
  groups.initialize();
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

int wait_for_child(int pid, unsigned long address, int options)
{
  return process_manager.waitpid(pid, address, options);
}

int kill(int pid)
{
  return process_manager.kill(pid);
}

int send_signal(int target, int signal)
{
  return process_manager.send_signal(target, signal);
}

void deliver_signals()
{
  process_manager.deliver_signals();
}

int set_signal_disposition(int signal, int action)
{
  return process_manager.set_signal_disposition(signal, action);
}

void post_terminal_interrupt(void *terminal, int signal)
{
  process_manager.post_terminal_interrupt(terminal, signal);
}

void service_signal_notifications()
{
  process_manager.service_signal_notifications();
}

void detach_terminal(void *terminal)
{
  process_manager.detach_terminal(terminal);
}

int claim_terminal(void *terminal)
{
  return process_manager.claim_terminal(terminal);
}

int terminal_foreground(void *terminal)
{
  return process_manager.terminal_foreground(terminal);
}

int set_terminal_foreground(void *terminal, int group)
{
  return process_manager.set_terminal_foreground(terminal, group);
}

bool may_read_terminal(void *terminal)
{
  return process_manager.may_read_terminal(terminal);
}

int get_process_group()
{
  return process_manager.get_process_group();
}

int set_process_group(int pid, int group)
{
  return process_manager.set_process_group(pid, group);
}

int get_session(int pid)
{
  return process_manager.get_session(pid);
}

int create_session()
{
  return process_manager.create_session();
}

void wakeup(void *channel)
{
  process_manager.wakeup(channel);
}

PollWait make_poll_wait()
{
  return PollWait(*current(), waits);
}

void poll_timer_tick(unsigned long milliseconds)
{
  waits.expire_timers(milliseconds);
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
bool snapshot_after(int after, ProcessInfo &result)
{
  return process_manager.snapshot_after(after, result);
}
int inspect_files(int pid, xv::fs::FileInfo *entries, unsigned capacity)
{ return process_manager.inspect_files(pid, entries, capacity); }
} // namespace xv::process
