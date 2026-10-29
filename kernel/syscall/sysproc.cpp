module;
#include "kernel/support/flow.h"

#include "kernel/support/types.h"
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/memory/paging/vm.h"

module xv.syscall;

import xv.memory;

import xv.arch;

import xv.runtime;

import :arguments;
import :handlers;
import xv.trap;
import xv.process;
import xv.sync;

namespace xv::syscall
{

uint64 sys_exit(void)
{
  int n;
  argument_integer(0, &n);
  xv::process::exit_process(n);
  return 0; // not reached
}

uint64 sys_getpid(void)
{
  return xv::process::current()->id();
}

uint64 sys_sigaction()
{
  int signal;
  uint64 action, old, restorer;
  argument_integer(0, &signal);
  argument_address(1, &action);
  argument_address(2, &old);
  argument_address(3, &restorer);
  return xv::process::current()->signal_action(signal, action, old, restorer);
}

uint64 sys_sigprocmask()
{
  int how;
  uint64 set, old;
  argument_integer(0, &how);
  argument_address(1, &set);
  argument_address(2, &old);
  return xv::process::current()->signal_mask(how, set, old);
}

uint64 sys_sigpending()
{
  uint64 set;
  argument_address(0, &set);
  return xv::process::current()->signal_pending(set);
}

uint64 sys_sigsuspend()
{
  uint64 set;
  argument_address(0, &set);
  return xv::process::current()->signal_suspend(set);
}

uint64 sys_sigreturn()
{
  return xv::process::current()->signal_return();
}

uint64 sys_sigdisposition(void)
{
  int signal, action;
  argument_integer(0, &signal);
  argument_integer(1, &action);
  return xv::process::set_signal_disposition(signal, action);
}

uint64 sys_sigsend(void)
{
  int target, signal;
  argument_integer(0, &target);
  argument_integer(1, &signal);
  return xv::process::send_signal(target, signal);
}

uint64 sys_getpgrp(void)
{
  return xv::process::get_process_group();
}

uint64 sys_setpgid(void)
{
  int pid, group;
  argument_integer(0, &pid);
  argument_integer(1, &group);
  return xv::process::set_process_group(pid, group);
}

uint64 sys_getsid(void)
{
  int pid;
  argument_integer(0, &pid);
  return xv::process::get_session(pid);
}

uint64 sys_setsid(void)
{
  return xv::process::create_session();
}

uint64 sys_fork(void)
{
  return xv::process::fork_process();
}

uint64 sys_waitpid(void)
{
  int pid, options;
  uint64 address;
  argument_integer(0, &pid);
  argument_address(1, &address);
  argument_integer(2, &options);
  return xv::process::wait_for_child(pid, address, options);
}

uint64 sys_wait(void)
{
  uint64 p;
  argument_address(0, &p);
  return xv::process::wait(p);
}

uint64 sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argument_integer(0, &n);
  argument_integer(1, &t);
  addr = xv::process::current()->memory().size();

  if (xv::process::current()->memory().resize(n, t == SBRK_EAGER || n < 0) < 0)
  {
    return -1;
  }
  return addr;
}

uint64 sys_pause(void)
{
  int count;
  argument_integer(0, &count);
  return xv::trap::clock().pause(count);
}

#ifdef LAB_PGTBL
uint64 sys_vmprint(void)
{
  xv::process::Process *p;

  p = xv::process::current();
  xv::memory::print_page_table(p->memory().page_table());
  return 0;
}
#endif

#ifdef LAB_PGTBL
namespace
{
namespace flow = xv::flow;
struct PgAccessFlow
{
  enum
  {
    MAXPAGES = 4096
  };
  struct Context
  {
    xv::process::Process *process;
    uchar *bits;
    uint64 base, out;
    int npages;
    int nbytes = 0;
    uint64 result = static_cast<uint64>(-1);
  };
  static bool bounds(Context &c) noexcept
  {
    if (c.npages <= 0 || c.npages > MAXPAGES || c.base >= MAXVA)
    {
      return false;
    }
    c.base = PGROUNDDOWN(c.base);
    if ((uint64)c.npages > (MAXVA - c.base) / PGSIZE)
    {
      return false;
    }

    return true;
  }
  static void prepare(Context &c) noexcept
  {
    c.nbytes = (c.npages + 7) / 8;
    xv::runtime::fill(c.bits, 0, c.nbytes);
  }
  static bool sources(Context &c) noexcept
  {
    // Validate the whole range before changing any access bits.
    for (int i = 0; i < c.npages; i++)
    {
      pte_t *pte = xv::memory::walk_entry(c.process->memory().page_table(),
                                          c.base + (uint64)i * PGSIZE, 0);
      if (pte == 0 || (*pte & (PTE_V | PTE_U)) != (PTE_V | PTE_U) || !PTE_LEAF(*pte))
      {
        return false;
      }
    }

    return true;
  }
  static bool output(Context &c) noexcept
  {
    // Reject unmapped/read-only output pages; COW is logically writable.
    if (c.out >= MAXVA || (uint64)c.nbytes > MAXVA - c.out)
    {
      return false;
    }
    for (uint64 va = PGROUNDDOWN(c.out); va < c.out + c.nbytes; va += PGSIZE)
    {
      pte_t *pte = xv::memory::walk_entry(c.process->memory().page_table(), va, 0);
      if (pte == 0 || (*pte & (PTE_V | PTE_U)) != (PTE_V | PTE_U) ||
          !(*pte & (PTE_W | PTE_COW)))
      {
        return false;
      }
    }

    return true;
  }
  static bool demote(Context &c) noexcept
  {
    // Superpages share one hardware A bit. Demote them so later samples
    // still detect accesses at the 4 KiB granularity promised by pgaccess.
    if (xv::memory::demote_user_pages(c.process->memory().page_table(), c.base,
                                      c.npages) < 0)
    {
      return false;
    }

    return true;
  }
  static void sample(Context &c) noexcept
  {
    for (int i = 0; i < c.npages; i++)
    {
      pte_t *pte = xv::memory::walk_entry(c.process->memory().page_table(),
                                          c.base + (uint64)i * PGSIZE, 0);
      if (*pte & PTE_A)
      {
        c.bits[i / 8] |= 1U << (i % 8);
      }
    }
  }
  static bool copy_result(Context &c) noexcept
  {
    if (xv::memory::copy_to_user(
            c.process->memory().page_table(), c.process->memory().size(),
            c.process->memory().regions(), c.out, (char *)c.bits, c.nbytes) < 0)
    {
      return false;
    }

    return true;
  }
  static void clear(Context &c) noexcept
  {
    for (int i = 0; i < c.npages; i++)
    {
      pte_t *pte = xv::memory::walk_entry(c.process->memory().page_table(),
                                          c.base + (uint64)i * PGSIZE, 0);
      *pte &= ~PTE_A;
    }
    // Invalidate cached translations so future accesses set PTE_A again.
    xv::arch::sfence_vma();
    c.result = 0;
  }
  struct Bounds;
  struct Prepare;
  struct Sources;
  struct Output;
  struct Demote;
  struct Sample;
  struct CopyResult;
  struct Clear;
  using Graph = flow::graph<
      Bounds,
      flow::types<
          flow::node<
              Bounds, flow::noop,
              flow::branch<flow::condition<&bounds>, flow::go<Prepare>, flow::stop>>,
          flow::node<Prepare, flow::action<&prepare>, flow::go<Sources>>,
          flow::node<
              Sources, flow::noop,
              flow::branch<flow::condition<&sources>, flow::go<Output>, flow::stop>>,
          flow::node<
              Output, flow::noop,
              flow::branch<flow::condition<&output>, flow::go<Demote>, flow::stop>>,
          flow::node<
              Demote, flow::noop,
              flow::branch<flow::condition<&demote>, flow::go<Sample>, flow::stop>>,
          flow::node<Sample, flow::action<&sample>, flow::go<CopyResult>>,
          flow::node<
              CopyResult, flow::noop,
              flow::branch<flow::condition<&copy_result>, flow::go<Clear>, flow::stop>>,
          flow::node<Clear, flow::action<&clear>, flow::stop>>>;
};

} // namespace

uint64 sys_pgaccess(void)
{
  enum
  {
    MAXPAGES = 4096
  };
  uchar bits[MAXPAGES / 8];
  uint64 base, out;
  int npages;
  auto *process = xv::process::current();
  argument_address(0, &base);
  argument_integer(1, &npages);
  argument_address(2, &out);
  PgAccessFlow::Context context{process, bits, base, out, npages};
  (void)PgAccessFlow::Graph::run<0>(context);
  return context.result;
}
#endif

uint64 sys_kill(void)
{
  int pid;

  argument_integer(0, &pid);
  return xv::process::kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64 sys_uptime(void)
{
  return xv::trap::clock().uptime();
}

#ifdef LAB_LOCK
uint64 sys_cpupin(void)
{
  xv::process::Process *p = xv::process::current();
  int cpu;

  argument_integer(0, &cpu);
  return p->pin_cpu(cpu);
}
#endif
uint64 sys_getuid() { return xv::process::current()->credentials().snapshot().uid; }
uint64 sys_geteuid() { return xv::process::current()->credentials().snapshot().effective_uid; }
uint64 sys_getgid() { return xv::process::current()->credentials().snapshot().gid; }
uint64 sys_getegid() { return xv::process::current()->credentials().snapshot().effective_gid; }
uint64 sys_setuid() { int value; argument_integer(0, &value); return xv::process::current()->credentials().set_uid(value); }
uint64 sys_seteuid() { int value; argument_integer(0, &value); return xv::process::current()->credentials().set_uid(value, true); }
uint64 sys_setgid() { int value; argument_integer(0, &value); return xv::process::current()->credentials().set_gid(value); }
uint64 sys_setegid() { int value; argument_integer(0, &value); return xv::process::current()->credentials().set_gid(value, true); }
uint64 sys_umask() { int mask; argument_integer(0, &mask); return xv::process::current()->credentials().set_mask(mask); }
namespace
{
namespace flow = xv::flow;
struct GetGroupsFlow
{
  struct Context
  {
    int count;
    uint64 address;
    const xv::process::CredentialState &state;
    uint64 result = static_cast<uint64>(-1);
  };
  static bool query(Context &c) noexcept { return c.count == 0; }
  static bool capacity(Context &c) noexcept
  {
    return c.count >= 0 && static_cast<unsigned>(c.count) >= c.state.group_count;
  }
  static bool copy(Context &c) noexcept
  {
    return !c.state.group_count ||
           xv::process::current()->memory().copy_out(
               c.address, c.state.groups, c.state.group_count * sizeof(unsigned)) >= 0;
  }
  static void success(Context &c) noexcept { c.result = c.state.group_count; }
  struct Query;
  struct Capacity;
  struct Copy;
  struct Success;
  using Graph = flow::graph<
      Query,
      flow::types<flow::node<Query, flow::noop,
                             flow::branch<flow::condition<&query>, flow::go<Success>,
                                          flow::go<Capacity>>>,
                  flow::node<Capacity, flow::noop,
                             flow::branch<flow::condition<&capacity>, flow::go<Copy>,
                                          flow::stop>>,
                  flow::node<Copy, flow::noop,
                             flow::branch<flow::condition<&copy>, flow::go<Success>,
                                          flow::stop>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};
struct SetGroupsFlow
{
  struct Context
  {
    int count;
    uint64 address;
    unsigned *groups;
    uint64 result = static_cast<uint64>(-1);
  };
  static bool capacity(Context &c) noexcept { return c.count >= 0 && c.count <= 16; }
  static bool copy(Context &c) noexcept
  {
    return !c.count || xv::process::current()->memory().copy_in(
                           c.groups, c.address, c.count * sizeof(unsigned)) >= 0;
  }
  static void apply(Context &c) noexcept
  {
    c.result = xv::process::current()->credentials().set_groups(c.groups, c.count);
  }
  struct Capacity;
  struct Copy;
  struct Apply;
  using Graph = flow::graph<
      Capacity,
      flow::types<
          flow::node<
              Capacity, flow::noop,
              flow::branch<flow::condition<&capacity>, flow::go<Copy>, flow::stop>>,
          flow::node<Copy, flow::noop,
                     flow::branch<flow::condition<&copy>, flow::go<Apply>, flow::stop>>,
          flow::node<Apply, flow::action<&apply>, flow::stop>>>;
};

} // namespace

uint64 sys_getgroups()
{
  int count;
  uint64 address;
  argument_integer(0, &count);
  argument_address(1, &address);
  auto state = xv::process::current()->credentials().snapshot();
  GetGroupsFlow::Context context{count, address, state};
  (void)GetGroupsFlow::Graph::run<0>(context);
  return context.result;
}
uint64 sys_setgroups()
{
  int count;
  uint64 address;
  argument_integer(0, &count);
  argument_address(1, &address);
  unsigned groups[16];
  SetGroupsFlow::Context context{count, address, groups};
  (void)SetGroupsFlow::Graph::run<0>(context);
  return context.result;
}
uint64 sys_clockticks()
{ uint64 value; asm volatile("rdtime %0" : "=r"(value)); return value; }
} // namespace xv::syscall
