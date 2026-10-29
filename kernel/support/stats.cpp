module;
#include "kernel/support/flow.h"

#include <stdarg.h>

#include "kernel/arch/riscv/register_constants.h"
#include "kernel/fs/format/layout.h"
#include "kernel/support/param.h"
#include "kernel/support/types.h"

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

int statswrite(int user_src, uint64 src, int n) { return -1; }

namespace flow = xv::flow;
struct StatsReadFlow
{
  struct Context
  {
    bool user;
    uint64 destination;
    int limit;
    int result = 0;
  };
  static bool empty(Context &) noexcept { return stats.sz == 0; }
  static void fill(Context &) noexcept
  {
#ifdef LAB_PGTBL
    stats.sz = statscopyin(stats.buf, BUFSZ);
#endif
#ifdef LAB_LOCK
    stats.sz = statslock(stats.buf, BUFSZ);
#endif
  }
  static void measure(Context &c) noexcept { c.result = stats.sz - stats.off; }
  static bool positive(Context &c) noexcept { return c.result > 0; }
  static void clamp(Context &c) noexcept
  {
    if (c.result > c.limit)
    {
      c.result = c.limit;
    }
  }
  static bool copied(Context &c) noexcept
  {
    return xv::process::copy_out(c.user, c.destination, stats.buf + stats.off,
                                 c.result) != -1;
  }
  static void advance(Context &c) noexcept { stats.off += c.result; }
  static void eof(Context &c) noexcept
  {
    c.result = -1;
    stats.sz = 0;
    stats.off = 0;
  }
  static void unlock(Context &) noexcept { stats.lock.release(); }
  struct Empty;
  struct Fill;
  struct Measure;
  struct Clamp;
  struct Copy;
  struct Advance;
  struct Eof;
  struct Unlock;
  using Graph = flow::graph<
      Empty, flow::types<flow::node<Empty, flow::noop,
                                    flow::branch<flow::condition<&empty>,
                                                 flow::go<Fill>, flow::go<Measure>>>,
                         flow::node<Fill, flow::action<&fill>, flow::go<Measure>>,
                         flow::node<Measure, flow::action<&measure>,
                                    flow::branch<flow::condition<&positive>,
                                                 flow::go<Clamp>, flow::go<Eof>>>,
                         flow::node<Clamp, flow::action<&clamp>, flow::go<Copy>>,
                         flow::node<Copy, flow::noop,
                                    flow::branch<flow::condition<&copied>,
                                                 flow::go<Advance>, flow::go<Unlock>>>,
                         flow::node<Advance, flow::action<&advance>, flow::go<Unlock>>,
                         flow::node<Eof, flow::action<&eof>, flow::go<Unlock>>,
                         flow::node<Unlock, flow::action<&unlock>, flow::stop>>>;
};

int statsread(int user_dst, uint64 dst, int n)
{
  stats.lock.acquire();
  StatsReadFlow::Context context{static_cast<bool>(user_dst), dst, n};
  (void)StatsReadFlow::Graph::run<0>(context);
  return context.result;
}

class StatsDevice final : public xv::drivers::CharacterDevice
{
public:
  constexpr StatsDevice() = default;

  const char *name() const override { return "statistics"; }

  void initialize() override { stats.lock.init("stats"); }

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