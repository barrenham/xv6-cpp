#pragma once

namespace xv::hardware
{
// ECAM address and BAR allocation belong to the bus, never to a device ELF.
inline bool
enable_pci_function(unsigned long configuration, unsigned identity) {
  if (!configuration || (configuration & 4095) || !identity || identity == ~0U)
  {
    return false;
  }
  auto *registers = reinterpret_cast<volatile unsigned *>(configuration);
  if (registers[0] != identity)
  {
    return false;
  }
  unsigned command = registers[1] & 0xffff;
  registers[1] = (command & ~(1U << 10)) | 6U;
  return true;
}
} // namespace xv::hardware
