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

namespace xv::drivers
{
namespace
{
constinit Uart16550 uart(UART0, UART0_IRQ);
constinit NttyDiscipline discipline;
constinit Tty terminal(uart, discipline);
constinit Plic plic(PLIC);
constinit VirtioBlock disk(VIRTIO0, VIRTIO0_IRQ);
constinit InterruptDevice *sources[] = {&uart, &disk};
constinit InterruptRouter router(plic, sources, 2);

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
