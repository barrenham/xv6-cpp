module;

#include "kernel/support/types.h"
#include "kernel/arch/riscv/riscv.h"

module xv.boot;

import :state;
import xv.trap;

namespace xv::boot
{
namespace
{
constinit unsigned long firmware_device_tree = 0;

void initialize_timer()
{
  // Permit supervisor-mode timer programming through the Sstc extension.
  w_menvcfg(r_menvcfg() | MENVCFG_STCE);
  w_mcounteren(r_mcounteren() | 2);
  w_stimecmp(r_time() + xv::trap::TimerInterval);
}
} // namespace

namespace detail
{
unsigned long device_tree()
{
  return firmware_device_tree;
}
} // namespace detail

[[noreturn]] void enter(unsigned long hart_id, unsigned long device_tree)
{
  if (hart_id == 0)
  {
    firmware_device_tree = device_tree;
  }

  // set M Previous Privilege mode to Supervisor, for mret.
  unsigned long x = r_mstatus();
  x &= ~MSTATUS_MPP_MASK;
  x |= MSTATUS_MPP_S;
  w_mstatus(x);

  // set M Exception Program Counter to run(), for mret.
  // requires gcc -mcmodel=medany
  w_mepc(reinterpret_cast<uint64>(&run));

  // disable paging for now.
  w_satp(0);

  // delegate all interrupts and exceptions to supervisor mode.
  w_medeleg(0xffff);
  w_mideleg(0xffff);
  w_sie(r_sie() | SIE_SEIE | SIE_STIE);

  // configure Physical Memory Protection to give supervisor mode
  // access to all of physical memory.
  w_pmpaddr0(0x3fffffffffffffull);
  w_pmpcfg0(0xf);

  // enable hardware updates of page table A and D bits
  w_menvcfg(r_menvcfg() | MENVCFG_ADUE);

  // ask for clock interrupts.
  initialize_timer();

  // keep each CPU's hartid in its tp register, for cpu_id().
  int id = r_mhartid();
  w_tp(id);

  // switch to supervisor mode and jump to run().
  asm volatile("mret" ::: "memory");
  __builtin_unreachable();
}

} // namespace xv::boot
