module;

#include "kernel/support/types.h"
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/drivers/device_numbers.h"

module xv.drivers;

import xv.print;

import xv.fs;

import :uart;
import :console;
import :plic;
import :virtio;

namespace xv::drivers
{
namespace
{
constinit Uart16550 uart(UART0, UART0_IRQ);
constinit LineConsole terminal(uart);
constinit Plic plic(PLIC);
constinit VirtioBlock disk(VIRTIO0, VIRTIO0_IRQ);
constinit InterruptDevice *sources[] = {&uart, &disk};
constinit InterruptRouter router(plic, sources, 2);

} // namespace

void initialize_console()
{
  terminal.initialize();
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
