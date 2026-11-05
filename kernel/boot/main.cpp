module;
#include "kernel/hardware/device_tree.h"
module xv.boot;

import :state;
import xv.print;
import xv.trap;
import xv.fs;
import xv.process;
import xv.drivers;
import xv.memory;
import xv.arch;

extern "C" [[noreturn]]
void enter_scheduler_stack(unsigned long top);

extern "C" [[noreturn]]
void
scheduler_on_stack() {
  xv::process::scheduler().run();
}

namespace xv::boot
{
namespace
{
constinit int started = 0;

void
initialize_bootstrap_hart() {
  if (!xv::hardware::initialize(reinterpret_cast<const void *>(detail::device_tree())))
  {
    xv::print::panic("devices: invalid or unsupported device tree");
  }
  xv::drivers::initialize_console();
  xv::print::initialize();
  xv::print::printk("\nxv6 kernel is booting\n\n");

  xv::memory::boot_initialize(xv::memory::configured_allocator_kind(),
                              xv::memory::PhysicalAddress(detail::device_tree()));
  xv::print::printk("physical allocator: %s\n", xv::memory::allocator().name());
  xv::memory::kernel_page_table().initialize();
  xv::memory::kernel_page_table().activate_hart();

  xv::process::initialize();
  xv::trap::initialize();
  xv::trap::initialize_hart();
  xv::drivers::initialize_interrupts();
  xv::drivers::initialize_hart(xv::process::cpu_id());

  xv::fs::initialize_cache();
  xv::fs::filesystem().initialize_inodes();
  xv::fs::files().initialize();
  xv::drivers::block_device().initialize();
  xv::process::create_initial_process();
}

void
initialize_secondary_hart() {
  // Pair with the bootstrap hart's release to observe initialized shared state.
  while (__atomic_load_n(&started, __ATOMIC_ACQUIRE) == 0)
  {
    asm volatile("nop");
  }

  xv::print::printk("hart %d starting\n", xv::process::cpu_id());
  xv::memory::kernel_page_table().activate_hart();
  xv::trap::initialize_hart();
  xv::drivers::initialize_hart(xv::process::cpu_id());
}
} // namespace

[[noreturn]]
void
run() {
  if (xv::process::cpu_id() == 0)
  {
    initialize_bootstrap_hart();
    __atomic_store_n(&started, 1, __ATOMIC_RELEASE);
  }
  else
  {
    initialize_secondary_hart();
  }

  xv::arch::mark_cpu_online(unsigned(xv::process::cpu_id()));
  enter_scheduler_stack(xv::trap::scheduler_stack_top());
}
} // namespace xv::boot
