module;

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
uint64 sys_pgaccess(void)
{
  // A bounded byte array supports up to 4096 pages without large stack use.
  enum
  {
    MAXPAGES = 4096
  };

  uchar bits[MAXPAGES / 8];
  uint64 base, out;
  int npages;
  xv::process::Process *p = xv::process::current();

  argument_address(0, &base);
  argument_integer(1, &npages);
  argument_address(2, &out);
  if (npages <= 0 || npages > MAXPAGES || base >= MAXVA)
  {
    return -1;
  }
  base = PGROUNDDOWN(base);
  if ((uint64)npages > (MAXVA - base) / PGSIZE)
  {
    return -1;
  }

  int nbytes = (npages + 7) / 8;
  xv::runtime::fill(bits, 0, nbytes);
  // Validate the whole range before changing any access bits.
  for (int i = 0; i < npages; i++)
  {
    pte_t *pte =
        xv::memory::walk_entry(p->memory().page_table(), base + (uint64)i * PGSIZE, 0);
    if (pte == 0 || (*pte & (PTE_V | PTE_U)) != (PTE_V | PTE_U) || !PTE_LEAF(*pte))
    {
      return -1;
    }
  }

  // Reject unmapped/read-only output pages rather than allocating them.
  if (out >= MAXVA || (uint64)nbytes > MAXVA - out)
  {
    return -1;
  }
  for (uint64 va = PGROUNDDOWN(out); va < out + nbytes; va += PGSIZE)
  {
    pte_t *pte = xv::memory::walk_entry(p->memory().page_table(), va, 0);
    if (pte == 0 || (*pte & (PTE_V | PTE_U | PTE_W)) != (PTE_V | PTE_U | PTE_W))
    {
      return -1;
    }
  }
  // Superpages share one hardware A bit. Demote them so later samples
  // still detect accesses at the 4 KiB granularity promised by pgaccess.
  if (xv::memory::demote_user_pages(p->memory().page_table(), base, npages) < 0)
  {
    return -1;
  }
  for (int i = 0; i < npages; i++)
  {
    pte_t *pte =
        xv::memory::walk_entry(p->memory().page_table(), base + (uint64)i * PGSIZE, 0);
    if (*pte & PTE_A)
    {
      bits[i / 8] |= 1U << (i % 8);
    }
  }
  if (xv::memory::copy_to_user(p->memory().page_table(), p->memory().size(), out,
                               (char *)bits, nbytes) < 0)
  {
    return -1;
  }

  for (int i = 0; i < npages; i++)
  {
    pte_t *pte =
        xv::memory::walk_entry(p->memory().page_table(), base + (uint64)i * PGSIZE, 0);
    *pte &= ~PTE_A;
  }
  // Invalidate cached translations so future accesses set PTE_A again.
  xv::arch::sfence_vma();
  return 0;
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
} // namespace xv::syscall
