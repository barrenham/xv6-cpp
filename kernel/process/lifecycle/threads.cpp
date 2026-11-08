module;
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/io/posix_errno.h"
#include "kernel/process/signals/signal.h"
module xv.process;
import :manager;
import xv.memory;
import xv.runtime;
import xv.print;
import xv.ipc;

namespace xv::process
{
void
Process::request_group_exit(int status, int signal) {
  if (!resources_ || !__atomic_load_n(&resources_->threaded, __ATOMIC_ACQUIRE))
  {
    return;
  }
  unsigned long expected = 0;
  auto value = (static_cast<unsigned long>(static_cast<unsigned>(status)) << 16) |
               (static_cast<unsigned long>(signal & 127) << 1) | 1UL;
  (void)__atomic_compare_exchange_n(&resources_->exit_request, &expected, value, false,
                                    __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
}

long
ProcessManager::create_thread(unsigned long entry, unsigned long argument,
                              unsigned long stack, unsigned long tls) {
  auto &parent = *current();
  if (!entry || stack < 16 || stack >= MAXVA || stack % 16)
  {
    return -EINVAL;
  }
  auto executable =
      parent.memory().handle_fault(entry, xv::memory::FaultAccess::Execute).result;
  if (executable != xv::memory::FaultResult::Resolved &&
      executable != xv::memory::FaultResult::AlreadyMapped)
  {
    return -EFAULT;
  }
  unsigned long empty[2]{};
  if (parent.memory().copy_out(stack - sizeof(empty), empty, sizeof(empty)) < 0)
  {
    return -EFAULT;
  }
  auto &resources = *parent.resources_;
  unsigned slot = 1;
  wait_lock_.acquire();
  while (slot < THREAD_FRAME_SLOTS && (resources.slots & (1U << slot)))
  {
    ++slot;
  }
  if (slot == THREAD_FRAME_SLOTS || parent.group_exit_requested() ||
      resources.exec_owner)
  {
    wait_lock_.release();
    return -EAGAIN;
  }
  resources.slots |= 1U << slot;
  ++resources.creating;
  __atomic_add_fetch(&resources.references, 1U, __ATOMIC_RELAXED);
  __atomic_store_n(&resources.threaded, true, __ATOMIC_RELEASE);
  wait_lock_.release();

  auto *child = allocate();
  if (!child)
  {
    wait_lock_.acquire();
    resources.slots &= ~(1U << slot);
    --resources.creating;
    wakeup(&resources);
    __atomic_sub_fetch(&resources.references, 1U, __ATOMIC_ACQ_REL);
    wait_lock_.release();
    return -EAGAIN;
  }
  // A thread keeps its own registers/stack, and adopts a retained group resource
  // reference. Its unpublished bootstrap address space has no FS callbacks.
  auto *private_resources = child->resources_;
  private_resources->memory.reset();
  if (private_resources->user_info)
  {
    xv::memory::allocator().deallocate(private_resources->user_info);
  }
  private_resources->~ProcessResources();
  xv::memory::allocator().deallocate(private_resources);
  child->resources_ = &resources;
  child->thread_slot_ = slot;
  child->pending_signals_.share_dispositions(parent.pending_signals_);
  child->exec_snapshot_.inherit(parent.exec_snapshot_);
  child->filesystem_worker_ = parent.filesystem_worker_;
  xv::runtime::copy_string(child->name, parent.name, sizeof(child->name));
  *child->trapframe = *parent.trapframe;
  child->trapframe->epc = entry;
  child->trapframe->sp = stack;
  child->trapframe->tp = tls;
  child->trapframe->ra = ~0UL;
  child->trapframe->a0 = argument;
  child->trapframe->a1 = 0;
  child->lock.release();

  resources.memory.enable_concurrency();
  bool mapped = resources.memory.map_thread_frame(slot, child->trapframe);
  wait_lock_.acquire();
  if (!mapped || parent.group_exit_requested() || resources.exec_owner)
  {
    wait_lock_.release();
    if (mapped && !resources.memory.unmap_thread_frame(slot, child->trapframe))
    {
      xv::print::panic("thread: rollback frame");
    }
    wait_lock_.acquire();
    child->lock.acquire();
    reap(child);
    --resources.creating;
    wakeup(&resources);
    wait_lock_.release();
    return -EAGAIN;
  }
  child->lock.acquire();
  resources.threads[slot] = child;
  --resources.creating;
  wakeup(&resources);
  ++resources.live;
  const int tid = child->pid;
  scheduler_.make_runnable(*child);
  child->lock.release();
  wait_lock_.release();
  return tid;
}

int
ProcessManager::signal_thread(int tid, int signal) {
  if (signal && !supported_signal(signal))
  {
    return -EINVAL;
  }
  auto &caller = *current();
  wait_lock_.acquire();
  Process *target = nullptr;
  for (auto *thread : caller.resources_->threads)
  {
    if (thread && thread->pid == tid)
    {
      target = thread;
      break;
    }
  }
  if (!target)
  {
    wait_lock_.release();
    return -ESRCH;
  }
  target->lock.acquire();
  bool alive = target->state != ProcessState::Unused &&
               target->state != ProcessState::Zombie &&
               target->state != ProcessState::ThreadZombie;
  if (alive && signal)
  {
    if (signal == SIGKILL)
    {
      target->request_group_exit(SIGNAL_EXIT_STATUS(SIGKILL), SIGKILL);
    }
    if (signal == SIGSTOP)
    {
      target->pending_signals_.post_process(signal);
    }
    else
    {
      // Continue resumes the group; its caught handler still targets this task.
      if (signal == SIGCONT)
      {
        target->pending_signals_.post_process(signal, false);
      }
      target->pending_signals_.post(signal);
    }
  }
  target->lock.release();
  wait_lock_.release();
  if (alive && signal)
  {
    wake_signal_waiters();
  }
  return alive ? 0 : -ESRCH;
}

int
ProcessManager::join_thread(int tid, unsigned long result_address) {
  auto &caller = *current();
  auto &resources = *caller.resources_;
  if (tid == caller.pid)
  {
    return -EDEADLK;
  }
  wait_lock_.acquire();
  Process *target = nullptr;
  for (auto *thread : resources.threads)
  {
    if (thread && thread->pid == tid)
    {
      target = thread;
      break;
    }
  }
  if (!target || target->thread_joined_ ||
      (target->thread_joiner_ && target->thread_joiner_ != caller.pid))
  {
    wait_lock_.release();
    return target ? -EINVAL : -ESRCH;
  }
  // Both join ownership and the wait-for chain are protected by the family lock.
  auto *link = target;
  for (unsigned n = 0; n < THREAD_FRAME_SLOTS && link->joining_tid_; ++n)
  {
    if (link->joining_tid_ == caller.pid)
    {
      wait_lock_.release();
      return -EDEADLK;
    }
    Process *next = nullptr;
    for (auto *thread : resources.threads)
    {
      if (thread && thread->pid == link->joining_tid_)
      {
        next = thread;
        break;
      }
    }
    if (!next)
    {
      break;
    }
    link = next;
  }
  target->thread_joiner_ = caller.pid;
  caller.joining_tid_ = tid;
  unsigned long value = 0;
  for (;;)
  {
    target->lock.acquire();
    bool done = target->state == ProcessState::ThreadZombie;
    value = target->thread_result_;
    target->lock.release();
    if (done)
    {
      break;
    }
    if (caller.is_killed())
    {
      target->thread_joiner_ = 0;
      caller.joining_tid_ = 0;
      wait_lock_.release();
      return -EINTR;
    }
    prepare_interruptible_sleep(&resources);
    wait_lock_.release();
    sleep();
    wait_lock_.acquire();
  }
  wait_lock_.release();
  // No family or target lock may span a user copy or translation quiescence.
  if (result_address &&
      caller.memory().copy_out(result_address, &value, sizeof(value)) < 0)
  {
    wait_lock_.acquire();
    target->thread_joiner_ = 0;
    caller.joining_tid_ = 0;
    wait_lock_.release();
    return -EFAULT;
  }
  if (target != resources.leader &&
      !resources.memory.unmap_thread_frame(target->thread_slot_, target->trapframe))
  {
    xv::print::panic("thread: joined frame");
  }
  wait_lock_.acquire();
  caller.joining_tid_ = 0;
  target->thread_joined_ = true;
  target->thread_joiner_ = 0;
  if (target != resources.leader)
  {
    target->lock.acquire();
    reap(target);
  }
  wait_lock_.release();
  return 0;
}

void
ProcessManager::report_group_stop(Process &source) {
  if (source.group_exit_requested())
  {
    return;
  }
  auto &resources = *source.resources_;
  if (!resources.live ||
      __atomic_load_n(&resources.stopped, __ATOMIC_ACQUIRE) != resources.live ||
      !source.pending_signals_.group_stop_signal())
  {
    return;
  }
  bool expected = false;
  if (!__atomic_compare_exchange_n(&resources.stop_reported, &expected, true, false,
                                   __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
  {
    return;
  }
  auto &leader = *resources.leader;
  if (&leader != &source)
  {
    leader.lock.acquire();
  }
  auto signal = source.pending_signals_.group_stop_signal();
  if (signal)
  {
    leader.stop_signal_ = signal;
    leader.stop_event_ = true;
    leader.continued_event_ = false;
    leader.continue_notification_ = true;
    queue_child_notification(leader);
  }
  else
  {
    __atomic_store_n(&resources.stop_reported, false, __ATOMIC_RELEASE);
  }
  if (&leader != &source)
  {
    leader.lock.release();
  }
}

[[noreturn]]
void
ProcessManager::exit_thread(unsigned long result) {
  auto &thread = *current();
  auto &resources = *thread.resources_;
  if (resources.leader == initial_)
  {
    xv::print::panic("init thread exiting");
  }
  if (thread.pending_signals_.deactivate())
  {
    wake_signal_waiters(false);
  }
  waits_.cancel(thread);
  thread.clear_file_pins();
  wait_lock_.acquire();
  __atomic_store_n(&resources.threaded, true, __ATOMIC_RELEASE);
  if (!resources.live)
  {
    xv::print::panic("thread: live count");
  }
  const bool last = !--resources.live;
  thread.thread_result_ = result;
  if (!last)
  {
    thread.lock.acquire();
    thread.state = ProcessState::ThreadZombie;
    report_group_stop(thread);
    wait_lock_.release();
    wakeup(&resources);
    scheduler_.suspend();
    xv::print::panic("thread: zombie returned");
  }
  wait_lock_.release();

  auto &leader = *resources.leader;
  groups_.exit_terminal(leader.membership_);
  resources.memory.close_file_mappings();
  resources.descriptors.close_all();
  resources.directory.clear();
  xv::ipc::collect_deferred_socket_cycles();

  wait_lock_.acquire();
  reparent(&leader);
  if (leader.parent)
  {
    wakeup(&leader.parent->child_events_);
  }
  auto request = __atomic_load_n(&resources.exit_request, __ATOMIC_ACQUIRE);
  if (&thread != &leader)
  {
    leader.lock.acquire();
  }
  thread.lock.acquire();
  leader.xstate = request ? static_cast<int>(static_cast<unsigned>(request >> 16)) : 0;
  leader.exit_signal_ = request ? (request >> 1) & 127 : 0;
  if (&thread != &leader)
  {
    thread.state = ProcessState::ThreadZombie;
  }
  leader.state = ProcessState::Zombie;
  post_child_signal(leader);
  if (&thread != &leader)
  {
    leader.lock.release();
  }
  wait_lock_.release();
  scheduler_.suspend();
  xv::print::panic("thread: last exit returned");
}

void
ProcessManager::reap_group_workers(Process &leader) {
  auto *resources = leader.resources_;
  if (!resources || !__atomic_load_n(&resources->threaded, __ATOMIC_ACQUIRE))
  {
    return;
  }
  for (unsigned slot = 1; slot < THREAD_FRAME_SLOTS; ++slot)
  {
    auto *thread = resources->threads[slot];
    if (!thread)
    {
      continue;
    }
    thread->lock.acquire();
    if (thread->state != ProcessState::ThreadZombie)
    {
      xv::print::panic("thread: reap live worker");
    }
    if (!resources->memory.unmap_thread_frame(slot, thread->trapframe))
    {
      xv::print::panic("thread: reaped frame");
    }
    reap(thread);
  }
}

bool
Process::prepare_exec() {
  return process_manager.prepare_exec();
}

void
Process::finish_exec() {
  process_manager.finish_exec();
}

bool
ProcessManager::prepare_exec() {
  auto &caller = *current();
  auto &resources = *caller.resources_;
  wait_lock_.acquire();
  if (resources.exec_owner || caller.group_exit_requested())
  {
    wait_lock_.release();
    return false;
  }
  __atomic_store_n(&resources.exec_owner, caller.pid, __ATOMIC_RELEASE);
  wait_lock_.release();
  wake_signal_waiters(false);
  wait_lock_.acquire();
  while (resources.live != 1 || resources.creating)
  {
    if (caller.group_exit_requested())
    {
      __atomic_store_n(&resources.exec_owner, 0, __ATOMIC_RELEASE);
      wait_lock_.release();
      return false;
    }
    prepare_interruptible_sleep(&resources);
    wait_lock_.release();
    sleep();
    wait_lock_.acquire();
  }
  wait_lock_.release();
  // live reaches one before a sibling switches off its kernel stack. Taking
  // that sibling's lock below also waits for the scheduler to finish switching.
  for (unsigned slot = 1; slot < THREAD_FRAME_SLOTS; ++slot)
  {
    auto *thread = resources.threads[slot];
    if (!thread || thread == &caller)
    {
      continue;
    }
    thread->lock.acquire();
    if (thread->state != ProcessState::ThreadZombie)
    {
      xv::print::panic("exec: live sibling");
    }
    thread->lock.release();
    if (!resources.memory.unmap_thread_frame(slot, thread->trapframe))
    {
      xv::print::panic("exec: sibling frame");
    }
    wait_lock_.acquire();
    thread->lock.acquire();
    reap(thread);
    wait_lock_.release();
  }
  if (resources.leader != &caller)
  {
    xv::sync::ScopedLock guard(resources.leader->lock);
    if (resources.leader->state != ProcessState::ThreadZombie)
    {
      xv::print::panic("exec: live leader");
    }
    resources.leader->thread_joined_ = true;
    resources.leader->thread_joiner_ = 0;
  }
  caller.thread_joiner_ = caller.joining_tid_ = 0;
  return true;
}

void
ProcessManager::finish_exec() {
  auto &caller = *current();
  auto &leader = caller.group_leader();
  if (&leader != &caller)
  {
    xv::sync::ScopedLock guard(leader.lock);
    leader.exec_snapshot_.inherit(caller.exec_snapshot_);
    xv::runtime::copy_string(leader.name, caller.name, sizeof(leader.name));
  }
  __atomic_store_n(&caller.resources_->exec_owner, 0, __ATOMIC_RELEASE);
}
} // namespace xv::process
