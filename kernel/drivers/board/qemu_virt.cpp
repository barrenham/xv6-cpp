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
#include "kernel/io/kmodule_v3.h"
#include "kernel/io/kmodule_v4.h"

module xv.drivers;

import xv.print;
import xv.arch;
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
  bool present = true;
  unsigned generation = 1, port = ~0U;
  unsigned revision = 0, class_code = 0, subsystem_vendor = 0, subsystem_device = 0;
  const unsigned char *compatible = nullptr;
  unsigned compatible_bytes = 0;
};

Resource resources[xv::hardware::MaxDevices];
unsigned resource_count = 0;

struct HotplugPort
{
  unsigned long configuration = 0;
  xv::hardware::Region window;
  unsigned bus = 0, root_slot = 0, root_function = 0, capability = 0, resource = 0;
};

HotplugPort hotplug_ports[8]{};
unsigned hotplug_port_count = 0;

volatile unsigned short &
pci16(unsigned long address) {
  return *reinterpret_cast<volatile unsigned short *>(address);
}

void
acknowledge_slot(HotplugPort &port) {
  auto &status = pci16(port.configuration + port.capability + 0x1a);
  unsigned observed = status & 0x1f;
  // Clear only observed W1C events. QEMU rejects clearing unset event bits,
  // because that could lose an attention event racing initialization.
  if (observed)
  {
    status = observed;
  }
}

unsigned
pcie_slot(unsigned long address) {
  auto *bytes = reinterpret_cast<volatile unsigned char *>(address);
  unsigned at = bytes[0x34];
  for (unsigned count = 0; at >= 0x40 && at <= 0xc0 && !(at & 3) && count < 48; ++count)
  {
    if (bytes[at] == 0x10 && (pci16(address + at + 2) & 0x100) &&
        (*reinterpret_cast<volatile unsigned *>(address + at + 0x14) & 0x40))
    {
      return at;
    }
    at = bytes[at + 1];
  }
  return 0;
}

unsigned long
child_configuration(const HotplugPort &port) {
  const auto &host = xv::hardware::description().pci;
  return host.configuration.address +
         (static_cast<unsigned long>(port.bus - host.bus_begin) << 20);
}

void
resource_name(char *out, const char *prefix, unsigned ordinal) {
  unsigned at = 0;
  while (*prefix && at < 24)
  {
    out[at++] = *prefix++;
  }
  char reversed[10];
  unsigned length = 0;
  do
  {
    reversed[length++] = '0' + ordinal % 10;
    ordinal /= 10;
  } while (ordinal);
  while (length)
  {
    out[at++] = reversed[--length];
  }
  out[at] = 0;
}

Resource &
append_resource() {
  if (resource_count == xv::hardware::MaxDevices)
  {
    xv::print::panic("device resource capacity exceeded");
  }
  auto &resource = resources[resource_count++];
  resource = {};
  resource.device.size = sizeof(xv_kmod_device);
  return resource;
}

class ModuleInterrupt final : public InterruptDevice
{
public:

  constexpr ModuleInterrupt() = default;

  void
  configure(unsigned irq) {
    irq_ = irq;
  }

  const char *
  name() const override {
    return "device interrupt";
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
    // INTx and platform devices may share a line. Each handler acknowledges
    // only its own device before the router completes the controller source.
    for (unsigned i = 0; i < resource_count; ++i)
    {
      if (__atomic_load_n(&resources[i].device.irq, __ATOMIC_ACQUIRE) == irq_)
      {
        module_interrupt(i);
      }
    }
  }

private:

  unsigned irq_ = 0;
};

constinit ModuleInterrupt interrupt_lines[xv::hardware::MaxDevices];
InterruptDevice *sources[xv::hardware::MaxDevices]{};
constinit InterruptRouter router(plic, sources, 0);

bool
assign_bars(volatile unsigned *config, unsigned long &cursor,
            const xv::hardware::Region &window, xv::hardware::Region &first) {
  unsigned original[6];
  for (unsigned i = 0; i < 6; ++i)
  {
    original[i] = config[4 + i];
  }
  unsigned command = config[1] & 0xffff;
  config[1] = command & ~7U;
  unsigned long start = cursor;
  bool valid = true;
  for (unsigned i = 0; i < 6; ++i)
  {
    unsigned flags = original[i] & 15;
    if (flags & 1)
    {
      continue; // This kernel's device ABI exposes MMIO resources.
    }
    bool wide = (flags & 6) == 4;
    if (wide && i == 5)
    {
      valid = false;
      break;
    }
    config[4 + i] = ~0U;
    if (wide)
    {
      config[5 + i] = ~0U;
    }
    unsigned low = config[4 + i];
    unsigned high = wide ? config[5 + i] : ~0U;
    config[4 + i] = original[i];
    if (wide)
    {
      config[5 + i] = original[i + 1];
    }
    if (!low || low == ~0U)
    {
      if (wide)
      {
        ++i;
      }
      continue;
    }
    unsigned long mask = (static_cast<unsigned long>(high) << 32) | (low & ~15U);
    unsigned long bytes = ~mask + 1;
    if (!bytes || (bytes & (bytes - 1)) || cursor > ~0UL - (bytes - 1))
    {
      valid = false;
      break;
    }
    unsigned long address = (cursor + bytes - 1) & ~(bytes - 1);
    if (address < window.address || address - window.address > window.bytes ||
        bytes > window.bytes - (address - window.address))
    {
      valid = false;
      break;
    }
    config[4 + i] = static_cast<unsigned>(address) | flags;
    if (wide)
    {
      config[5 + i] = 0;
    }
    if (!first.bytes)
    {
      first = {address, bytes};
    }
    cursor = address + bytes;
    if (wide)
    {
      ++i;
    }
  }
  if (!valid || !first.bytes)
  {
    for (unsigned i = 0; i < 6; ++i)
    {
      config[4 + i] = original[i];
    }
    cursor = start;
    config[1] = command;
    return false;
  }
  config[1] = command & ~7U;
  return true;
}

unsigned
scan_ports() {
  unsigned added = 0;
  const auto &tree = xv::hardware::description();
  for (unsigned i = 0; i < hotplug_port_count; ++i)
  {
    auto &port = hotplug_ports[i];
    auto &r = resources[port.resource];
    unsigned status = pci16(port.configuration + port.capability + 0x1a);
    if (r.present || !(status & 0x40))
    {
      continue;
    }
    auto &control = pci16(port.configuration + port.capability + 0x18);
    control = (control & ~0x73fU) | 0x100U; // power on, LED on, polled events
    asm volatile("fence iorw, iorw" ::: "memory");
    auto *config = reinterpret_cast<volatile unsigned *>(child_configuration(port));
    unsigned identity = config[0];
    if (!(identity & 0xffff) || (identity & 0xffff) == 0xffff)
    {
      continue;
    }
    if ((config[3] & 0x7f0000) || (config[2] >> 24) == 6)
    {
      continue;
    }
    xv::hardware::Region registers;
    unsigned long cursor = port.window.address;
    if (!assign_bars(config, cursor, port.window, registers))
    {
      continue;
    }
    unsigned pin = (config[15] >> 8) & 0xff;
    unsigned irq = pin ? xv::hardware::pci_interrupt(tree.pci, port.root_slot,
                                                     port.root_function, pin)
                       : 0;
    if ((pin && !irq) || irq > tree.interrupt_sources ||
        !xv::hardware::enable_pci_function(child_configuration(port), identity))
    {
      continue;
    }
    r.device.id = identity >> 16;
    r.device.vendor = identity & 0xffff;
    r.device.mmio = registers.address;
    r.device.mmio_size = registers.bytes;
    __atomic_store_n(&r.device.irq, irq, __ATOMIC_RELEASE);
    r.flags = XV_KMOD_DEVICE_PCI_READY;
    r.revision = config[2] & 255;
    r.class_code = config[2] >> 8;
    r.subsystem_vendor = config[11] & 0xffff;
    r.subsystem_device = config[11] >> 16;
    ++r.generation;
    r.present = true;
    acknowledge_slot(port); // W1C latched changes
    ++added;
  }
  return added;
}

bool
configure_hotplug_port(unsigned long address, unsigned slot, unsigned function,
                       unsigned &bus, unsigned long &cursor) {
  unsigned cap = pcie_slot(address);
  const auto &host = xv::hardware::description().pci;
  constexpr unsigned long window_size = 2UL * 1024 * 1024;
  unsigned long base = (cursor + window_size - 1) & ~(window_size - 1);
  if (!cap || hotplug_port_count == 8 || bus > host.bus_end ||
      (static_cast<unsigned long>(bus - host.bus_begin) << 20) + (1UL << 20) >
          host.configuration.bytes ||
      base < host.memory.address || base - host.memory.address > host.memory.bytes ||
      window_size > host.memory.bytes - (base - host.memory.address))
  {
    return false;
  }
  auto *config = reinterpret_cast<volatile unsigned *>(address);
  config[6] = host.bus_begin | (bus << 8) | (bus << 16);
  config[8] = ((base >> 16) & 0xfff0) | ((base + window_size - 1) & 0xfff00000);
  config[9] = 0x0000fff0; // disable unused prefetchable and I/O windows
  config[10] = 0;
  config[11] = 0;
  pci16(address + 0x1c) = 0x00f0;
  pci16(address + 4) = 6 | 0x400;
  pci16(address + cap + 0x18) = (pci16(address + cap + 0x1a) & 0x40) ? 0x100 : 0x700;
  auto &r = append_resource();
  r.device.bus = XV_KMOD_BUS_PCI;
  r.port = hotplug_port_count;
  r.present = false;
  r.slot = 0;
  resource_name(r.device.name, "pcie-slot.", hotplug_port_count);
  hotplug_ports[hotplug_port_count++] = {address,
                                         {base, window_size},
                                         bus++,
                                         slot,
                                         function,
                                         cap,
                                         static_cast<unsigned>(&r - resources)};
  cursor = base + window_size;
  return true;
}

void
discover_resources() {
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
    r.compatible = item.compatible;
    r.compatible_bytes = item.compatible_bytes;
    if (item.kind == xv::hardware::DeviceKind::Virtio)
    {
      r.device.bus = XV_KMOD_BUS_VIRTIO_MMIO;
      if (r.device.mmio_size < 16)
      {
        xv::print::panic("VirtIO register window too small");
      }
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
  if (!host.configuration.bytes)
  {
    return;
  }
  unsigned long cursor = host.memory.address;
  unsigned ordinal = 0, secondary_bus = host.bus_begin + 1;
  for (unsigned bus = host.bus_begin; bus <= host.bus_begin; ++bus)
  {
    for (unsigned slot = 0; slot < 32; ++slot)
    {
      unsigned functions = 1;
      for (unsigned function = 0; function < functions; ++function)
      {
        unsigned long offset =
            (static_cast<unsigned long>(bus - host.bus_begin) << 20) | (slot << 15) |
            (function << 12);
        if (offset > host.configuration.bytes ||
            4096 > host.configuration.bytes - offset)
        {
          continue;
        }
        unsigned long address = host.configuration.address + offset;
        auto *config = reinterpret_cast<volatile unsigned *>(address);
        unsigned identity = config[0];
        if (!(identity & 0xffff) || (identity & 0xffff) == 0xffff)
        {
          continue;
        }
        if (!function && (config[3] & 0x800000))
        {
          functions = 8;
        }
        if ((config[3] & 0x7f0000) == 0x10000)
        {
          (void)configure_hotplug_port(address, slot, function, secondary_bus, cursor);
          continue;
        }
        if ((config[2] >> 24) == 6 || (config[3] & 0x7f0000))
        {
          continue;
        }
        xv::hardware::Region registers;
        if (!assign_bars(config, cursor, host.memory, registers))
        {
          xv::print::panic("PCI: unsupported BARs or exhausted DT memory window");
        }
        unsigned pin = (config[15] >> 8) & 0xff;
        unsigned irq = pin ? xv::hardware::pci_interrupt(host, slot, function, pin) : 0;
        if ((pin && !irq) || irq > tree.interrupt_sources ||
            !xv::hardware::enable_pci_function(address, identity))
        {
          xv::print::panic("PCI: invalid DT interrupt route");
        }
        auto &r = append_resource();
        r.device.bus = XV_KMOD_BUS_PCI;
        r.device.id = identity >> 16;
        r.device.vendor = identity & 0xffff;
        r.device.mmio = registers.address;
        r.device.mmio_size = registers.bytes;
        r.device.irq = irq;
        r.slot = slot;
        r.flags = XV_KMOD_DEVICE_PCI_READY;
        r.revision = config[2] & 255;
        r.class_code = config[2] >> 8;
        r.subsystem_vendor = config[11] & 0xffff;
        r.subsystem_device = config[11] >> 16;
        resource_name(r.device.name, "pci.", ordinal++);
      }
    }
  }
  (void)scan_ports();
}

void
configure_routes() {
  const auto &tree = xv::hardware::description();
  plic.configure(tree.plic.address, tree.supervisor_contexts, xv::hardware::MaxHarts);
  unsigned count = 0;
  auto include = [&](unsigned irq) {
    if (!irq || irq > tree.interrupt_sources)
    {
      return;
    }
    for (unsigned j = 0; j < count; ++j)
    {
      if (sources[j]->irq() == irq)
      {
        return;
      }
    }
    if (count == xv::hardware::MaxDevices)
    {
      xv::print::panic("IRQ route capacity");
    }
    interrupt_lines[count].configure(irq);
    sources[count] = &interrupt_lines[count];
    ++count;
  };
  for (unsigned i = 0; i < resource_count; ++i)
  {
    include(resources[i].device.irq);
  }
  if (tree.pci.configuration.bytes)
  {
    for (unsigned slot = 0; slot < 32; ++slot)
    {
      for (unsigned pin = 1; pin <= 4; ++pin)
      {
        include(xv::hardware::pci_interrupt(tree.pci, slot, 0, pin));
      }
    }
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
    if (tree.rtc.bytes)
    {
      rtc.configure(tree.rtc.address);
      rtc.initialize();
      boot_epoch = rtc.seconds();
    }
    bool found_serial = false;
    for (unsigned i = 0; i < tree.devices_count; ++i)
    {
      if (tree.devices[i].kind != xv::hardware::DeviceKind::Serial)
      {
        continue;
      }
      modules::serial.configure(tree.devices[i].registers.address, tree.devices[i].irq);
      found_serial = true;
      break;
    }
    if (!found_serial)
    {
      xv::print::panic("device tree has no console UART");
    }
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
  if (index >= resource_count)
  {
    return false;
  }
  device = resources[index].device;
  if (!resources[index].present)
  {
    device.id = 0;
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
  device.slot = resources[index].slot;
  device.flags = resources[index].flags;
  return true;
}

bool
module_device_v3(unsigned index, xv_kmod_device_v3 &device) {
  device = {};
  if (!module_device_v2(index, device.base))
  {
    return false;
  }
  const auto &resource = resources[index];
  device.revision = resource.revision;
  device.class_code = resource.class_code;
  device.subsystem_vendor = resource.subsystem_vendor;
  device.subsystem_device = resource.subsystem_device;
  // Reject a truncated identity rather than silently matching its prefix.
  if (resource.compatible_bytes > sizeof(device.compatible))
  {
    device.base.flags |= XV_KMOD_DEVICE_COMPATIBLE_TOO_LONG;
    device.bound_module = device.match_index = ~0U;
    return true;
  }
  device.compatible_bytes = resource.compatible_bytes;
  if (resource.compatible_bytes)
  {
    xv::runtime::move(device.compatible, resource.compatible,
                      resource.compatible_bytes);
  }
  device.bound_module = ~0U;
  device.match_index = ~0U;
  return true;
}

unsigned
scan_device_resources() {
  return scan_ports();
}

bool
device_hotplug_status(unsigned index, xv_kmod_hotplug_status &status) {
  if (index >= resource_count)
  {
    return false;
  }
  const auto &r = resources[index];
  status = {};
  status.size = sizeof(status);
  status.index = index;
  status.generation = r.generation;
  status.identity = r.device;
  status.flags = (r.present && r.device.id ? XV_KMOD_PRESENT : 0) |
                 (r.port != ~0U ? XV_KMOD_REMOVABLE : 0);
  status.network_slot = ~0U;
  return true;
}

bool
device_removal_requested(unsigned index, bool &surprise) {
  if (index >= resource_count || !resources[index].present ||
      resources[index].port == ~0U)
  {
    return false;
  }
  auto &port = hotplug_ports[resources[index].port];
  auto *config = reinterpret_cast<volatile unsigned *>(child_configuration(port));
  unsigned identity = config[0];
  surprise = (identity & 0xffff) == 0xffff || !(identity & 0xffff);
  return surprise || (pci16(port.configuration + port.capability + 0x1a) & 1);
}

bool
eject_device_resource(unsigned index) {
  if (index >= resource_count || resources[index].port == ~0U)
  {
    return false;
  }
  auto &r = resources[index];
  auto &port = hotplug_ports[r.port];
  auto *config = reinterpret_cast<volatile unsigned *>(child_configuration(port));
  if ((config[0] & 0xffff) != 0xffff)
  {
    pci16(child_configuration(port) + 4) = 0x400;
  }
  asm volatile("fence iorw, iorw" ::: "memory");
  pci16(port.configuration + port.capability + 0x18) =
      0x700; // power and indicator off, completes QMP device_del
  acknowledge_slot(port);
  r.present = false;
  return true;
}

bool
can_power_off() {
  return xv::hardware::description().finisher.bytes >= sizeof(unsigned);
}

namespace
{
[[noreturn]] void
finish_system(unsigned command) {
  xv::arch::intr_off();
  xv::arch::io_fence();
  *reinterpret_cast<volatile unsigned *>(xv::hardware::description().finisher.address) =
      command;
  xv::arch::io_fence();
  for (;;)
  {
    asm volatile("wfi");
  }
}
} // namespace

[[noreturn]] void
power_off() {
  finish_system(0x5555); // SiFive finisher PASS: guest shutdown, status zero.
}

[[noreturn]] void
reboot() {
  finish_system(0x7777); // SiFive finisher RESET: restart the whole machine.
}

InterruptController &
interrupt_controller() {
  return plic;
}
} // namespace xv::drivers
