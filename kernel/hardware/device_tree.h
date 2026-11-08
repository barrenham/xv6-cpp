#pragma once

namespace xv::hardware
{
inline constexpr unsigned MaxDevices = 64;
inline constexpr unsigned MaxRegions = 128;
inline constexpr unsigned MaxHarts = 64;
inline constexpr unsigned Invalid = ~0U;
inline constexpr unsigned long MaxBlob = 2UL * 1024 * 1024;

enum class DeviceKind
{
  Virtio,
  Serial,
  Platform
};

struct Region
{
  unsigned long address = 0, bytes = 0;
};

struct Device
{
  Region registers;
  DeviceKind kind = DeviceKind::Platform;
  unsigned irq = 0, type = 0;
  char name[32]{};
  const unsigned char *compatible = nullptr;
  unsigned compatible_bytes = 0;
};

struct PciHost
{
  Region configuration, memory;
  unsigned bus_begin = 0, bus_end = 0;
  const unsigned char *interrupt_map = nullptr;
  unsigned map_bytes = 0;
  unsigned mask[4]{};
  unsigned controller = 0;
};

struct Description
{
  Device devices[MaxDevices]{};
  Region regions[MaxRegions]{};
  unsigned devices_count = 0, regions_count = 0;
  Region plic, rtc, timer;
  unsigned plic_phandle = 0, interrupt_sources = 0;
  unsigned supervisor_contexts[MaxHarts]{};
  PciHost pci;
};

// Bounded FDT v17 parser. The blob must outlive references in PciHost.
// Unsupported address translation or interrupt controllers fail explicitly.
bool discover(const void *blob, unsigned long capacity, Description &out);
bool initialize(const void *blob);
const Description &description();
unsigned pci_interrupt(const PciHost &, unsigned slot, unsigned function, unsigned pin);
unsigned compatible_id(const char *compatible);
} // namespace xv::hardware
