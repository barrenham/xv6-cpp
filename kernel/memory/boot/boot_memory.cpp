module xv.memory;

import :boot;

namespace xv::memory
{
namespace
{
using Byte = unsigned char;

unsigned be32(const Byte *p)
{
  return (unsigned(p[0]) << 24) | (unsigned(p[1]) << 16) | (unsigned(p[2]) << 8) |
         unsigned(p[3]);
}

MemorySize be64(const Byte *p)
{
  return (MemorySize(be32(p)) << 32) | be32(p + 4);
}

bool same(const char *left, const char *right)
{
  while (*left && *left == *right)
  {
    ++left;
    ++right;
  }
  return *left == *right;
}

bool node_name(const char *name, const char *wanted)
{
  while (*wanted && *name == *wanted)
  {
    ++name;
    ++wanted;
  }
  return !*wanted && (!*name || *name == '@');
}

bool fits(MemorySize offset, MemorySize length, MemorySize total)
{
  return offset <= total && length <= total - offset;
}

bool cstring(const Byte *p, MemorySize available, MemorySize &length)
{
  for (length = 0; length < available; ++length)
  {
    if (p[length] == 0)
    {
      return true;
    }
  }
  return false;
}

bool cells(const Byte *p, unsigned count, MemorySize &value)
{
  if (count != 1 && count != 2)
  {
    return false;
  }
  value = count == 1 ? be32(p) : be64(p);
  return true;
}

struct Node
{
  const Byte *reg = nullptr;
  MemorySize reg_size = 0;
  unsigned parent_address = 2, parent_size = 1;
  unsigned child_address = 2, child_size = 1;
  bool memory = false, reserved_container = false, reserved = false;
  bool disabled = false, no_map = false, children_started = false;
};

bool record(const Node &node, BootMemoryInfo &info)
{
  if (node.disabled || (!node.memory && !node.reserved))
  {
    return true;
  }
  unsigned stride = (node.parent_address + node.parent_size) * 4;
  if (!node.reg || !stride || node.reg_size % stride != 0 || node.parent_address < 1 ||
      node.parent_address > 2 || node.parent_size < 1 || node.parent_size > 2)
  {
    return false;
  }
  for (MemorySize off = 0; off < node.reg_size; off += stride)
  {
    MemorySize address, size;
    if (!cells(node.reg + off, node.parent_address, address) ||
        !cells(node.reg + off + node.parent_address * 4, node.parent_size, size) ||
        size == 0 || size > ~MemorySize(0) - address)
    {
      return false;
    }
    PhysicalMemoryRange range(PhysicalAddress(address),
                              PhysicalAddress(address + size));
    if (node.memory ? !info.set_ram(range) : !info.reserve(range, node.no_map))
    {
      return false;
    }
  }
  return true;
}
} // namespace

bool FdtMemorySource::discover(BootMemoryInfo &result)
{
  // The caller supplies an accessible blob span. No parser allocation is needed.
  if (!blob_ || capacity_ < 40 || be32(blob_) != 0xd00dfeed)
  {
    return false;
  }
  MemorySize total = be32(blob_ + 4), structure = be32(blob_ + 8);
  MemorySize strings = be32(blob_ + 12), reservations = be32(blob_ + 16);
  unsigned version = be32(blob_ + 20), compatible = be32(blob_ + 24);
  MemorySize string_size = be32(blob_ + 32), structure_size = be32(blob_ + 36);
  if (total < 40 || total > capacity_ || total > MaxBlobSize || version < 17 ||
      compatible > 17 || structure < 40 || strings < 40 || reservations < 40 ||
      structure % 4 || reservations % 8 || !fits(structure, structure_size, total) ||
      !fits(strings, string_size, total) ||
      !(structure + structure_size <= strings || strings + string_size <= structure))
  {
    return false;
  }
  BootMemoryInfo found;
  // Reservation entries are big-endian pairs of 64-bit address and size.
  MemorySize cursor = reservations;
  for (;;)
  {
    if (!fits(cursor, 16, total) ||
        (cursor < structure + structure_size && cursor + 16 > structure) ||
        (cursor < strings + string_size && cursor + 16 > strings))
    {
      return false;
    }
    MemorySize address = be64(blob_ + cursor), size = be64(blob_ + cursor + 8);
    cursor += 16;
    if (!address && !size)
    {
      break;
    }
    if (size > ~MemorySize(0) - address ||
        !found.reserve(PhysicalMemoryRange(PhysicalAddress(address),
                                           PhysicalAddress(address + size))))
    {
      return false;
    }
  }
  Node nodes[24]{};
  unsigned depth = 0;
  bool root_seen = false, root_closed = false, finished = false;
  cursor = structure;
  const MemorySize stop = structure + structure_size;
  while (fits(cursor, 4, stop))
  {
    unsigned token = be32(blob_ + cursor);
    cursor += 4;
    if (token == 1)
    {
      if (root_closed || depth == 24)
      {
        return false;
      }
      MemorySize length;
      if (!cstring(blob_ + cursor, stop - cursor, length))
      {
        return false;
      }
      const char *name = reinterpret_cast<const char *>(blob_ + cursor);
      Node node;
      if (depth)
      {
        nodes[depth - 1].children_started = true;
        node.parent_address = nodes[depth - 1].child_address;
        node.parent_size = nodes[depth - 1].child_size;
        node.reserved = nodes[depth - 1].reserved_container;
        node.disabled = nodes[depth - 1].disabled;
      }
      else
      {
        if (root_seen || length != 0)
        {
          return false;
        }
        root_seen = true;
      }
      node.memory = depth == 1 && node_name(name, "memory");
      node.reserved_container = depth == 1 && same(name, "reserved-memory");
      nodes[depth++] = node;
      MemorySize padded = (length + 1 + 3) & ~MemorySize(3);
      if (!fits(cursor, padded, stop))
      {
        return false;
      }
      cursor += padded;
    }
    else if (token == 2)
    {
      if (!depth || !record(nodes[depth - 1], found))
      {
        return false;
      }
      if (--depth == 0)
      {
        root_closed = true;
      }
    }
    else if (token == 3)
    {
      if (!depth || nodes[depth - 1].children_started || !fits(cursor, 8, stop))
      {
        return false;
      }
      MemorySize size = be32(blob_ + cursor), name_offset = be32(blob_ + cursor + 4);
      cursor += 8;
      MemorySize name_length;
      if (name_offset >= string_size || !fits(cursor, size, stop) ||
          !cstring(blob_ + strings + name_offset, string_size - name_offset,
                   name_length))
      {
        return false;
      }
      const char *name = reinterpret_cast<const char *>(blob_ + strings + name_offset);
      auto &node = nodes[depth - 1];
      if (same(name, "#address-cells"))
      {
        if (size != 4)
        {
          return false;
        }
        node.child_address = be32(blob_ + cursor);
      }
      else if (same(name, "#size-cells"))
      {
        if (size != 4)
        {
          return false;
        }
        node.child_size = be32(blob_ + cursor);
      }
      else if (same(name, "reg"))
      {
        if (node.reg)
        {
          return false;
        }
        node.reg = blob_ + cursor;
        node.reg_size = size;
      }
      else if (same(name, "device_type") || same(name, "status"))
      {
        MemorySize length;
        if (!cstring(blob_ + cursor, size, length))
        {
          return false;
        }
        const char *value = reinterpret_cast<const char *>(blob_ + cursor);
        if (same(name, "device_type") && same(value, "memory"))
        {
          if (depth != 2)
          {
            return false; // no bus address translation in this provider
          }
          node.memory = true;
        }
        if (same(name, "status") && !same(value, "okay") && !same(value, "ok"))
        {
          node.disabled = true;
        }
      }
      else if (same(name, "no-map"))
      {
        if (size != 0)
        {
          return false;
        }
        node.no_map = true;
      }
      MemorySize padded = (size + 3) & ~MemorySize(3);
      if (!fits(cursor, padded, stop))
      {
        return false;
      }
      cursor += padded;
    }
    else if (token == 4)
    {
      continue;
    }
    else if (token == 9)
    {
      if (!root_closed || depth)
      {
        return false;
      }
      finished = true;
      break;
    }
    else
    {
      return false;
    }
  }
  if (!finished || found.ram().empty())
  {
    return false;
  }
  auto blob_address = PhysicalAddress::from_pointer(blob_);
  // Only reserve the blob when it actually resides in the physical RAM bank.
  // Host tests deliberately supply a separate native buffer.
  if (found.ram().contains(blob_address))
  {
    if (!found.ram().contains_block(blob_address, total) ||
        !found.reserve(PhysicalMemoryRange(blob_address, blob_address.advanced(total))))
    {
      return false;
    }
  }
  result = found;
  return true;
}
} // namespace xv::memory
