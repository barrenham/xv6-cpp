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
  return va < MAXVA ? xv::memory::walk_entry(pagetable, va, 0) : nullptr;
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

uint64 sys_ksupernpte()
{
  xv::process::Process *p;
  uint64 as, an;
  auto counts = xv::memory::kernel_page_table().statistics();
  uint64 s = counts.leaves_1g + counts.leaves_2m, n = counts.leaves_4k;

  p = xv::process::current();
  argument_address(0, &as);
  argument_address(1, &an);

  if (xv::memory::copy_to_user(p->memory().page_table(), p->memory().size(), p->memory().regions(), as,
                               (char *)&s, sizeof(uint64)) < 0)
  {
    return -1;
  }
  if (xv::memory::copy_to_user(p->memory().page_table(), p->memory().size(), p->memory().regions(), an,
                               (char *)&n, sizeof(uint64)) < 0)
  {
    return -1;
  }

  return 0;
}
#endif
} // namespace xv::syscall
