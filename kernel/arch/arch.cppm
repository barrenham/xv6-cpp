module;

#include "kernel/arch/riscv/register_constants.h"

export module xv.arch;

export namespace xv::arch
{
#include "kernel/arch/riscv/register_operations.inc"
struct CpuHardware
{
  unsigned hart=0;
  unsigned long isa=0,vendor=0,architecture=0,implementation=0;
};
// Capture machine-only CSRs before dropping to S-mode; publish online only
// after that hart completes its supervisor initialization.
void record_cpu_hardware(unsigned hart);
void mark_cpu_online(unsigned hart);
bool cpu_hardware(unsigned index,CpuHardware &result);
} // namespace xv::arch
