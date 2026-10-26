module xv.drivers;
import :pci;
import :platform;
namespace xv::drivers::detail
{
bool configure_e1000(unsigned long registers)
{
  // QEMU virt bus 0, slot 1, function 0; board IRQ routing uses this slot.
  MmioRegion config(0x30000000UL + (1UL << 15));
  if (config.read32(0) != 0x100e8086) return false;
  auto command = config.read32(4) & 0xffff;
  config.write32(4, command & ~6U);
  config.write32(0x10, static_cast<unsigned>(registers));
  config.write32(4, (command & ~(1U << 10)) | 6);
  io_barrier();
  return true;
}
}
