module;

#include "kernel/support/types.h"
#include "kernel/support/flow.h"
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/drivers/device_numbers.h"

module xv.drivers;

import xv.print;

import xv.fs;

import :uart;
import :rtc;
import :n_tty;
import :tty;
import :controlling;
import :entropy;
import :gpu;
import :keyboard;
import :plic;
import :virtio;
import :e1000;
import :pci;

namespace xv::drivers
{
namespace
{
constinit VirtioGpu gpu(VIRTIO0+0x3000,VIRTIO0_IRQ+3);
constinit VirtioKeyboard keyboard(VIRTIO0 + 0x4000, VIRTIO0_IRQ + 4);
constinit ControllingTerminal controlling;
constinit VirtioEntropy entropy(VIRTIO0+0x2000,VIRTIO0_IRQ+2);
constinit GoldfishRtc rtc(RTC0);
unsigned long boot_epoch=0;
constinit Uart16550 uart(UART0, UART0_IRQ);
constinit NttyDiscipline discipline;
constinit Tty terminal(uart, discipline);
constinit Plic plic(PLIC);
constinit VirtioBlock disk(VIRTIO0, VIRTIO0_IRQ);
constinit VirtioBlock data_disk(VIRTIO0 + 0x1000, VIRTIO0_IRQ + 1);
constinit E1000 ethernet[] = {
    {0x40000000UL, 0}, {0x40020000UL, 0}, {0x40040000UL, 0}, {0x40060000UL, 0}};
constinit unsigned ethernet_count = 0;

class PciNetworkInterrupt final : public InterruptDevice
{
public:

  constexpr explicit PciNetworkInterrupt(unsigned irq) : irq_(irq)
  {
  }

  const char *name() const override
  {
    return "PCI Ethernet INTx";
  }

  unsigned irq() const override
  {
    return irq_;
  }

  void initialize() override
  {
  }

  void handle_interrupt() override
  {
    for (unsigned i = 0; i < ethernet_count; ++i)
    {
      if (ethernet[i].irq() == irq_)
      {
        ethernet[i].handle_interrupt();
      }
    }
  }

private:

  unsigned irq_;
};

constinit PciNetworkInterrupt pci_lines[] = {
    PciNetworkInterrupt(32), PciNetworkInterrupt(33), PciNetworkInterrupt(34),
    PciNetworkInterrupt(35)};
constinit InterruptDevice *sources[] = {&uart,         &disk,         &data_disk, &entropy, &gpu, &keyboard,
                                        &pci_lines[0], &pci_lines[1], &pci_lines[2],
                                        &pci_lines[3]};
constinit InterruptRouter router(plic, sources, 10);

namespace flow = xv::flow;

struct ConsoleFlow
{
  struct Context
  {
  };

  static void terminal(Context &) noexcept
  {
    rtc.initialize();
    boot_epoch=rtc.seconds();
    xv::drivers::terminal.initialize();
    initialize_ptys();
  }

  static void serial(Context &) noexcept
  {
    uart.attach(xv::drivers::terminal);
    uart.initialize();
  }

  static void devices(Context &) noexcept
  {
    xv::fs::devices().attach("console", CONSOLE, 0, xv::drivers::terminal,0600);
    xv::fs::devices().attach("ptmx", 4, 0, ptmx());
    xv::fs::devices().attach("tty",6,0,controlling);
    xv::fs::devices().attach("fb0",29,0,gpu,0600);
    xv::fs::devices().attach("kbd0", 30, 0, keyboard, 0600);
    xv::fs::devices().attach("random",7,0,entropy,0444);
    xv::fs::devices().attach("urandom",7,1,entropy,0444);
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
    unsigned (&slots)[4];
  };

  static void discover(Context &c) noexcept
  {
    ethernet_count = detail::find_e1000(c.slots, 4);
  }

  static void locate(Context &c) noexcept
  {
    for (unsigned i = 0; i < ethernet_count; ++i)
    {
      ethernet[i].locate(c.slots[i]);
    }
  }

  static void route(Context &) noexcept
  {
    entropy.initialize();
    gpu.initialize();
    keyboard.initialize();
    router.initialize();
  }
  struct Discover;
  struct Locate;
  struct Route;
  using Graph = flow::graph<
      Discover,
      flow::types<flow::node<Discover, flow::action<&discover>, flow::go<Locate>>,
                  flow::node<Locate, flow::action<&locate>, flow::go<Route>>,
                  flow::node<Route, flow::action<&route>, flow::stop>>>;
};

} // namespace

unsigned long realtime_seconds(){return rtc.seconds();}
unsigned long boot_time_seconds(){return boot_epoch;}

void initialize_console()
{
  ConsoleFlow::Context context{};
  (void)ConsoleFlow::Graph::run<0>(context);
}

void initialize_interrupts()
{
  unsigned slots[4];
  InterruptsFlow::Context context{slots};
  (void)InterruptsFlow::Graph::run<0>(context);
}

void initialize_hart(int hart)
{
  router.initialize_hart(hart);
}

void handle_external_interrupt(int hart)
{
  const auto unknown = router.dispatch(hart);
  if (unknown)
  {
    xv::print::printk("unexpected interrupt irq=%d\n", unknown);
  }
}

unsigned network_device_count()
{
  return ethernet_count;
}

void initialize_network(EthernetReceiver &receiver, unsigned index)
{
  if (index >= ethernet_count)
  {
    return;
  }
  ethernet[index].attach(receiver);
  ethernet[index].initialize();
}

EthernetDevice &network_device(unsigned index)
{
  if (index >= 4)
  {
    xv::print::panic("network device index");
  }
  return ethernet[index];
}

CharacterDevice &console()
{
  return terminal;
}

void terminal_timer_tick()
{
  terminal.timer_tick();
  pty_timer_tick();
}

SerialPort &serial_port()
{
  return uart;
}

BlockDevice *data_device()
{
  return data_disk.present() ? &data_disk : nullptr;
}

BlockDevice &block_device()
{
  return disk;
}

InterruptController &interrupt_controller()
{
  return plic;
}
} // namespace xv::drivers
