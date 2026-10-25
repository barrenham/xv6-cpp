import xv.memory;

import xv.drivers;

import xv.fs;

import xv.process;

import xv.sync;

#include <stdarg.h>

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"

extern "C" {

//
// This file is for grading scripts.  You shouldn't have to look at code in this file.
//

#ifdef LAB_LOCK

#define BUFSZ 4096
static constinit struct {
  xv::sync::SpinLock lock;
  char buf[BUFSZ];
  int sz;
  int off;
} stats{};

int statscopyin(char *, int);
int statslock(char *, int);

int
statswrite(int user_src, uint64 src, int n)
{
  return -1;
}

int
statsread(int user_dst, uint64 dst, int n)
{
  int m;

  stats.lock.acquire();

  if (stats.sz == 0) {
#ifdef LAB_PGTBL
    stats.sz = statscopyin(stats.buf, BUFSZ);
#endif
#ifdef LAB_LOCK
    stats.sz = statslock(stats.buf, BUFSZ);
#endif
  }
  m = stats.sz - stats.off;

  if (m > 0) {
    if (m > n)
      m = n;
    if (xv::process::copy_out(user_dst, dst, stats.buf + stats.off, m) != -1) {
      stats.off += m;
    }
  } else {
    m = -1;
    stats.sz = 0;
    stats.off = 0;
  }
  stats.lock.release();
  return m;
}

class StatsDevice final : public xv::drivers::CharacterDevice
{
public:
  constexpr StatsDevice() = default;
  const char *name() const override
  {
    return "statistics";
  }
  void initialize() override
  {
    stats.lock.init("stats");
  }
  int read(xv::drivers::IoBuffer destination, int count) override
  {
    return statsread(destination.space == xv::drivers::AddressSpace::User,
                     destination.address, count);
  }
  int write(xv::drivers::IoBuffer source, int count) override
  {
    return statswrite(source.space == xv::drivers::AddressSpace::User,
                      source.address, count);
  }
  void put_sync(int) override
  {
    // The statistics device has no polling output operation.
  }
};
static constinit StatsDevice stats_device;

void
statsinit(void)
{
  stats_device.initialize();
  xv::fs::devices().attach(STATS, stats_device);
}

#endif

#ifdef LAB_PGTBL


pte_t *
pgpte(pagetable_t pagetable, uint64 va)
{
  return walk(pagetable, va, 0);
}

uint64
sys_pgpte(void)
{
  uint64 va;
  xv::process::Process *p;

  p = xv::process::current();
  argaddr(0, &va);
  pte_t *pte = pgpte(p->memory().page_table(), va);
  if (pte != 0) {
    return (uint64)*pte;
  }
  return 0;
}


void
cnt_level(pagetable_t pagetable, uint64 *nsuper, uint64 *n2, uint64 va_start, int level)
{
  uint64 va;
  int n = (level == 3) ? 1 : 512;

  for (int i = 0; i < n; i++) {
    pte_t pte = pagetable[i];
    if (pte & PTE_V) {
      va = (((uint64)i) << PXSHIFT(level)) + va_start;
      uint64 child = PTE2PA(pte);
      if (level > 0) {
        if (!PTE_LEAF(pte)) {
          cnt_level((pagetable_t)child, nsuper, n2, va, level-1);
        } else {
          *nsuper += 1;
        }
      } else {
        *n2 += 1;
      }
    }
  }
}

uint64
sys_ksupernpte() {
  xv::process::Process *p;
  uint64 as, an;
  uint64 s = 0, n = 0;

  cnt_level((pagetable_t)xv::memory::kernel_page_table().root(), &s, &n, 0, 2);

  p = xv::process::current();
  argaddr(0, &as);
  argaddr(1, &an);

  if (copyout(p->memory().page_table(), p->memory().size(), as, (char *)&s, sizeof(uint64)) < 0) {
    return -1;
  }
  if (copyout(p->memory().page_table(), p->memory().size(), an, (char *)&n, sizeof(uint64)) < 0) {
    return -1;
  }

  return 0;
}
#endif
}
