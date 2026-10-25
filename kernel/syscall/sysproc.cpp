#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/param.h"
#include "kernel/support/defs.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/sync/spinlock.h"
#include "kernel/process/proc.h"
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
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > UTOP)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  if (n < 0)
    n = 0;
  tickslock.acquire();
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      tickslock.release();
      return -1;
    }
    sleep_prepare(&ticks);
    tickslock.release();
    sleep();
    tickslock.acquire();
  }
  tickslock.release();
  return 0;
}


#ifdef LAB_PGTBL
int
sys_vmprint(void)
{
  struct proc *p;

  p = myproc();
  vmprint(p->pagetable);
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
  struct proc *p = myproc();

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
    pte_t *pte = walk(p->pagetable, base + (uint64)i * PGSIZE, 0);
    if (pte == 0 || (*pte & (PTE_V | PTE_U)) != (PTE_V | PTE_U) ||
        !PTE_LEAF(*pte))
      return -1;
  }

  // Reject unmapped/read-only output pages rather than allocating them.
  if (out >= MAXVA || (uint64)nbytes > MAXVA - out)
    return -1;
  for (uint64 va = PGROUNDDOWN(out); va < out + nbytes; va += PGSIZE) {
    pte_t *pte = walk(p->pagetable, va, 0);
    if (pte == 0 || (*pte & (PTE_V | PTE_U | PTE_W)) !=
                    (PTE_V | PTE_U | PTE_W))
      return -1;
  }
  // Superpages share one hardware A bit. Demote them so later samples
  // still detect accesses at the 4 KiB granularity promised by pgaccess.
  if (uvmdemote(p->pagetable, base, npages) < 0)
    return -1;
  for (int i = 0; i < npages; i++) {
    pte_t *pte = walk(p->pagetable, base + (uint64)i * PGSIZE, 0);
    if (*pte & PTE_A)
      bits[i / 8] |= 1U << (i % 8);
  }
  if (copyout(p->pagetable, p->sz, out, (char *)bits, nbytes) < 0)
    return -1;

  for (int i = 0; i < npages; i++) {
    pte_t *pte = walk(p->pagetable, base + (uint64)i * PGSIZE, 0);
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
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  tickslock.acquire();
  xticks = ticks;
  tickslock.release();
  return xticks;
}

#ifdef LAB_LOCK
uint64
sys_cpupin(void)
{
  struct proc *p = myproc();
  int cpu;

  argint(0, &cpu);
  if (cpu < 0 || cpu >= NCPU)
    return -1;
  p->lock.acquire();
  p->pincpu = &cpus[cpu];
  p->lock.release();
  return 0;
}
#endif
}
