module xv.boot;

import :state;
import xv.print;
import xv.trap;
import xv.fs;
import xv.process;
import xv.drivers;
import xv.memory;
import xv.arch;
import xv.net;

extern "C" [[noreturn]] void enter_scheduler_stack(unsigned long top);
extern "C" [[noreturn]] void scheduler_on_stack()
{
  xv::process::scheduler().run();
}

namespace xv::boot
{
namespace
{
constinit int started = 0;

void initialize_bootstrap_hart()
{
  xv::drivers::initialize_console();
  xv::print::initialize();
  xv::print::printk("\nxv6 kernel is booting\n\n");

  xv::memory::boot_initialize(
      xv::memory::configured_allocator_kind(),
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
  xv::net::initialize();
  xv::print::printk("e1000: %s\n", xv::drivers::network_device().online() ? "ready" : "not present");
  for (unsigned i = 0; i < xv::net::interfaces().count(); ++i)
  {
    auto *interface = xv::net::interfaces().at(i);
    xv::print::printk("net: %s mtu=%d %s\n", interface->name(), interface->mtu(),
                      interface->up() && interface->carrier() ? "up" : "down");
    for (unsigned n = 0; n < interface->address_count(); ++n)
    {
      auto address = interface->address(n);
      xv::print::printk("net: %s %d.%d.%d.%d/%d\n", interface->name(),
          address.address>>24, (address.address>>16)&255, (address.address>>8)&255,
          address.address&255, address.prefix);
    }
  }
  xv::process::create_initial_process();
}

void initialize_secondary_hart()
{
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

[[noreturn]] void run()
{
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
