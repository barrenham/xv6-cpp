module xv.drivers;
import :pci;
import :platform;
namespace xv::drivers::detail
{
unsigned find_e1000(unsigned *slots, unsigned capacity)
{
  unsigned count = 0;
  for (unsigned slot = 0; slot < 32 && count < capacity; ++slot)
  {
    MmioRegion config(0x30000000UL + (static_cast<unsigned long>(slot)<<15));
    if (config.read32(0) == 0x100e8086) slots[count++] = slot;
  }
  return count;
}
bool configure_e1000(unsigned long registers, unsigned slot)
{
  if (slot >= 32) return false;
  MmioRegion config(0x30000000UL + (static_cast<unsigned long>(slot)<<15));
  if (config.read32(0) != 0x100e8086) return false;
  auto command = config.read32(4) & 0xffff;
  config.write32(4, command & ~6U);
  config.write32(0x10, static_cast<unsigned>(registers));
  config.write32(4, (command & ~(1U << 10)) | 6);
  io_barrier();
  return true;
}
}
