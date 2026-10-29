module;
#include "kernel/support/flow.h"

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

namespace flow = xv::flow;

struct RecordFlow
{
  struct Context
  {
    const Node &node;
    BootMemoryInfo &info;
    unsigned stride;
    MemorySize offset = 0, address = 0, size = 0;
    bool ok = false, result = false;
  };

  static bool valid(Context &c) noexcept
  {
    return c.node.reg && c.stride && !(c.node.reg_size % c.stride) &&
           c.node.parent_address >= 1 && c.node.parent_address <= 2 &&
           c.node.parent_size >= 1 && c.node.parent_size <= 2;
  }

  static bool remaining(Context &c) noexcept
  {
    return c.offset < c.node.reg_size;
  }

  static void tuple(Context &c) noexcept
  {
    c.ok = cells(c.node.reg + c.offset, c.node.parent_address, c.address) &&
           cells(c.node.reg + c.offset + c.node.parent_address * 4, c.node.parent_size,
                 c.size) &&
           c.size != 0 && c.size <= ~MemorySize(0) - c.address;
  }

  static bool ok(Context &c) noexcept
  {
    return c.ok;
  }

  static void publish(Context &c) noexcept
  {
    PhysicalMemoryRange range(PhysicalAddress(c.address),
                              PhysicalAddress(c.address + c.size));
    c.ok = c.node.memory ? c.info.set_ram(range) : c.info.reserve(range, c.node.no_map);
  }

  static void advance(Context &c) noexcept
  {
    c.offset += c.stride;
  }

  static void success(Context &c) noexcept
  {
    c.result = true;
  }
  struct Valid;
  struct Remaining;
  struct Tuple;
  struct Publish;
  struct Advance;
  struct Success;
  using Graph = flow::graph<
      Valid,
      flow::types<
          flow::node<
              Valid, flow::noop,
              flow::branch<flow::condition<&valid>, flow::go<Remaining>, flow::stop>>,
          flow::node<Remaining, flow::noop,
                     flow::branch<flow::condition<&remaining>, flow::go<Tuple>,
                                  flow::go<Success>>>,
          flow::node<Tuple, flow::action<&tuple>,
                     flow::branch<flow::condition<&ok>, flow::go<Publish>, flow::stop>>,
          flow::node<Publish, flow::action<&publish>,
                     flow::branch<flow::condition<&ok>, flow::go<Advance>, flow::stop>>,
          flow::node<Advance, flow::action<&advance>, flow::go<Remaining>>,
          flow::node<Success, flow::action<&success>, flow::stop>>>;
};

bool record(const Node &node, BootMemoryInfo &info)
{
  if (node.disabled || (!node.memory && !node.reserved))
  {
    return true;
  }
  RecordFlow::Context context{node, info, (node.parent_address + node.parent_size) * 4};
  (void)RecordFlow::Graph::run<0>(context);
  return context.result;
}
} // namespace

namespace flow = xv::flow;

struct FdtMemorySource::DiscoverFlow
{
  struct Context
  {
    const Byte *blob;
    MemorySize total, structure, strings, string_size, stop, cursor;
    BootMemoryInfo &found, &output;
    Node *nodes;
    unsigned depth = 0, token = 0;
    bool root_seen = false, root_closed = false, reservation_end = false,
         finished = false, ok = false, result = false;
  };

  static void reservation(Context &c) noexcept
  {
    c.ok = false;
    if (!fits(c.cursor, 16, c.total) ||
        (c.cursor < c.stop && c.cursor + 16 > c.structure) ||
        (c.cursor < c.strings + c.string_size && c.cursor + 16 > c.strings))
    {
      return;
    }
    MemorySize address = be64(c.blob + c.cursor), size = be64(c.blob + c.cursor + 8);
    c.cursor += 16;
    c.reservation_end = !address && !size;
    c.ok = c.reservation_end ||
           (size <= ~MemorySize(0) - address &&
            c.found.reserve(PhysicalMemoryRange(PhysicalAddress(address),
                                                PhysicalAddress(address + size))));
  }

  static bool ok(Context &c) noexcept
  {
    return c.ok;
  }

  static bool reservations_done(Context &c) noexcept
  {
    return c.reservation_end;
  }

  static void start(Context &c) noexcept
  {
    c.cursor = c.structure;
  }

  static void read(Context &c) noexcept
  {
    c.ok = fits(c.cursor, 4, c.stop);
    if (c.ok)
    {
      c.token = be32(c.blob + c.cursor);
      c.cursor += 4;
    }
  }

  static bool begin_token(Context &c) noexcept
  {
    return c.token == 1;
  }

  static bool end_token(Context &c) noexcept
  {
    return c.token == 2;
  }

  static bool property_token(Context &c) noexcept
  {
    return c.token == 3;
  }

  static bool nop_token(Context &c) noexcept
  {
    return c.token == 4;
  }

  static bool finish_token(Context &c) noexcept
  {
    return c.token == 9;
  }

  static void begin(Context &c) noexcept
  {
    c.ok = true;

    if (c.root_closed || c.depth == 24)
    {
      {
        c.ok = false;
        return;
      }
    }
    MemorySize length;
    if (!cstring(c.blob + c.cursor, c.stop - c.cursor, length))
    {
      {
        c.ok = false;
        return;
      }
    }
    const char *name = reinterpret_cast<const char *>(c.blob + c.cursor);
    Node node;
    if (c.depth)
    {
      c.nodes[c.depth - 1].children_started = true;
      node.parent_address = c.nodes[c.depth - 1].child_address;
      node.parent_size = c.nodes[c.depth - 1].child_size;
      node.reserved = c.nodes[c.depth - 1].reserved_container;
      node.disabled = c.nodes[c.depth - 1].disabled;
    }
    else
    {
      if (c.root_seen || length != 0)
      {
        {
          c.ok = false;
          return;
        }
      }
      c.root_seen = true;
    }
    node.memory = c.depth == 1 && node_name(name, "memory");
    node.reserved_container = c.depth == 1 && same(name, "reserved-memory");
    c.nodes[c.depth++] = node;
    MemorySize padded = (length + 1 + 3) & ~MemorySize(3);
    if (!fits(c.cursor, padded, c.stop))
    {
      {
        c.ok = false;
        return;
      }
    }
    c.cursor += padded;
  }

  static void end(Context &c) noexcept
  {
    c.ok = true;

    if (!c.depth || !record(c.nodes[c.depth - 1], c.found))
    {
      {
        c.ok = false;
        return;
      }
    }
    if (--c.depth == 0)
    {
      c.root_closed = true;
    }
  }

  static void property(Context &c) noexcept
  {
    c.ok = true;

    if (!c.depth || c.nodes[c.depth - 1].children_started || !fits(c.cursor, 8, c.stop))
    {
      {
        c.ok = false;
        return;
      }
    }
    MemorySize size = be32(c.blob + c.cursor),
               name_offset = be32(c.blob + c.cursor + 4);
    c.cursor += 8;
    MemorySize name_length;
    if (name_offset >= c.string_size || !fits(c.cursor, size, c.stop) ||
        !cstring(c.blob + c.strings + name_offset, c.string_size - name_offset,
                 name_length))
    {
      {
        c.ok = false;
        return;
      }
    }
    const char *name = reinterpret_cast<const char *>(c.blob + c.strings + name_offset);
    auto &node = c.nodes[c.depth - 1];
    if (same(name, "#address-cells"))
    {
      if (size != 4)
      {
        {
          c.ok = false;
          return;
        }
      }
      node.child_address = be32(c.blob + c.cursor);
    }
    else if (same(name, "#size-cells"))
    {
      if (size != 4)
      {
        {
          c.ok = false;
          return;
        }
      }
      node.child_size = be32(c.blob + c.cursor);
    }
    else if (same(name, "reg"))
    {
      if (node.reg)
      {
        {
          c.ok = false;
          return;
        }
      }
      node.reg = c.blob + c.cursor;
      node.reg_size = size;
    }
    else if (same(name, "device_type") || same(name, "status"))
    {
      MemorySize length;
      if (!cstring(c.blob + c.cursor, size, length))
      {
        {
          c.ok = false;
          return;
        }
      }
      const char *value = reinterpret_cast<const char *>(c.blob + c.cursor);
      if (same(name, "device_type") && same(value, "memory"))
      {
        if (c.depth != 2)
        {
          {
            c.ok = false;
            return;
          } // no bus address translation in this provider
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
        {
          c.ok = false;
          return;
        }
      }
      node.no_map = true;
    }
    MemorySize padded = (size + 3) & ~MemorySize(3);
    if (!fits(c.cursor, padded, c.stop))
    {
      {
        c.ok = false;
        return;
      }
    }
    c.cursor += padded;
  }

  static void finish(Context &c) noexcept
  {
    c.finished = c.root_closed && !c.depth;
  }

  static bool complete(Context &c) noexcept
  {
    return c.finished && !c.found.ram().empty();
  }

  static void blob(Context &c) noexcept
  {
    c.ok = true;
    auto blob_address = PhysicalAddress::from_pointer(c.blob);
    // Only reserve the blob when it actually resides in the physical RAM bank.
    // Host tests deliberately supply a separate native buffer.
    if (c.found.ram().contains(blob_address))
    {
      if (!c.found.ram().contains_block(blob_address, c.total) ||
          !c.found.reserve(
              PhysicalMemoryRange(blob_address, blob_address.advanced(c.total))))
      {
        {
          c.ok = false;
          return;
        }
      }
    }
  }

  static void publish(Context &c) noexcept
  {
    c.output = c.found;
    c.result = true;
  }
  struct Reservation;
  struct Start;
  struct Read;
  struct Begin;
  struct End;
  struct Property;
  struct Finish;
  struct Blob;
  struct Publish;
  using TokenNext = flow::branch<
      flow::condition<&begin_token>, flow::go<Begin>,
      flow::branch<
          flow::condition<&end_token>, flow::go<End>,
          flow::branch<flow::condition<&property_token>, flow::go<Property>,
                       flow::branch<flow::condition<&nop_token>, flow::go<Read>,
                                    flow::branch<flow::condition<&finish_token>,
                                                 flow::go<Finish>, flow::stop>>>>>;
  using Graph = flow::graph<
      Reservation,
      flow::types<
          flow::node<Reservation, flow::action<&reservation>,
                     flow::branch<flow::condition<&ok>,
                                  flow::branch<flow::condition<&reservations_done>,
                                               flow::go<Start>, flow::go<Reservation>>,
                                  flow::stop>>,
          flow::node<Start, flow::action<&start>, flow::go<Read>>,
          flow::node<Read, flow::action<&read>,
                     flow::branch<flow::condition<&ok>, TokenNext, flow::stop>>,
          flow::node<Begin, flow::action<&begin>,
                     flow::branch<flow::condition<&ok>, flow::go<Read>, flow::stop>>,
          flow::node<End, flow::action<&end>,
                     flow::branch<flow::condition<&ok>, flow::go<Read>, flow::stop>>,
          flow::node<Property, flow::action<&property>,
                     flow::branch<flow::condition<&ok>, flow::go<Read>, flow::stop>>,
          flow::node<
              Finish, flow::action<&finish>,
              flow::branch<flow::condition<&complete>, flow::go<Blob>, flow::stop>>,
          flow::node<Blob, flow::action<&blob>,
                     flow::branch<flow::condition<&ok>, flow::go<Publish>, flow::stop>>,
          flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

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
  Node nodes[24]{};
  DiscoverFlow::Context context{
      blob_,        total, structure, strings, string_size, structure + structure_size,
      reservations, found, result,    nodes};
  (void)DiscoverFlow::Graph::run<0>(context);
  return context.result;
}
} // namespace xv::memory
