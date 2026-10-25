module;
#include "kernel/support/param.h"

module xv.process;
import :cpu;
import xv.sync;

namespace xv::process
{
int Process::pin_cpu(int index)
{
  if (index < 0 || index >= NCPU)
  {
    return -1;
  }
  // The optional lab binds via the scheduler's per-hart array.
  xv::sync::ScopedLock guard(lock);
  __atomic_store_n(&pincpu, cpu_at(index), __ATOMIC_RELEASE);
  return 0;
}

} // namespace xv::process
