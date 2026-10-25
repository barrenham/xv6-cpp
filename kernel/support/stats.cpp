module;

#include <stdarg.h>

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.drivers;

import xv.memory;
import xv.fs;
import xv.process;
import xv.sync;

namespace xv::drivers::detail
{

//
// This file is for grading scripts.  You shouldn't have to look at code in this file.
//

#ifdef LAB_LOCK

#define BUFSZ 4096

static constinit struct
{
  xv::sync::SpinLock lock;
  char buf[BUFSZ];
  int sz;
  int off;
} stats{};

int statscopyin(char *, int);
int statslock(char *, int);

int statswrite(int user_src, uint64 src, int n)
{
  return -1;
}

int statsread(int user_dst, uint64 dst, int n)
{
  int m;

  stats.lock.acquire();

  if (stats.sz == 0)
  {
#ifdef LAB_PGTBL
    stats.sz = statscopyin(stats.buf, BUFSZ);
#endif
#ifdef LAB_LOCK
    stats.sz = statslock(stats.buf, BUFSZ);
#endif
  }
  m = stats.sz - stats.off;

  if (m > 0)
  {
    if (m > n)
    {
      m = n;
    }
    if (xv::process::copy_out(user_dst, dst, stats.buf + stats.off, m) != -1)
    {
      stats.off += m;
    }
  }
  else
  {
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
    return statswrite(source.space == xv::drivers::AddressSpace::User, source.address,
                      count);
  }

  void put_sync(int) override
  {
    // The statistics device has no polling output operation.
  }
};

static constinit StatsDevice stats_device;

void statsinit(void)
{
  stats_device.initialize();
  xv::fs::devices().attach(STATS, stats_device);
}

#endif

} // namespace xv::drivers::detail