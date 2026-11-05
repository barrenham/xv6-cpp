#include "device_tree.h"

namespace xv::hardware
{
namespace
{
using Byte = unsigned char;
constexpr unsigned MaxNodes = 256, MaxDepth = 32;

struct Property
{
  const Byte *data = nullptr;
  unsigned bytes = 0;
};

struct Node
{
  const char *name = nullptr;
  unsigned parent = Invalid, address_cells = 2, size_cells = 1;
  unsigned phandle = 0, interrupt_cells = 0, interrupt_parent = 0;
  Property reg, compatible, ranges, interrupts, extended, map, mask, buses;
  unsigned sources = 0;
  bool disabled = false, children = false;
};

// Discovery runs once, before allocators or secondary harts use the result.
Node nodes[MaxNodes];
Description system;

unsigned
word(const Byte *p) {
  return (unsigned(p[0]) << 24) | (unsigned(p[1]) << 16) | (unsigned(p[2]) << 8) | p[3];
}

bool
same(const char *a, const char *b) {
  while (*a && *a == *b)
  {
    ++a;
    ++b;
  }
  return *a == *b;
}

bool
text(const Byte *p, unsigned bytes, unsigned &length) {
  for (length = 0; length < bytes; ++length)
  {
    if (!p[length])
    {
      return true;
    }
  }
  return false;
}

bool
compatible(const Node &n, const char *wanted) {
  for (unsigned at = 0; at < n.compatible.bytes;)
  {
    unsigned length;
    if (!text(n.compatible.data + at, n.compatible.bytes - at, length))
    {
      return false;
    }
    if (same(reinterpret_cast<const char *>(n.compatible.data + at), wanted))
    {
      return true;
    }
    at += length + 1;
  }
  return false;
}

bool
cell(const Byte *p, unsigned count, unsigned long &out) {
  if (count > 2)
  {
    return false;
  }
  out = 0;
  for (unsigned i = 0; i < count; ++i)
  {
    out = (out << 32) | word(p + i * 4);
  }
  return true;
}

unsigned
find(unsigned phandle, unsigned count) {
  if (!phandle)
  {
    return Invalid;
  }
  for (unsigned i = 0; i < count; ++i)
  {
    if (nodes[i].phandle == phandle)
    {
      return i;
    }
  }
  return Invalid;
}

void
copy_name(char *out, const char *in) {
  unsigned i = 0;
  while (i < 31 && in[i] && in[i] != '@')
  {
    out[i] = in[i];
    ++i;
  }
  out[i] = 0;
}

bool
region(unsigned index, Region &out) {
  const auto &node = nodes[index];
  if (node.parent == Invalid)
  {
    return false;
  }
  const auto &parent = nodes[node.parent];
  unsigned stride = (parent.address_cells + parent.size_cells) * 4;
  if (!node.reg.data || !stride || node.reg.bytes < stride || node.reg.bytes % stride ||
      !parent.address_cells || !parent.size_cells ||
      !cell(node.reg.data, parent.address_cells, out.address) ||
      !cell(node.reg.data + parent.address_cells * 4, parent.size_cells, out.bytes) ||
      !out.bytes || out.bytes > ~0UL - out.address)
  {
    return false;
  }
  for (unsigned bus = node.parent; nodes[bus].parent != Invalid;
       bus = nodes[bus].parent)
  {
    const auto &mapping = nodes[bus];
    if (!mapping.ranges.data)
    {
      return false;
    }
    if (!mapping.ranges.bytes)
    {
      continue;
    }
    const auto &up = nodes[mapping.parent];
    unsigned step = (mapping.address_cells + up.address_cells + mapping.size_cells) * 4;
    if (!step || mapping.ranges.bytes % step)
    {
      return false;
    }
    bool translated = false;
    for (unsigned offset = 0; offset < mapping.ranges.bytes; offset += step)
    {
      const auto *p = mapping.ranges.data + offset;
      unsigned long from, to, bytes;
      if (!cell(p, mapping.address_cells, from) ||
          !cell(p + mapping.address_cells * 4, up.address_cells, to) ||
          !cell(p + (mapping.address_cells + up.address_cells) * 4, mapping.size_cells,
                bytes))
      {
        return false;
      }
      if (out.address < from || out.address - from >= bytes ||
          out.bytes > bytes - (out.address - from))
      {
        continue;
      }
      unsigned long delta = out.address - from;
      if (to > ~0UL - delta || out.bytes > ~0UL - to - delta)
      {
        return false;
      }
      out.address = to + delta;
      translated = true;
      break;
    }
    if (!translated)
    {
      return false;
    }
  }
  return true;
}

bool
add_region(Description &out, Region r) {
  if (!r.bytes || r.bytes > ~0UL - r.address || out.regions_count == MaxRegions)
  {
    return false;
  }
  out.regions[out.regions_count++] = r;
  return true;
}

bool
irq(unsigned index, unsigned count, unsigned controller, unsigned &value) {
  const auto &n = nodes[index];
  value = 0;
  if (n.extended.data)
  {
    unsigned at = 0;
    bool found = false;
    while (at < n.extended.bytes)
    {
      if (n.extended.bytes - at < 4)
      {
        return false;
      }
      unsigned handle = word(n.extended.data + at);
      at += 4;
      unsigned owner = find(handle, count);
      if (owner == Invalid || !nodes[owner].interrupt_cells ||
          nodes[owner].interrupt_cells > 4 ||
          nodes[owner].interrupt_cells * 4 > n.extended.bytes - at)
      {
        return false;
      }
      if (handle == controller)
      {
        if (nodes[owner].interrupt_cells != 1 || found)
        {
          return false;
        }
        value = word(n.extended.data + at);
        found = true;
      }
      at += nodes[owner].interrupt_cells * 4;
    }
    return found;
  }
  if (!n.interrupts.data)
  {
    return true;
  }
  unsigned handle = 0;
  for (unsigned p = index; p != Invalid && !handle; p = nodes[p].parent)
  {
    handle = nodes[p].interrupt_parent;
  }
  unsigned owner = find(handle, count);
  if (handle != controller || owner == Invalid || nodes[owner].interrupt_cells != 1 ||
      n.interrupts.bytes != 4)
  {
    return false;
  }
  value = word(n.interrupts.data);
  return value != 0;
}

bool
contexts(unsigned index, unsigned count, Description &out) {
  const auto &n = nodes[index];
  if (!n.extended.data)
  {
    return false;
  }
  unsigned at = 0, context = 0;
  bool any = false;
  while (at < n.extended.bytes)
  {
    if (n.extended.bytes - at < 8)
    {
      return false;
    }
    unsigned owner = find(word(n.extended.data + at), count);
    if (owner == Invalid || nodes[owner].interrupt_cells != 1)
    {
      return false;
    }
    if (word(n.extended.data + at + 4) == 9)
    {
      unsigned cpu = nodes[owner].parent;
      if (cpu == Invalid || nodes[cpu].parent == Invalid || !nodes[cpu].reg.data)
      {
        return false;
      }
      unsigned cells = nodes[nodes[cpu].parent].address_cells;
      unsigned long hart;
      if (!cells || nodes[cpu].reg.bytes != cells * 4 ||
          !cell(nodes[cpu].reg.data, cells, hart) || hart >= MaxHarts ||
          out.supervisor_contexts[hart] != Invalid)
      {
        return false;
      }
      out.supervisor_contexts[hart] = context;
      any = true;
    }
    ++context;
    at += 8;
  }
  return any;
}

bool
pci_host(unsigned index, Description &out) {
  const auto &n = nodes[index];
  auto &pci = out.pci;
  if (pci.configuration.bytes || !region(index, pci.configuration) || !n.ranges.data ||
      !n.ranges.bytes || n.address_cells != 3 || n.size_cells != 2 ||
      n.parent == Invalid || nodes[n.parent].address_cells > 2 || !n.map.data ||
      !n.mask.data || n.mask.bytes != 16)
  {
    return false;
  }
  unsigned parent_cells = nodes[n.parent].address_cells;
  unsigned step = (3 + parent_cells + 2) * 4;
  if (n.ranges.bytes % step)
  {
    return false;
  }
  for (unsigned at = 0; at < n.ranges.bytes; at += step)
  {
    const auto *p = n.ranges.data + at;
    unsigned space = (word(p) >> 24) & 3;
    if (space != 2)
    {
      continue; // Non-prefetchable 32-bit memory window for BARs.
    }
    unsigned long child, physical, bytes;
    if (!cell(p + 4, 2, child) || !cell(p + 12, parent_cells, physical) ||
        !cell(p + (3 + parent_cells) * 4, 2, bytes) || !bytes || child != physical ||
        bytes > ~0UL - physical || physical + bytes > 0x100000000UL)
    {
      return false;
    }
    if (!pci.memory.bytes)
    {
      pci.memory = {physical, bytes};
    }
  }
  if (!pci.memory.bytes || (n.buses.data && n.buses.bytes != 8))
  {
    return false;
  }
  pci.bus_begin = n.buses.data ? word(n.buses.data) : 0;
  pci.bus_end = n.buses.data ? word(n.buses.data + 4) : 255;
  if (pci.bus_begin > pci.bus_end || pci.bus_end > 255)
  {
    return false;
  }
  pci.interrupt_map = n.map.data;
  pci.map_bytes = n.map.bytes;
  pci.controller = out.plic_phandle;
  for (unsigned i = 0; i < 4; ++i)
  {
    pci.mask[i] = word(n.mask.data + i * 4);
  }
  // Supported PCI INTx parent: zero address cells and one PLIC interrupt cell.
  if (pci.map_bytes % 24)
  {
    return false;
  }
  for (unsigned at = 0; at < pci.map_bytes; at += 24)
  {
    if (word(pci.interrupt_map + at + 16) != pci.controller ||
        !word(pci.interrupt_map + at + 20))
    {
      return false;
    }
  }
  return add_region(out, pci.configuration) && add_region(out, pci.memory);
}
} // namespace

unsigned
compatible_id(const char *name) {
  unsigned value = 2166136261U;
  while (*name)
  {
    value ^= static_cast<unsigned char>(*name++);
    value *= 16777619U;
  }
  return value ? value : 1;
}

bool
discover(const void *blob, unsigned long capacity, Description &out) {
  if (!blob || capacity < 40)
  {
    return false;
  }
  auto *data = static_cast<const Byte *>(blob);
  unsigned total = word(data + 4), structure = word(data + 8),
           strings = word(data + 12);
  unsigned string_bytes = word(data + 32), structure_bytes = word(data + 36);
  if (word(data) != 0xd00dfeed || total < 40 || total > capacity || total > MaxBlob ||
      word(data + 20) < 17 || word(data + 24) > 17 || structure < 40 || structure % 4 ||
      strings < 40 || structure > total || structure_bytes > total - structure ||
      strings > total || string_bytes > total - strings ||
      (structure < strings + string_bytes && strings < structure + structure_bytes))
  {
    return false;
  }
  unsigned stack[MaxDepth], depth = 0, count = 0, at = structure,
                            stop = structure + structure_bytes;
  bool ended = false;
  while (at < stop)
  {
    if (stop - at < 4)
    {
      return false;
    }
    unsigned token = word(data + at);
    at += 4;
    if (token == 1)
    {
      unsigned length;
      if (depth == MaxDepth || count == MaxNodes ||
          !text(data + at, stop - at, length) || (!count && length) ||
          (count && !depth))
      {
        return false;
      }
      auto &n = nodes[count];
      n = {};
      n.name = reinterpret_cast<const char *>(data + at);
      n.parent = depth ? stack[depth - 1] : Invalid;
      if (depth)
      {
        nodes[n.parent].children = true;
        n.disabled = nodes[n.parent].disabled;
      }
      stack[depth++] = count++;
      unsigned padded = (length + 1 + 3) & ~3U;
      if (padded > stop - at)
      {
        return false;
      }
      at += padded;
    }
    else if (token == 2)
    {
      if (!depth)
      {
        return false;
      }
      --depth;
    }
    else if (token == 3)
    {
      if (!depth || stop - at < 8)
      {
        return false;
      }
      auto &n = nodes[stack[depth - 1]];
      if (n.children)
      {
        return false;
      }
      unsigned bytes = word(data + at), name_at = word(data + at + 4), length;
      at += 8;
      if (bytes > stop - at || name_at >= string_bytes ||
          !text(data + strings + name_at, string_bytes - name_at, length))
      {
        return false;
      }
      const char *name = reinterpret_cast<const char *>(data + strings + name_at);
      Property value{data + at, bytes};
      if (same(name, "reg"))
      {
        n.reg = value;
      }
      else if (same(name, "compatible"))
      {
        unsigned part = 0;
        while (part < bytes)
        {
          unsigned len;
          if (!text(data + at + part, bytes - part, len) || !len)
          {
            return false;
          }
          part += len + 1;
        }
        n.compatible = value;
      }
      else if (same(name, "ranges"))
      {
        n.ranges = value;
      }
      else if (same(name, "interrupts"))
      {
        n.interrupts = value;
      }
      else if (same(name, "interrupts-extended"))
      {
        n.extended = value;
      }
      else if (same(name, "interrupt-map"))
      {
        n.map = value;
      }
      else if (same(name, "interrupt-map-mask"))
      {
        n.mask = value;
      }
      else if (same(name, "bus-range"))
      {
        n.buses = value;
      }
      else if (same(name, "status"))
      {
        unsigned len;
        if (!text(data + at, bytes, len))
        {
          return false;
        }
        n.disabled |= !same(reinterpret_cast<const char *>(data + at), "okay") &&
                      !same(reinterpret_cast<const char *>(data + at), "ok");
      }
      else if (same(name, "#address-cells") || same(name, "#size-cells") ||
               same(name, "#interrupt-cells") || same(name, "interrupt-parent") ||
               same(name, "phandle") || same(name, "linux,phandle") ||
               same(name, "riscv,ndev"))
      {
        if (bytes != 4)
        {
          return false;
        }
        unsigned v = word(data + at);
        if (same(name, "#address-cells"))
        {
          if (v > 3)
          {
            return false;
          }
          n.address_cells = v;
        }
        else if (same(name, "#size-cells"))
        {
          if (v > 2)
          {
            return false;
          }
          n.size_cells = v;
        }
        else if (same(name, "#interrupt-cells"))
        {
          if (v > 4)
          {
            return false;
          }
          n.interrupt_cells = v;
        }
        else if (same(name, "interrupt-parent"))
        {
          n.interrupt_parent = v;
        }
        else if (same(name, "riscv,ndev"))
        {
          n.sources = v;
        }
        else
        {
          if (!v || (n.phandle && n.phandle != v))
          {
            return false;
          }
          n.phandle = v;
        }
      }
      unsigned padded = (bytes + 3) & ~3U;
      if (padded < bytes || padded > stop - at)
      {
        return false;
      }
      at += padded;
    }
    else if (token == 9)
    {
      if (depth || !count)
      {
        return false;
      }
      ended = true;
      break;
    }
    else if (token != 4)
    {
      return false;
    }
  }
  if (!ended)
  {
    return false;
  }
  for (unsigned i = 0; i < count; ++i)
  {
    for (unsigned j = 0; nodes[i].phandle && j < i; ++j)
    {
      if (nodes[i].phandle == nodes[j].phandle)
      {
        return false;
      }
    }
  }
  // Avoid an aggregate Description temporary: it is over 6 KiB, while
  // firmware entry has only a 4 KiB stack. Initialize bounded fields in place.
  for (auto &device : out.devices) { device = {}; }
  for (auto &entry : out.regions) { entry = {}; }
  out.devices_count = out.regions_count = 0;
  out.plic = out.rtc = out.timer = {};
  out.plic_phandle = out.interrupt_sources = 0;
  out.pci = {};
  for (auto &context : out.supervisor_contexts)
  {
    context = Invalid;
  }
  for (unsigned i = 0; i < count; ++i)
  {
    auto &n = nodes[i];
    if (n.disabled ||
        (!compatible(n, "riscv,plic0") && !compatible(n, "sifive,plic-1.0.0")))
    {
      continue;
    }
    if (out.plic.bytes || !region(i, out.plic) || !n.phandle ||
        n.interrupt_cells != 1 || !n.sources || n.sources >= 1024)
    {
      return false;
    }
    out.plic_phandle = n.phandle;
    out.interrupt_sources = n.sources;
    if (!contexts(i, count, out) || !add_region(out, out.plic))
    {
      return false;
    }
  }
  if (!out.plic.bytes)
  {
    return false;
  }
  for (unsigned i = 0; i < count; ++i)
  {
    auto &n = nodes[i];
    if (n.disabled || !n.compatible.bytes || !n.reg.bytes)
    {
      continue;
    }
    if (compatible(n, "riscv,plic0") || compatible(n, "sifive,plic-1.0.0") ||
        compatible(n, "riscv,cpu-intc") || compatible(n, "riscv"))
    {
      continue;
    }
    if (compatible(n, "pci-host-ecam-generic"))
    {
      if (!pci_host(i, out))
      {
        return false;
      }
      continue;
    }
    if (compatible(n, "google,goldfish-rtc"))
    {
      if (out.rtc.bytes || !region(i, out.rtc) || !add_region(out, out.rtc))
      {
        return false;
      }
      continue;
    }
    if (compatible(n, "riscv,clint0"))
    {
      if (out.timer.bytes || !region(i, out.timer) || !add_region(out, out.timer))
      {
        return false;
      }
      continue;
    }
    // Non-MMIO nodes (e.g. CPU descriptions) do not have a size cell.
    if (n.parent == Invalid || !nodes[n.parent].size_cells)
    {
      continue;
    }
    Device device;
    bool virtio = compatible(n, "virtio,mmio");
    bool serial = compatible(n, "ns16550a") || compatible(n, "ns16550");
    if (!virtio && !serial && !n.interrupts.data && !n.extended.data)
    {
      continue;
    }
    if (!region(i, device.registers) || !irq(i, count, out.plic_phandle, device.irq) ||
        device.irq > out.interrupt_sources || ((virtio || serial) && !device.irq) ||
        out.devices_count == MaxDevices)
    {
      return false;
    }
    device.kind = virtio   ? DeviceKind::Virtio
                  : serial ? DeviceKind::Serial
                           : DeviceKind::Platform;
    device.type =
        serial   ? 1
        : virtio ? 0
                 : compatible_id(reinterpret_cast<const char *>(n.compatible.data));
    copy_name(device.name, n.name);
    out.devices[out.devices_count++] = device;
    if (!add_region(out, device.registers))
    {
      return false;
    }
  }
  // Stable resource indices are independent of DT structure order.
  for (unsigned i = 1; i < out.devices_count; ++i)
  {
    auto value = out.devices[i];
    unsigned j = i;
    auto before = [](const Device &a, const Device &b) {
      if (a.kind != b.kind)
      {
        return unsigned(a.kind) < unsigned(b.kind);
      }
      return a.registers.address < b.registers.address;
    };
    while (j && before(value, out.devices[j - 1]))
    {
      out.devices[j] = out.devices[j - 1];
      --j;
    }
    out.devices[j] = value;
  }
  // The mapping list is page-granular and disjoint; adjacent device register
  // windows can share one page without reserving it twice in the kernel pool.
  for (unsigned i = 0; i < out.regions_count; ++i)
  {
    auto &r = out.regions[i];
    if (r.address + r.bytes > ~0UL - 4095)
    {
      return false;
    }
    unsigned long end = (r.address + r.bytes + 4095) & ~4095UL;
    r.address &= ~4095UL;
    r.bytes = end - r.address;
  }
  for (unsigned i = 1; i < out.regions_count; ++i)
  {
    auto value = out.regions[i];
    unsigned j = i;
    while (j && value.address < out.regions[j - 1].address)
    {
      out.regions[j] = out.regions[j - 1];
      --j;
    }
    out.regions[j] = value;
  }
  unsigned used = 0;
  for (unsigned i = 0; i < out.regions_count; ++i)
  {
    auto value = out.regions[i];
    if (used &&
        value.address <= out.regions[used - 1].address + out.regions[used - 1].bytes)
    {
      auto &last = out.regions[used - 1];
      unsigned long end = value.address + value.bytes;
      if (end > last.address + last.bytes)
      {
        last.bytes = end - last.address;
      }
    }
    else
    {
      out.regions[used++] = value;
    }
  }
  out.regions_count = used;
  return out.devices_count != 0;
}

bool
initialize(const void *blob) {
  return discover(blob, MaxBlob, system);
}

const Description &
description() {
  return system;
}

unsigned
pci_interrupt(const PciHost &pci, unsigned slot, unsigned function, unsigned pin) {
  if (slot >= 32 || function >= 8 || !pin || pin > 4)
  {
    return 0;
  }
  unsigned key[4] = {(pci.bus_begin << 16) | (slot << 11) | (function << 8), 0, 0, pin};
  for (unsigned at = 0; at < pci.map_bytes; at += 24)
  {
    bool match = true;
    for (unsigned i = 0; i < 4; ++i)
    {
      match &= (word(pci.interrupt_map + at + i * 4) & pci.mask[i]) ==
               (key[i] & pci.mask[i]);
    }
    if (match)
    {
      return word(pci.interrupt_map + at + 20);
    }
  }
  return 0;
}
} // namespace xv::hardware
