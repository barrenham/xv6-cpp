module;

#include "kernel/support/types.h"
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/drivers/device_numbers.h"

module xv.drivers;

import xv.print;

import xv.fs;

import :uart;
import :n_tty;
import :tty;
import :plic;
import :virtio;
import :e1000;
import :pci;

namespace xv::drivers
{
namespace
{
constinit Uart16550 uart(UART0, UART0_IRQ);
constinit NttyDiscipline discipline;
constinit Tty terminal(uart, discipline);
constinit Plic plic(PLIC);
constinit VirtioBlock disk(VIRTIO0, VIRTIO0_IRQ);
constinit E1000 ethernet[] = {{0x40000000UL, 0}, {0x40020000UL, 0},
                              {0x40040000UL, 0}, {0x40060000UL, 0}};
constinit unsigned ethernet_count = 0;
class PciNetworkInterrupt final : public InterruptDevice
{
public:
  constexpr explicit PciNetworkInterrupt(unsigned irq) : irq_(irq) {}
  const char *name() const override { return "PCI Ethernet INTx"; }
  unsigned irq() const override { return irq_; }
  void initialize() override {}
  void handle_interrupt() override
  {
    for (unsigned i = 0; i < ethernet_count; ++i)
      if (ethernet[i].irq() == irq_) ethernet[i].handle_interrupt();
  }
private:
  unsigned irq_;
};
constinit PciNetworkInterrupt pci_lines[] = {PciNetworkInterrupt(32), PciNetworkInterrupt(33),
                                            PciNetworkInterrupt(34), PciNetworkInterrupt(35)};
constinit InterruptDevice *sources[] = {&uart, &disk, &pci_lines[0], &pci_lines[1], &pci_lines[2], &pci_lines[3]};
constinit InterruptRouter router(plic, sources, 6);

} // namespace

void initialize_console()
{
  terminal.initialize();
  initialize_ptys();
  uart.attach(terminal);
  uart.initialize();
  xv::fs::devices().attach("console", CONSOLE, 0, terminal);
  xv::fs::devices().attach("ptmx", 4, 0, ptmx());
}

void initialize_interrupts()
{
  unsigned slots[4]; ethernet_count = detail::find_e1000(slots, 4);
  for (unsigned i = 0; i < ethernet_count; ++i) ethernet[i].locate(slots[i]);
  router.initialize();
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

unsigned network_device_count() { return ethernet_count; }
void initialize_network(EthernetReceiver &receiver, unsigned index)
{
  if (index >= ethernet_count) return;
  ethernet[index].attach(receiver); ethernet[index].initialize();
}
EthernetDevice &network_device(unsigned index)
{
  if (index >= 4) xv::print::panic("network device index");
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

BlockDevice &block_device()
{
  return disk;
}

InterruptController &interrupt_controller()
{
  return plic;
}
} // namespace xv::drivers
