module;
#include "kernel/support/types.h"
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/process/scheduler/policies/scheduling_policy.h"

module xv.process;

import xv.runtime;
import xv.print;
import :manager;
import :paged_store;
import :fair_policy;
import :round_robin;
import :mlfq_policy;
import :scheduler_runtime;
import :wait_channels;

namespace xv::process
{
namespace
{
constinit PagedProcessStore store;
constinit RoundRobinPolicy round_robin_ready;
constinit FairPolicy fair_ready;
constinit MlfqPolicy feedback_ready;
constinit SchedulerRuntime active_scheduler(fair_ready);
int scheduler_initialized = 0;
SchedulerKind selected_kind = static_cast<SchedulerKind>(XV_DEFAULT_SCHEDULER);
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

SchedulerKind configured_scheduler_kind()
{
  return __atomic_load_n(&scheduler_initialized, __ATOMIC_ACQUIRE) == 2
             ? selected_kind : static_cast<SchedulerKind>(XV_DEFAULT_SCHEDULER);
}

bool initialize(SchedulerKind kind)
{
  ReadyQueue *ready = nullptr;
  switch (kind)
  {
  case SchedulerKind::Fair: ready = &fair_ready; break;
  case SchedulerKind::RoundRobin: ready = &round_robin_ready; break;
  case SchedulerKind::Feedback: ready = &feedback_ready; break;
  default: return false;
  }
  int expected = 0;
  if (!__atomic_compare_exchange_n(&scheduler_initialized, &expected, 1, false,
                                    __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
    return false;
  active_scheduler.select(*ready);
  groups.initialize();
  store.initialize();
  ready->initialize();
  xv::print::printk("scheduler: %s\n", active_scheduler.name());
  waits.initialize();
  process_manager.initialize();
  selected_kind = kind;
  __atomic_store_n(&scheduler_initialized, 2, __ATOMIC_RELEASE);
  return true;
}

void initialize()
{
  if (!initialize(static_cast<SchedulerKind>(XV_DEFAULT_SCHEDULER)))
    xv::print::panic("scheduler: invalid or repeated initialization");
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
