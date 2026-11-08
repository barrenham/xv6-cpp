#include "kernel/drivers/modules/matching.h"
#include <cassert>
#include <cstring>
#include <cstdio>

int
main() {
  using xv::drivers::matching::accepts;
  using xv::drivers::matching::valid;
  xv_kmod_device_v3 device{};
  device.base.base.bus = XV_KMOD_BUS_PCI;
  device.base.base.vendor = 0x8086;
  device.base.base.id = 0x100e;
  device.revision = 2;
  device.class_code = 0x020001;
  device.subsystem_vendor = 0x1234;
  device.subsystem_device = 0x5678;
  xv_kmod_match match{};
  match.bus = XV_KMOD_BUS_PCI;
  match.flags = XV_KMOD_MATCH_VENDOR | XV_KMOD_MATCH_DEVICE | XV_KMOD_MATCH_REVISION |
                XV_KMOD_MATCH_CLASS | XV_KMOD_MATCH_SUBSYSTEM;
  match.vendor = 0x8086;
  match.device = 0x100e;
  match.revision_min = 1;
  match.revision_max = 3;
  match.class_code = 0x020000;
  match.class_mask = 0xffff00;
  match.subsystem_vendor = 0x1234;
  match.subsystem_device = 0x5678;
  assert(accepts(match, device));
  device.base.base.vendor = 0x1234;
  assert(!accepts(match, device)); // same device ID from another vendor
  device.base.base.vendor = 0x8086;
  device.revision = 0;
  assert(!accepts(match, device));
  device.revision = 4;
  assert(!accepts(match, device));
  device.revision = 3;
  assert(accepts(match, device));
  device.class_code = 0x010001;
  assert(!accepts(match, device));
  device.class_code = 0x020001;
  device.subsystem_device ^= 1;
  assert(!accepts(match, device));
  device.subsystem_device ^= 1;
  device.base.base.bus = XV_KMOD_BUS_PLATFORM;
  assert(!accepts(match, device));
  const char compatibles[] = "acme,uart-rev2\0ns16550a";
  std::memcpy(device.compatible, compatibles, sizeof(compatibles));
  device.compatible_bytes = sizeof(compatibles);
  match = {};
  match.bus = XV_KMOD_BUS_PLATFORM;
  match.flags = XV_KMOD_MATCH_COMPATIBLE;
  std::strcpy(match.compatible, "ns16550a");
  assert(accepts(match, device)); // fallback string, not only first compatible
  std::strcpy(match.compatible, "ns16550");
  assert(!accepts(match, device)); // exact match, not a prefix
  std::strcpy(match.compatible, "ns16550a");
  --device.compatible_bytes;
  assert(!accepts(match, device)); // unterminated final compatible
  match.flags |= 128;
  assert(!valid(match));
  match.flags = XV_KMOD_MATCH_REVISION;
  match.revision_min = 4;
  match.revision_max = 2;
  assert(!valid(match));
  puts(
      "Driver matching: vendor/device/subsystem, revision ranges, class masks and DT fallback identities PASS");
}
