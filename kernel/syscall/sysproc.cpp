import xv.trap;

import xv.process;

import xv.sync;

#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/param.h"
#include "kernel/support/defs.h"
#include "kernel/memory/layout/memlayout.h"
#ifdef PGTBL_SOL
#include "kernel/arch/riscv/riscv.h"
#endif
#include "kernel/memory/paging/vm.h"

extern "C" {

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  xv::process::exit_process(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return xv::process::current()->id();
}

uint64
sys_fork(void)
{
  return xv::process::fork_process();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return xv::process::wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = xv::process::current()->memory().size();

  if (xv::process::current()->memory().resize(n, t == SBRK_EAGER || n < 0) < 0)
    return -1;
  return addr;
}

uint64
sys_pause(void)
{
  int count;
  argint(0, &count);
  return xv::trap::clock().pause(count);
}


#ifdef LAB_PGTBL
int
sys_vmprint(void)
{
  xv::process::Process *p;

  p = xv::process::current();
  vmprint(p->memory().page_table());
  return 0;
}
#endif

#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // A bounded byte array supports up to 4096 pages without large stack use.
  enum { MAXPAGES = 4096 };
  uchar bits[MAXPAGES / 8];
  uint64 base, out;
  int npages;
  xv::process::Process *p = xv::process::current();

  argaddr(0, &base);
  argint(1, &npages);
  argaddr(2, &out);
  if (npages <= 0 || npages > MAXPAGES || base >= MAXVA)
    return -1;
  base = PGROUNDDOWN(base);
  if ((uint64)npages > (MAXVA - base) / PGSIZE)
    return -1;

  int nbytes = (npages + 7) / 8;
  memset(bits, 0, nbytes);
  // Validate the whole range before changing any access bits.
  for (int i = 0; i < npages; i++) {
    pte_t *pte = walk(p->memory().page_table(), base + (uint64)i * PGSIZE, 0);
    if (pte == 0 || (*pte & (PTE_V | PTE_U)) != (PTE_V | PTE_U) ||
        !PTE_LEAF(*pte))
      return -1;
  }

  // Reject unmapped/read-only output pages rather than allocating them.
  if (out >= MAXVA || (uint64)nbytes > MAXVA - out)
    return -1;
  for (uint64 va = PGROUNDDOWN(out); va < out + nbytes; va += PGSIZE) {
    pte_t *pte = walk(p->memory().page_table(), va, 0);
    if (pte == 0 || (*pte & (PTE_V | PTE_U | PTE_W)) !=
                    (PTE_V | PTE_U | PTE_W))
      return -1;
  }
  // Superpages share one hardware A bit. Demote them so later samples
  // still detect accesses at the 4 KiB granularity promised by pgaccess.
  if (uvmdemote(p->memory().page_table(), base, npages) < 0)
    return -1;
  for (int i = 0; i < npages; i++) {
    pte_t *pte = walk(p->memory().page_table(), base + (uint64)i * PGSIZE, 0);
    if (*pte & PTE_A)
      bits[i / 8] |= 1U << (i % 8);
  }
  if (copyout(p->memory().page_table(), p->memory().size(), out, (char *)bits, nbytes) < 0)
    return -1;

  for (int i = 0; i < npages; i++) {
    pte_t *pte = walk(p->memory().page_table(), base + (uint64)i * PGSIZE, 0);
    *pte &= ~PTE_A;
  }
  // Invalidate cached translations so future accesses set PTE_A again.
  sfence_vma();
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return xv::process::kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  return xv::trap::clock().uptime();
}

#ifdef LAB_LOCK
uint64
sys_cpupin(void)
{
  xv::process::Process *p = xv::process::current();
  int cpu;

  argint(0, &cpu);
  return p->pin_cpu(cpu);
}
#endif
}
