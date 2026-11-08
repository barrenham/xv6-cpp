#pragma once
#include "kernel/io/kmodule_v3.h"

namespace xv::drivers::matching
{
inline bool
valid(const xv_kmod_match &match) {
  if (match.bus < XV_KMOD_BUS_VIRTIO_MMIO || match.bus > XV_KMOD_BUS_PCI ||
      (match.flags & ~XV_KMOD_MATCH_FLAGS) ||
      ((match.flags & XV_KMOD_MATCH_REVISION) &&
       (match.revision_min > match.revision_max || match.revision_max > 255)) ||
      ((match.flags & XV_KMOD_MATCH_CLASS) &&
       (!match.class_mask || (match.class_mask | match.class_code) > 0xffffff)))
  {
    return false;
  }
  if (match.flags & XV_KMOD_MATCH_COMPATIBLE)
  {
    unsigned length = 0;
    while (length < sizeof(match.compatible) && match.compatible[length])
    {
      ++length;
    }
    if (!length || length == sizeof(match.compatible))
    {
      return false;
    }
  }
  return true;
}

inline bool
compatible(const xv_kmod_device_v3 &device, const char *wanted) {
  if (device.compatible_bytes > sizeof(device.compatible))
  {
    return false;
  }
  unsigned at = 0;
  while (at < device.compatible_bytes)
  {
    unsigned length = 0;
    while (at + length < device.compatible_bytes && device.compatible[at + length])
    {
      ++length;
    }
    if (at + length == device.compatible_bytes)
    {
      return false;
    }
    unsigned i = 0;
    while (i < length && wanted[i] && wanted[i] == device.compatible[at + i])
    {
      ++i;
    }
    if (i == length && !wanted[i])
    {
      return true;
    }
    at += length + 1;
  }
  return false;
}

inline bool
accepts(const xv_kmod_match &match, const xv_kmod_device_v3 &device) {
  const auto &base = device.base.base;
  unsigned flags = match.flags;
  return valid(match) && match.bus == base.bus &&
         (!(flags & XV_KMOD_MATCH_VENDOR) || match.vendor == base.vendor) &&
         (!(flags & XV_KMOD_MATCH_DEVICE) || match.device == base.id) &&
         (!(flags & XV_KMOD_MATCH_SUBSYSTEM) ||
          (match.subsystem_vendor == device.subsystem_vendor &&
           match.subsystem_device == device.subsystem_device)) &&
         (!(flags & XV_KMOD_MATCH_REVISION) ||
          (device.revision >= match.revision_min &&
           device.revision <= match.revision_max)) &&
         (!(flags & XV_KMOD_MATCH_CLASS) ||
          (device.class_code & match.class_mask) ==
              (match.class_code & match.class_mask)) &&
         (!(flags & XV_KMOD_MATCH_COMPATIBLE) || compatible(device, match.compatible));
}
} // namespace xv::drivers::matching
