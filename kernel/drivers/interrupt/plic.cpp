module xv.drivers;

import :plic;

namespace xv::drivers
{
void Plic::initialize()
{
  // Source priorities are supplied by the board's InterruptRouter.
}

void Plic::set_priority(unsigned irq, unsigned priority)
{
  registers_.write32(irq * 4, priority);
}

void Plic::initialize_hart(int hart)
{
  for (unsigned word = 0; word < 32; ++word)
  {
    registers_.write32(0x2080 + hart * 0x100 + word * 4, 0);
  }
  registers_.write32(0x201000 + hart * 0x2000, 0);
}

void Plic::enable(int hart, unsigned irq)
{
  unsigned offset = 0x2080 + hart * 0x100 + (irq / 32) * 4;
  unsigned mask = registers_.read32(offset);
  registers_.write32(offset, mask | (1U << (irq % 32)));
}

unsigned Plic::claim(int hart)
{
  return registers_.read32(0x201004 + hart * 0x2000);
}

void Plic::complete(int hart, unsigned irq)
{
  registers_.write32(0x201004 + hart * 0x2000, irq);
}
} // namespace xv::drivers
