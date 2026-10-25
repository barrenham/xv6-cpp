module xv.process:cpu;

import :process;

namespace xv::process
{
// Internal CPU lookup used by affinity; current_cpu/current remain public APIs.
CpuState *cpu_at(int index);
} // namespace xv::process
