module;
#include "kernel/support/param.h"

module xv.process;

import :cpu;
import xv.arch;
import xv.sync;

namespace xv::process
{
namespace
{
constinit CpuState cpus[NCPU]{};
} // namespace

// CPU identity/current_cpu require interrupts off; current() brackets its lookup.
int cpu_id()
{
  int id = xv::arch::r_tp();
  return id;
}

CpuState *current_cpu(void)
{
  int id = cpu_id();
  CpuState *c = &cpus[id];
  return c;
}

Process *current(void)
{
  xv::sync::push_interrupts_off();
  CpuState *c = current_cpu();
  Process *p = c->current;
  xv::sync::pop_interrupts_off();
  return p;
}

CpuState *cpu_at(int index)
{
  return &cpus[index];
}

} // namespace xv::process
