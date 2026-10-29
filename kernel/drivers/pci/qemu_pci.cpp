module;
#include "kernel/support/flow.h"

module xv.drivers;
import :pci;
import :platform;

namespace xv::drivers::detail
{

namespace
{
namespace flow = xv::flow;

struct PciConfigureFlow
{
  struct Context
  {
    MmioRegion &config;
    unsigned long registers;
    unsigned command = 0;
    bool result = false;
  };

  static bool supported(Context &c) noexcept
  {
    return c.config.read32(0) == 0x100e8086;
  }

  static void disable(Context &c) noexcept
  {
    c.command = c.config.read32(4) & 0xffff;
    c.config.write32(4, c.command & ~6U);
  }

  static void bar(Context &c) noexcept
  {
    c.config.write32(0x10, static_cast<unsigned>(c.registers));
  }

  static void enable(Context &c) noexcept
  {
    c.config.write32(4, (c.command & ~(1U << 10)) | 6);
    io_barrier();
    c.result = true;
  }
  struct Check;
  struct Disable;
  struct Bar;
  struct Enable;
  using Graph = flow::graph<
      Check, flow::types<flow::node<Check, flow::noop,
                                    flow::branch<flow::condition<&supported>,
                                                 flow::go<Disable>, flow::stop>>,
                         flow::node<Disable, flow::action<&disable>, flow::go<Bar>>,
                         flow::node<Bar, flow::action<&bar>, flow::go<Enable>>,
                         flow::node<Enable, flow::action<&enable>, flow::stop>>>;
};
} // namespace

unsigned find_e1000(unsigned *slots, unsigned capacity)
{
  unsigned count = 0;
  for (unsigned slot = 0; slot < 32 && count < capacity; ++slot)
  {
    MmioRegion config(0x30000000UL + (static_cast<unsigned long>(slot) << 15));
    if (config.read32(0) == 0x100e8086)
    {
      slots[count++] = slot;
    }
  }
  return count;
}

bool configure_e1000(unsigned long registers, unsigned slot)
{
  if (slot >= 32)
  {
    return false;
  }
  MmioRegion config(0x30000000UL + (static_cast<unsigned long>(slot) << 15));
  PciConfigureFlow::Context context{config, registers};
  (void)PciConfigureFlow::Graph::run<0>(context);
  return context.result;
}
} // namespace xv::drivers::detail
