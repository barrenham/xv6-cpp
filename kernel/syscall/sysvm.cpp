module;

#include "kernel/support/types.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.syscall;

import :arguments;
import :handlers;
import xv.memory;
import xv.process;

namespace xv::syscall
{
#ifdef LAB_PGTBL

static pte_t *pgpte(pagetable_t pagetable, uint64 va)
{
  return xv::memory::walk_entry(pagetable, va, 0);
}

uint64 sys_pgpte(void)
{
  uint64 va;
  xv::process::Process *p;

  p = xv::process::current();
  argument_address(0, &va);
  pte_t *pte = pgpte(p->memory().page_table(), va);
  if (pte != 0)
  {
    return (uint64)*pte;
  }
  return 0;
}

static void cnt_level(pagetable_t pagetable, uint64 *nsuper, uint64 *n2,
                      uint64 va_start, int level)
{
  uint64 va;
  int n = (level == 3) ? 1 : 512;

  for (int i = 0; i < n; i++)
  {
    pte_t pte = __atomic_load_n(&pagetable[i], __ATOMIC_ACQUIRE);
    if (pte & PTE_V)
    {
      va = (((uint64)i) << PXSHIFT(level)) + va_start;
      uint64 child = PTE2PA(pte);
      if (level > 0)
      {
        if (!PTE_LEAF(pte))
        {
          cnt_level((pagetable_t)child, nsuper, n2, va, level - 1);
        }
        else
        {
          *nsuper += 1;
        }
      }
      else
      {
        *n2 += 1;
      }
    }
  }
}

uint64 sys_ksupernpte()
{
  xv::process::Process *p;
  uint64 as, an;
  uint64 s = 0, n = 0;

  cnt_level((pagetable_t)xv::memory::kernel_page_table().root(), &s, &n, 0, 2);

  p = xv::process::current();
  argument_address(0, &as);
  argument_address(1, &an);

  if (xv::memory::copy_to_user(p->memory().page_table(), p->memory().size(), as,
                               (char *)&s, sizeof(uint64)) < 0)
  {
    return -1;
  }
  if (xv::memory::copy_to_user(p->memory().page_table(), p->memory().size(), an,
                               (char *)&n, sizeof(uint64)) < 0)
  {
    return -1;
  }

  return 0;
}
#endif
} // namespace xv::syscall
