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

namespace xv::drivers
{
namespace
{
constinit Uart16550 uart(UART0, UART0_IRQ);
constinit NttyDiscipline discipline;
constinit Tty terminal(uart, discipline);
constinit Plic plic(PLIC);
constinit VirtioBlock disk(VIRTIO0, VIRTIO0_IRQ);
constinit E1000 ethernet(0x40000000UL, E1000_IRQ);
constinit InterruptDevice *sources[] = {&uart, &disk, &ethernet};
constinit InterruptRouter router(plic, sources, 3);

} // namespace

void initialize_console()
{
  terminal.initialize();
  initialize_ptys();
  uart.attach(terminal);
  uart.initialize();
  xv::fs::devices().attach(CONSOLE, terminal);
}

void initialize_interrupts()
{
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

void initialize_network(EthernetReceiver &receiver)
{
  ethernet.attach(receiver);
  ethernet.initialize();
}

EthernetDevice &network_device()
{
  return ethernet;
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
