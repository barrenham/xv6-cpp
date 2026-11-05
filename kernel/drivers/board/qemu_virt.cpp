module;

#include "kernel/support/types.h"
#include "kernel/hardware/device_tree.h"
#include "kernel/hardware/pci.h"
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
constinit GoldfishRtc rtc(0);
unsigned long boot_epoch = 0;
constinit NttyDiscipline discipline;
constinit Tty terminal(modules::serial, discipline);
constinit Plic plic(0);

struct Resource
{
  xv_kmod_device device{};
  unsigned slot = 0, flags = 0;
};
Resource resources[xv::hardware::MaxDevices];
unsigned resource_count = 0;

void resource_name(char *out, const char *prefix, unsigned ordinal)
{
  unsigned at = 0;
  while (*prefix && at < 24) out[at++] = *prefix++;
  char reversed[10]; unsigned length = 0;
  do { reversed[length++] = '0' + ordinal % 10; ordinal /= 10; } while (ordinal);
  while (length) out[at++] = reversed[--length];
  out[at] = 0;
}

Resource &append_resource()
{
  if (resource_count == xv::hardware::MaxDevices) xv::print::panic("device resource capacity exceeded");
  auto &resource = resources[resource_count++];
  resource = {};
  resource.device.size = sizeof(xv_kmod_device);
  return resource;
}

class ModuleInterrupt final : public InterruptDevice
{
public:
  constexpr ModuleInterrupt() = default;
  void configure(unsigned irq) { irq_ = irq; }
  const char *name() const override { return "device interrupt"; }
  unsigned irq() const override { return irq_; }
  void initialize() override {}
  void handle_interrupt() override
  {
    // INTx and platform devices may share a line. Each handler acknowledges
    // only its own device before the router completes the controller source.
    for (unsigned i = 0; i < resource_count; ++i)
      if (resources[i].device.irq == irq_) module_interrupt(i);
  }
private:
  unsigned irq_ = 0;
};
constinit ModuleInterrupt interrupt_lines[xv::hardware::MaxDevices];
InterruptDevice *sources[xv::hardware::MaxDevices]{};
constinit InterruptRouter router(plic, sources, 0);

bool assign_bars(volatile unsigned *config, unsigned long &cursor,
                 const xv::hardware::Region &window, xv::hardware::Region &first)
{
  unsigned original[6];
  for (unsigned i = 0; i < 6; ++i) original[i] = config[4 + i];
  unsigned command = config[1] & 0xffff;
  config[1] = command & ~7U;
  unsigned long start = cursor;
  bool valid = true;
  for (unsigned i = 0; i < 6; ++i)
  {
    unsigned flags = original[i] & 15;
    if (flags & 1) continue; // This kernel's device ABI exposes MMIO resources.
    bool wide = (flags & 6) == 4;
    if (wide && i == 5) { valid = false; break; }
    config[4 + i] = ~0U;
    if (wide) config[5 + i] = ~0U;
    unsigned low = config[4 + i];
    unsigned high = wide ? config[5 + i] : ~0U;
    config[4 + i] = original[i];
    if (wide) config[5 + i] = original[i + 1];
    if (!low || low == ~0U) { if (wide) ++i; continue; }
    unsigned long mask = (static_cast<unsigned long>(high) << 32) | (low & ~15U);
    unsigned long bytes = ~mask + 1;
    if (!bytes || (bytes & (bytes - 1)) || cursor > ~0UL - (bytes - 1)) { valid = false; break; }
    unsigned long address = (cursor + bytes - 1) & ~(bytes - 1);
    if (address < window.address || address - window.address > window.bytes ||
        bytes > window.bytes - (address - window.address)) { valid = false; break; }
    config[4 + i] = static_cast<unsigned>(address) | flags;
    if (wide) config[5 + i] = 0;
    if (!first.bytes) first = {address, bytes};
    cursor = address + bytes;
    if (wide) ++i;
  }
  if (!valid || !first.bytes)
  {
    for (unsigned i = 0; i < 6; ++i) config[4 + i] = original[i];
    cursor = start;
    config[1] = command;
    return false;
  }
  config[1] = command & ~7U;
  return true;
}

void discover_resources()
{
  const auto &tree = xv::hardware::description();
  unsigned virtio = 0, serial = 0;
  for (unsigned i = 0; i < tree.devices_count; ++i)
  {
    const auto &item = tree.devices[i];
    auto &r = append_resource();
    r.device.mmio = item.registers.address;
    r.device.mmio_size = item.registers.bytes;
    r.device.irq = item.irq;
    r.device.id = item.type;
    if (item.kind == xv::hardware::DeviceKind::Virtio)
    {
      r.device.bus = XV_KMOD_BUS_VIRTIO_MMIO;
      if (r.device.mmio_size < 16) xv::print::panic("VirtIO register window too small");
      auto *registers = reinterpret_cast<volatile unsigned *>(r.device.mmio);
      r.device.id = registers[0] == 0x74726976 ? registers[2] : 0;
      r.device.vendor = registers[3];
      resource_name(r.device.name, "virtio-mmio.", virtio++);
    }
    else if (item.kind == xv::hardware::DeviceKind::Serial)
    {
      r.device.bus = XV_KMOD_BUS_PLATFORM;
      resource_name(r.device.name, "uart", serial++);
    }
    else
    {
      r.device.bus = XV_KMOD_BUS_PLATFORM;
      xv::runtime::copy_fixed(r.device.name, item.name, sizeof(r.device.name));
    }
  }
  auto &software = append_resource();
  software.device.bus = XV_KMOD_BUS_SOFTWARE;
  software.device.id = 1;
  xv::runtime::copy_fixed(software.device.name, "memory", sizeof(software.device.name));
  const auto &host = tree.pci;
  if (!host.configuration.bytes) return;
  unsigned long cursor = host.memory.address;
  unsigned ordinal = 0;
  for (unsigned bus = host.bus_begin; bus <= host.bus_end; ++bus)
  {
    for (unsigned slot = 0; slot < 32; ++slot)
    {
      unsigned functions = 1;
      for (unsigned function = 0; function < functions; ++function)
      {
        unsigned long offset = (static_cast<unsigned long>(bus - host.bus_begin) << 20) |
                               (slot << 15) | (function << 12);
        if (offset > host.configuration.bytes || 4096 > host.configuration.bytes - offset) continue;
        unsigned long address = host.configuration.address + offset;
        auto *config = reinterpret_cast<volatile unsigned *>(address);
        unsigned identity = config[0];
        if (!(identity & 0xffff) || (identity & 0xffff) == 0xffff) continue;
        if (!function && (config[3] & 0x800000)) functions = 8;
        if ((config[2] >> 24) == 6 || (config[3] & 0x7f0000)) continue;
        xv::hardware::Region registers;
        if (!assign_bars(config, cursor, host.memory, registers))
          xv::print::panic("PCI: unsupported BARs or exhausted DT memory window");
        unsigned pin = (config[15] >> 8) & 0xff;
        unsigned irq = pin ? xv::hardware::pci_interrupt(host, slot, function, pin) : 0;
        if ((pin && !irq) || irq > tree.interrupt_sources || !xv::hardware::enable_pci_function(address, identity))
          xv::print::panic("PCI: invalid DT interrupt route");
        auto &r = append_resource();
        r.device.bus = XV_KMOD_BUS_PCI;
        r.device.id = identity >> 16; r.device.vendor = identity & 0xffff;
        r.device.mmio = registers.address; r.device.mmio_size = registers.bytes;
        r.device.irq = irq; r.slot = slot; r.flags = XV_KMOD_DEVICE_PCI_READY;
        resource_name(r.device.name, "pci.", ordinal++);
      }
    }
  }
}

void configure_routes()
{
  const auto &tree = xv::hardware::description();
  plic.configure(tree.plic.address, tree.supervisor_contexts, xv::hardware::MaxHarts);
  unsigned count = 0;
  for (unsigned i = 0; i < resource_count; ++i)
  {
    unsigned irq = resources[i].device.irq;
    if (!irq) continue;
    bool present = false;
    for (unsigned j = 0; j < count; ++j) present |= sources[j]->irq() == irq;
    if (present) continue;
    interrupt_lines[count].configure(irq);
    sources[count] = &interrupt_lines[count];
    ++count;
  }
  router.configure(sources, count);
  router.initialize();
}

namespace flow = xv::flow;

struct ConsoleFlow
{
  struct Context
  {
  };

  static void
  terminal(Context &) noexcept {
    const auto &tree = xv::hardware::description();
    if (tree.rtc.bytes) { rtc.configure(tree.rtc.address); rtc.initialize(); boot_epoch = rtc.seconds(); }
    bool found_serial = false;
    for (unsigned i = 0; i < tree.devices_count; ++i)
    {
      if (tree.devices[i].kind != xv::hardware::DeviceKind::Serial) continue;
      modules::serial.configure(tree.devices[i].registers.address, tree.devices[i].irq);
      found_serial = true; break;
    }
    if (!found_serial) xv::print::panic("device tree has no console UART");
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
    discover_resources();
  }

  static void
  route(Context &) noexcept {
    configure_routes();
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
realtime_nanoseconds() {
  return xv::hardware::description().rtc.bytes ? rtc.nanoseconds() : 0;
}

unsigned long
realtime_seconds() {
  return xv::hardware::description().rtc.bytes ? rtc.seconds() : 0;
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
  if (index >= resource_count) return false;
  device = resources[index].device;
  return true;
}

bool
module_device_v2(unsigned index, xv_kmod_device_v2 &device) {
  device = {};
  if (!module_device(index, device.base)) return false;
  device.slot = resources[index].slot;
  device.flags = resources[index].flags;
  return true;
}


InterruptController &
interrupt_controller() {
  return plic;
}
} // namespace xv::drivers
