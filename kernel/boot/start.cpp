module;

#include "kernel/support/types.h"
#include "kernel/arch/riscv/register_constants.h"

module xv.boot;

import xv.arch;

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
  xv::arch::w_menvcfg(xv::arch::r_menvcfg() | MENVCFG_STCE);
  xv::arch::w_mcounteren(xv::arch::r_mcounteren() | 2);
  xv::arch::w_stimecmp(xv::arch::r_time() + xv::trap::TimerInterval);
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
  xv::arch::record_cpu_hardware(unsigned(hart_id));
  if (hart_id == 0)
  {
    firmware_device_tree = device_tree;
  }

  // set M Previous Privilege mode to Supervisor, for mret.
  unsigned long x = xv::arch::r_mstatus();
  x &= ~MSTATUS_MPP_MASK;
  x |= MSTATUS_MPP_S;
  xv::arch::w_mstatus(x);

  // set M Exception Program Counter to run(), for mret.
  // requires gcc -mcmodel=medany
  xv::arch::w_mepc(reinterpret_cast<uint64>(&run));

  // disable paging for now.
  xv::arch::w_satp(0);

  // delegate all interrupts and exceptions to supervisor mode.
  xv::arch::w_medeleg(0xffff);
  xv::arch::w_mideleg(0xffff);
  xv::arch::w_sie(xv::arch::r_sie() | SIE_SEIE | SIE_STIE);

  // configure Physical Memory Protection to give supervisor mode
  // access to all of physical memory.
  xv::arch::w_pmpaddr0(0x3fffffffffffffull);
  xv::arch::w_pmpcfg0(0xf);

  // enable hardware updates of page table A and D bits
  xv::arch::w_menvcfg(xv::arch::r_menvcfg() | MENVCFG_ADUE);

  // ask for clock interrupts.
  initialize_timer();

  // keep each CPU's hartid in its tp register, for cpu_id().
  int id = xv::arch::r_mhartid();
  xv::arch::w_tp(id);

  // switch to supervisor mode and jump to run().
  asm volatile("mret" ::: "memory");
  __builtin_unreachable();
}

} // namespace xv::boot
