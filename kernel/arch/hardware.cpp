module;
#include "kernel/support/param.h"
module xv.arch;

namespace xv::arch
{
namespace
{
struct Record {CpuHardware hardware;bool recorded=false,online=false;};
constinit Record records[NCPU]{};
}
void record_cpu_hardware(unsigned hart)
{
  if(hart>=NCPU)return;
  auto &record=records[hart];record.hardware.hart=hart;
  asm volatile("csrr %0, misa":"=r"(record.hardware.isa));
  asm volatile("csrr %0, mvendorid":"=r"(record.hardware.vendor));
  asm volatile("csrr %0, marchid":"=r"(record.hardware.architecture));
  asm volatile("csrr %0, mimpid":"=r"(record.hardware.implementation));
  __atomic_store_n(&record.recorded,true,__ATOMIC_RELEASE);
}
void mark_cpu_online(unsigned hart)
{
  if(hart<NCPU && __atomic_load_n(&records[hart].recorded,__ATOMIC_ACQUIRE))
    __atomic_store_n(&records[hart].online,true,__ATOMIC_RELEASE);
}
bool cpu_hardware(unsigned index,CpuHardware &result)
{
  if(index>=NCPU || !__atomic_load_n(&records[index].online,__ATOMIC_ACQUIRE))return false;
  result=records[index].hardware;return true;
}
}
