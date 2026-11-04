module;

#include "kernel/support/types.h"
#include "kernel/support/flow.h"
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/drivers/device_numbers.h"
#include "kernel/io/kmodule.h"
#include "kernel/io/kmodule_v2.h"

module xv.drivers;

import xv.print;
import xv.runtime;

import xv.fs;

import :serial_proxy;
import :module_runtime;
import :rtc;
import :n_tty;
import :tty;
import :controlling;
import :plic;

namespace xv::drivers
{
namespace
{
constinit ControllingTerminal controlling;
constinit GoldfishRtc rtc(RTC0);
unsigned long boot_epoch = 0;
constinit NttyDiscipline discipline;
constinit Tty terminal(modules::serial, discipline);
constinit Plic plic(PLIC);

class ModuleInterrupt final : public InterruptDevice
{
public:

  constexpr explicit ModuleInterrupt(unsigned index) : index_(index) {
  }

  const char *
  name() const override {
    return "loadable device";
  }

  unsigned
  irq() const override {
    return index_ == 6 ? UART0_IRQ : VIRTIO0_IRQ + index_;
  }

  void
  initialize() override {
  }

  void
  handle_interrupt() override {
    module_interrupt(index_);
  }

private:

  unsigned index_;
};

constinit ModuleInterrupt disk_interrupt(0);
constinit ModuleInterrupt data_interrupt(1);
constinit ModuleInterrupt entropy_interrupt(2);
constinit ModuleInterrupt gpu_interrupt(3);
constinit ModuleInterrupt keyboard_interrupt(4);
constinit ModuleInterrupt mouse_interrupt(5);
constinit ModuleInterrupt serial_interrupt(6);
unsigned pci_slots[4]{};
unsigned pci_ids[4]{};
unsigned pci_count = 0;

class PciInterrupt final : public InterruptDevice
{
public:

  constexpr explicit PciInterrupt(unsigned irq) : irq_(irq) {
  }

  const char *
  name() const override {
    return "PCI INTx";
  }

  unsigned
  irq() const override {
    return irq_;
  }

  void
  initialize() override {
  }

  void
  handle_interrupt() override {
    for (unsigned i = 0; i < pci_count; ++i)
    {
      if (32 + pci_slots[i] % 4 == irq_)
      {
        module_interrupt(8 + i);
      }
    }
  }

private:

  unsigned irq_;
};

constinit PciInterrupt pci_lines[] = {PciInterrupt(32), PciInterrupt(33),
                                      PciInterrupt(34), PciInterrupt(35)};
constinit InterruptDevice *sources[] = {
    &serial_interrupt, &disk_interrupt,     &data_interrupt,  &entropy_interrupt,
    &gpu_interrupt,    &keyboard_interrupt, &mouse_interrupt, &pci_lines[0],
    &pci_lines[1],     &pci_lines[2],       &pci_lines[3]};
constinit InterruptRouter router(plic, sources, 11);

namespace flow = xv::flow;

struct ConsoleFlow
{
  struct Context
  {
  };

  static void
  terminal(Context &) noexcept {
    rtc.initialize();
    boot_epoch = rtc.seconds();
    xv::drivers::terminal.initialize();
    initialize_ptys();
  }

  static void
  serial(Context &) noexcept {
    modules::serial.attach(xv::drivers::terminal);
    modules::serial.initialize();
  }

  static void
  devices(Context &) noexcept {
    xv::fs::devices().attach("console", CONSOLE, 0, xv::drivers::terminal, 0600);
    xv::fs::devices().attach("ptmx", 4, 0, ptmx());
    xv::fs::devices().attach("tty", 6, 0, controlling);
  }
  struct Terminal;
  struct Serial;
  struct Devices;
  using Graph = flow::graph<
      Terminal,
      flow::types<flow::node<Terminal, flow::action<&terminal>, flow::go<Serial>>,
                  flow::node<Serial, flow::action<&serial>, flow::go<Devices>>,
                  flow::node<Devices, flow::action<&devices>, flow::stop>>>;
};

struct InterruptsFlow
{
  struct Context
  {
  };

  static void
  discover(Context &c) noexcept {
    for (unsigned slot = 0; slot < 32 && pci_count < 4; ++slot)
    {
      auto *config = reinterpret_cast<volatile unsigned *>(0x30000000UL + (slot << 15));
      unsigned id = config[0];
      unsigned vendor = id & 0xffff;
      if (!vendor || vendor == 0xffff || (config[2] >> 24) == 6)
      {
        continue;
      }
      pci_slots[pci_count] = slot;
      pci_ids[pci_count] = id;
      ++pci_count;
    }
  }

  static void
  route(Context &) noexcept {
    router.initialize();
  }
  struct Discover;
  struct Route;
  using Graph = flow::graph<
      Discover,
      flow::types<flow::node<Discover, flow::action<&discover>, flow::go<Route>>,
                  flow::node<Route, flow::action<&route>, flow::stop>>>;
};

} // namespace

unsigned long
realtime_seconds() {
  return rtc.seconds();
}

unsigned long
boot_time_seconds() {
  return boot_epoch;
}

void
initialize_console() {
  ConsoleFlow::Context context{};
  (void)ConsoleFlow::Graph::run<0>(context);
}

void
initialize_interrupts() {
  initialize_modules();
  InterruptsFlow::Context context{};
  (void)InterruptsFlow::Graph::run<0>(context);
}

void
initialize_hart(int hart) {
  router.initialize_hart(hart);
}

void
handle_external_interrupt(int hart) {
  const auto unknown = router.dispatch(hart);
  if (unknown)
  {
    xv::print::printk("unexpected interrupt irq=%d\n", unknown);
  }
}

unsigned
network_device_count() {
  return modules::network_count();
}

void
initialize_network(EthernetReceiver &receiver, unsigned index) {
  modules::network_receiver(index, receiver);
}

EthernetDevice &
network_device(unsigned index) {
  return modules::network_proxy(index);
}

CharacterDevice &
console() {
  return terminal;
}

void
terminal_timer_tick() {
  terminal.timer_tick();
  pty_timer_tick();
}

SerialPort &
serial_port() {
  return modules::serial;
}

bool
module_device(unsigned index, xv_kmod_device &device) {
  if (index > 11)
  {
    return false;
  }
  if (index >= 8)
  {
    unsigned port = index - 8;
    device = {};
    device.size = sizeof(device);
    device.bus = XV_KMOD_BUS_PCI;
    device.id = port < pci_count ? pci_ids[port] >> 16 : 0;
    device.vendor = port < pci_count ? pci_ids[port] & 0xffff : 0;
    device.irq = port < pci_count ? 32 + pci_slots[port] % 4 : 0;
    device.mmio = 0x40000000UL + port * 0x20000UL;
    device.mmio_size = 0x20000;
    char name[] = "pci.0";
    name[4] += port;
    xv::runtime::copy_fixed(device.name, name, sizeof(device.name));
    return true;
  }
  if (index == 6 || index == 7)
  {
    device = {};
    device.size = sizeof(device);
    device.bus = index == 6 ? XV_KMOD_BUS_PLATFORM : XV_KMOD_BUS_SOFTWARE;
    device.id = 1;
    device.irq = index == 6 ? UART0_IRQ : 0;
    device.mmio = index == 6 ? UART0 : 0;
    device.mmio_size = index == 6 ? 4096 : 0;
    xv::runtime::copy_fixed(device.name, index == 6 ? "uart0" : "memory",
                            sizeof(device.name));
    return true;
  }
  auto *registers = reinterpret_cast<volatile unsigned *>(VIRTIO0 + index * 0x1000UL);
  device = {};
  device.size = sizeof(device);
  device.bus = XV_KMOD_BUS_VIRTIO_MMIO;
  device.id = registers[0] == 0x74726976 ? registers[2] : 0;
  device.vendor = registers[3];
  device.irq = VIRTIO0_IRQ + index;
  device.mmio = VIRTIO0 + index * 0x1000UL;
  device.mmio_size = 4096;
  char name[] = "virtio-mmio.0";
  name[12] = '0' + index;
  for (unsigned i = 0; i < sizeof(name); ++i)
  {
    device.name[i] = name[i];
  }
  return true;
}

bool
module_device_v2(unsigned index, xv_kmod_device_v2 &device) {
  device = {};
  if (!module_device(index, device.base))
  {
    return false;
  }
  if (index >= 8 && index - 8 < pci_count)
  {
    device.slot = pci_slots[index - 8];
  }
  return true;
}

InterruptController &
interrupt_controller() {
  return plic;
}
} // namespace xv::drivers
