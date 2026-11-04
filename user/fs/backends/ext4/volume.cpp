#include "volume.h"

namespace userfs::ext4
{
namespace
{
struct CrcTable
{
  u32 values[256]{};
  constexpr CrcTable()
  {
    for (unsigned i = 0; i < 256; ++i)
    {
      u32 value = i;
      for (unsigned bit = 0; bit < 8; ++bit) value = (value >> 1) ^ ((value & 1) ? 0x82f63b78U : 0);
      values[i] = value;
    }
  }
};
constexpr CrcTable checksum_table{};
class Page
{
public:
  explicit Page(Io &io) : io_(io), bytes_(static_cast<u8 *>(io.allocate_page())) {}
  ~Page() { if (bytes_) io_.free_page(bytes_); }
  u8 *get() const { return bytes_; }
  Page(const Page &) = delete;
  Page &operator=(const Page &) = delete;
private:
  Io &io_;
  u8 *bytes_;
};
u16 crc16(u16 seed, const void *data, unsigned size)
{
  auto *bytes = static_cast<const u8 *>(data);
  for (unsigned i = 0; i < size; ++i)
  {
    seed ^= bytes[i];
    for (unsigned bit = 0; bit < 8; ++bit) seed = (seed >> 1) ^ ((seed & 1) ? 0xa001 : 0);
  }
  return seed;
}
}
u32 Volume::crc(u32 seed, const void *data, unsigned size)
{
  auto *bytes = static_cast<const u8 *>(data);
  for (unsigned i = 0; i < size; ++i)
  {
    seed = (seed >> 8) ^ checksum_table.values[(seed ^ bytes[i]) & 255];
  }
  return seed;
}
u32 Volume::word_crc(u32 seed, u32 value)
{
  u8 bytes[4]; put32(bytes, value);
  return crc(seed, bytes, sizeof(bytes));
}
bool Volume::block(u64 number, void *data)
{
  if (!number || number >= geometry_.blocks) return fail(Error::Corrupt);
  return io_.read(number * geometry_.block_size, data, geometry_.block_size) || fail(Error::Io);
}
bool Volume::mount()
{
  ready_ = false; error_ = Error::None; geometry_ = {};
  Page memory(io_);
  auto *s = memory.get();
  if (!s) return fail(Error::NoMemory);
  if (!io_.read(1024, s, 1024)) return fail(Error::Io);
  if (le16(s + 56) != Magic) return fail(Error::Invalid);
  geometry_.compatible = le32(s + 92);
  geometry_.incompatible = le32(s + 96);
  geometry_.readonly = le32(s + 100);
  // Only layouts interpreted below. In particular, never replay a dirty journal.
  constexpr u32 supported_incompat = 0x2 | 0x40 | 0x80 | 0x200 | 0x2000;
  constexpr u32 supported_readonly = 0x1 | 0x2 | 0x8 | 0x10 | 0x20 | 0x40 | 0x400;
  if (!(geometry_.incompatible & 0x40) || (geometry_.incompatible & ~supported_incompat) ||
      (geometry_.readonly & ~supported_readonly) || le32(s + 72) || le32(s + 76) != 1 ||
      le16(s + 58) != 1 || le32(s + 232)) return fail(Error::Unsupported);
  metadata_ = geometry_.readonly & 0x400;
  gdt_checksum_ = geometry_.readonly & 0x10;
  if (metadata_ && (s[373] != 1 || crc(~0U, s, 1020) != le32(s + 1020))) return fail(Error::Corrupt);
  copy(uuid_, s + 104, 16);
  checksum_seed_ = geometry_.incompatible & 0x2000 ? le32(s + 624) : crc(~0U, uuid_, 16);
  if (le32(s + 24) > 2 || le32(s + 28) != le32(s + 24)) return fail(Error::Unsupported);
  geometry_.block_size = 1024U << le32(s + 24);
  geometry_.blocks = le32(s + 4);
  if (geometry_.incompatible & 0x80) geometry_.blocks |= u64(le32(s + 336)) << 32;
  geometry_.inodes = le32(s);
  geometry_.first_data = le32(s + 20);
  geometry_.blocks_per_group = le32(s + 32);
  geometry_.inodes_per_group = le32(s + 40);
  geometry_.inode_size = le16(s + 88);
  geometry_.descriptor_size = geometry_.incompatible & 0x80 ? le16(s + 254) : 32;
  const auto &g = geometry_;
  if (!g.inodes || g.blocks <= g.first_data || g.blocks > io_.size_bytes() / g.block_size ||
      g.first_data != unsigned(g.block_size == 1024) || !g.blocks_per_group ||
      g.blocks_per_group > g.block_size * 8 || !g.inodes_per_group ||
      g.inodes_per_group > g.block_size * 8 || g.inode_size < 128 ||
      g.inode_size > g.block_size || (g.inode_size & (g.inode_size - 1)) ||
      g.descriptor_size < ((g.incompatible & 0x80) ? 64u : 32u) ||
      g.descriptor_size > g.block_size || (g.descriptor_size & (g.descriptor_size - 1)))
    return fail(Error::Corrupt);
  u64 groups = (g.blocks - g.first_data + g.blocks_per_group - 1) / g.blocks_per_group;
  if (groups > 0xffffffffU || (u64(g.inodes) + g.inodes_per_group - 1) / g.inodes_per_group > groups)
    return fail(Error::Corrupt);
  geometry_.groups = groups;
  ready_ = true;
  Inode root;
  if (!inode(2, root) || (root.mode() & 0xf000) != Directory)
  { ready_ = false; if (error_ == Error::None) fail(Error::Corrupt); return false; }
  if (root.size() < 24 || !directory_block(root, 0, s))
  { ready_ = false; if (error_ == Error::None) fail(Error::Corrupt); return false; }
  return true;
}
bool Volume::group(u32 number, u8 *out)
{
  const auto &g = geometry_;
  if (number >= g.groups) return fail(Error::Corrupt);
  u64 offset = u64(g.first_data + 1) * g.block_size + u64(number) * g.descriptor_size;
  if (!io_.read(offset, out, g.descriptor_size)) return fail(Error::Io);
  u16 expected = le16(out + 30);
  if (metadata_)
  {
    put16(out + 30, 0);
    u32 actual = crc(word_crc(checksum_seed_, number), out, g.descriptor_size);
    put16(out + 30, expected);
    if (u16(actual) != expected) return fail(Error::Corrupt);
  }
  else if (gdt_checksum_)
  {
    u8 bytes[4]; put32(bytes, number);
    u16 actual = crc16(crc16(~u16(0), uuid_, 16), bytes, 4);
    actual = crc16(actual, out, 30);
    if (g.descriptor_size > 32) actual = crc16(actual, out + 32, g.descriptor_size - 32);
    if (actual != expected) return fail(Error::Corrupt);
  }
  return true;
}
bool Volume::inode(u32 number, Inode &out)
{
  error_ = Error::None;
  const auto &g = geometry_;
  if (!ready_ || !number || number > g.inodes) return fail(Error::Invalid);
  Page memory(io_), inode_memory(io_);
  auto *descriptor = memory.get(), *data = inode_memory.get();
  if (!descriptor || !data) return fail(Error::NoMemory);
  u32 group_number = (number - 1) / g.inodes_per_group;
  if (!group(group_number, descriptor)) return false;
  if (le16(descriptor + 18) & 1) return fail(Error::Corrupt);
  u64 table = le32(descriptor + 8), bitmap = le32(descriptor + 4);
  if (g.incompatible & 0x80)
  { table |= u64(le32(descriptor + 40)) << 32; bitmap |= u64(le32(descriptor + 36)) << 32; }
  if (!table || table >= g.blocks ||
      (u64(g.inodes_per_group) * g.inode_size + g.block_size - 1) / g.block_size > g.blocks - table)
    return fail(Error::Corrupt);
  if (!block(bitmap, data)) return false;
  if (metadata_)
  {
    u32 wanted = le16(descriptor + 26);
    u32 actual = crc(checksum_seed_, data, (g.inodes_per_group + 7) / 8);
    if (g.descriptor_size >= 64) wanted |= u32(le16(descriptor + 58)) << 16;
    else actual &= 0xffff;
    if (actual != wanted) return fail(Error::Corrupt);
  }
  unsigned slot = (number - 1) % g.inodes_per_group;
  if (!(data[slot / 8] & (1U << (slot % 8)))) return fail(Error::Corrupt);
  u64 offset = table * g.block_size + u64(slot) * g.inode_size;
  if (!io_.read(offset, data, g.inode_size)) return fail(Error::Io);
  unsigned extra = g.inode_size > 128 ? le16(data + 128) : 0;
  if (extra > g.inode_size - 128 || (extra & 3)) return fail(Error::Corrupt);
  u32 seed = word_crc(word_crc(checksum_seed_, number), le32(data + 100));
  if (metadata_)
  {
    u32 wanted = le16(data + 124);
    bool high = extra >= 4;
    if (high) wanted |= u32(le16(data + 130)) << 16;
    put16(data + 124, 0);
    if (high) put16(data + 130, 0);
    u32 actual = crc(seed, data, g.inode_size);
    if (!high) actual &= 0xffff;
    if (actual != wanted) return fail(Error::Corrupt);
  }
  // No ACL implementation yet: refuse both external and in-inode xattrs.
  unsigned ea = 128 + extra;
  if (le32(data + 104) || le16(data + 118) ||
      (ea + 8 <= g.inode_size && le32(data + ea) == 0xea020000 && le32(data + ea + 4)))
    return fail(Error::Unsupported);
  if (!le16(data) || le32(data + 20) || (le32(data + 32) & (0x4U | 0x800U | 0x10000000U | 0xc0000000U | 0x00300000U)))
    return fail(Error::Unsupported);
  out = {}; out.number = number; out.seed = seed;
  copy(out.raw, data, g.inode_size < sizeof(out.raw) ? g.inode_size : sizeof(out.raw));
  return true;
}
bool Volume::mapping(const Inode &inode, u32 logical, u64 &physical)
{
  physical = 0;
  Page memory(io_);
  auto *scratch = memory.get();
  if (!scratch) return fail(Error::NoMemory);
  if (!(inode.flags() & 0x80000))
  {
    u64 index = logical, span = 1;
    unsigned level = 0, pointer = 0, fanout = geometry_.block_size / 4;
    if (index < 12) physical = le32(inode.raw + 40 + index * 4);
    else
    {
      index -= 12;
      for (level = 1; level <= 3; ++level)
      {
        span *= fanout;
        if (index < span) { pointer = 11 + level; break; }
        index -= span;
      }
      if (level > 3) return fail(Error::Corrupt);
      physical = le32(inode.raw + 40 + pointer * 4);
      while (physical && level--)
      {
        if (!block(physical, scratch)) return false;
        span /= fanout;
        physical = le32(scratch + (index / span) * 4);
        index %= span;
      }
    }
    return !physical || physical < geometry_.blocks || fail(Error::Corrupt);
  }
  const u8 *node = inode.raw + 40;
  unsigned size = 60, expected_depth = le16(node + 6);
  if (expected_depth > 5) return fail(Error::Corrupt);
  u64 lower = 0, upper = 1ULL << 32;
  for (;;)
  {
    unsigned count = le16(node + 2), maximum = le16(node + 4), depth = le16(node + 6);
    if (le16(node) != 0xf30a || depth != expected_depth || !maximum ||
        maximum > (size - 12) / 12 || count > maximum || (depth && !count)) return fail(Error::Corrupt);
    if (node == scratch && metadata_)
    {
      unsigned tail = 12 + maximum * 12;
      if (tail + 4 > size || crc(inode.seed, node, tail) != le32(node + tail)) return fail(Error::Corrupt);
    }
    const u8 *selected = nullptr;
    u64 previous_end = lower, selected_upper = upper;
    for (unsigned i = 0; i < count; ++i)
    {
      const auto *entry = node + 12 + i * 12;
      u64 start = le32(entry);
      if (start < previous_end || start >= upper) return fail(Error::Corrupt);
      if (depth)
      {
        if (i && start == previous_end) return fail(Error::Corrupt);
        previous_end = start;
        if (start <= logical) selected = entry;
        else if (selected && selected_upper == upper) selected_upper = start;
      }
      else
      {
        unsigned encoded = le16(entry + 4), length = encoded > 32768 ? encoded - 32768 : encoded;
        u64 disk = le32(entry + 8) | (u64(le16(entry + 6)) << 32);
        if (!length || start + length > upper || !disk || disk >= geometry_.blocks ||
            length > geometry_.blocks - disk) return fail(Error::Corrupt);
        previous_end = start + length;
        if (logical >= start && logical < previous_end)
          physical = encoded > 32768 ? 0 : disk + logical - start;
      }
    }
    if (!depth || !selected) return true;
    u64 child = le32(selected + 4) | (u64(le16(selected + 8)) << 32);
    lower = le32(selected); upper = selected_upper;
    if (!block(child, scratch)) return false;
    node = scratch; size = geometry_.block_size; --expected_depth;
  }
}
long Volume::read(const Inode &inode, u64 offset, void *destination, unsigned count)
{
  error_ = Error::None;
  if (!ready_) { fail(Error::Invalid); return -1; }
  u64 size = inode.size();
  if (offset >= size || !count) return 0;
  if (count > size - offset) count = size - offset;
  if ((inode.mode() & 0xf000) == Symlink && size < 60 && !(inode.flags() & 0x80000) &&
      !le32(inode.raw + 28) && !le16(inode.raw + 116))
  { copy(destination, inode.raw + 40 + offset, count); return count; }
  if (offset / geometry_.block_size > 0xffffffffU || (offset + count - 1) / geometry_.block_size > 0xffffffffU)
  { fail(Error::Unsupported); return -1; }
  auto *out = static_cast<u8 *>(destination);
  unsigned done = 0;
  while (done < count)
  {
    u64 at = offset + done, physical;
    unsigned within = at % geometry_.block_size, take = geometry_.block_size - within;
    if (take > count - done) take = count - done;
    if (!mapping(inode, at / geometry_.block_size, physical)) return -1;
    if (physical)
    {
      if (!io_.read(physical * geometry_.block_size + within, out + done, take))
      { fail(Error::Io); return -1; }
    }
    else zero(out + done, take);
    done += take;
  }
  return done;
}
bool Volume::directory_block(const Inode &inode, u32 logical, u8 *data)
{
  u64 physical;
  if (!mapping(inode, logical, physical)) return false;
  if (!physical) return fail(Error::Corrupt);
  if (!block(physical, data)) return false;
  if (!metadata_) return true;
  unsigned size = geometry_.block_size;
  auto *tail = data + size - 12;
  if (!le32(tail) && le16(tail + 4) == 12 && !tail[6] && tail[7] == 0xde)
    return crc(inode.seed, data, size - 12) == le32(tail + 8) || fail(Error::Corrupt);
  // The first htree block holds dot/dotdot; internal htree blocks are fake empty dirents.
  unsigned start;
  if ((inode.flags() & 0x1000) && logical == 0 && le16(data + 4) == 12 &&
      le16(data + 16) == size - 12 && !le32(data + 24) && data[29] == 8 && data[30] <= 2)
    start = 32;
  else if ((inode.flags() & 0x1000) && !le32(data) && le16(data + 4) == size && !data[6]) start = 8;
  else return fail(Error::Corrupt);
  unsigned limit = le16(data + start), count = le16(data + start + 2);
  if (!count || count > limit || limit > (size - start - 8) / 8) return fail(Error::Corrupt);
  unsigned checksum_at = start + limit * 8;
  if (le32(data + checksum_at)) return fail(Error::Corrupt);
  u32 value = crc(inode.seed, data, start + count * 8);
  value = crc(value, data + checksum_at, 4);
  u8 zeros[4]{};
  value = crc(value, zeros, 4);
  return value == le32(data + checksum_at + 4) || fail(Error::Corrupt);
}
int Volume::next(const Inode &inode, u64 &cookie, DirectoryEntry &out)
{
  error_ = Error::None;
  if (!ready_ || (inode.mode() & 0xf000) != Directory) { fail(Error::Invalid); return -1; }
  Page memory(io_);
  auto *data = memory.get();
  if (!data) { fail(Error::NoMemory); return -1; }
  u64 cursor = cookie;
  while (cursor < inode.size())
  {
    u64 logical = cursor / geometry_.block_size;
    if (logical > 0xffffffffU || !directory_block(inode, logical, data)) return -1;
    unsigned within = cursor % geometry_.block_size;
    if (inode.size() - cursor < 8 || geometry_.block_size - within < 8)
    { fail(Error::Corrupt); return -1; }
    const auto *entry = data + within;
    if (metadata_ && within == geometry_.block_size - 12 && !le32(entry) &&
        le16(entry + 4) == 12 && !entry[6] && entry[7] == 0xde)
    { cursor += 12; continue; }
    unsigned length = le16(entry + 4), name_size = geometry_.incompatible & 2 ? entry[6] : le16(entry + 6);
    if (length < 8 || (length & 3) || length > geometry_.block_size - within ||
        length > inode.size() - cursor || name_size > 255 || name_size > length - 8)
    { fail(Error::Corrupt); return -1; }
    cursor += length;
    u32 number = le32(entry);
    if (!number) continue;
    if (!name_size || number > geometry_.inodes) { fail(Error::Corrupt); return -1; }
    out = {}; out.inode = number; out.length = name_size;
    out.type = geometry_.incompatible & 2 ? entry[7] : 0;
    for (unsigned i = 0; i < name_size; ++i)
    {
      if (!entry[8 + i] || entry[8 + i] == '/') { fail(Error::Corrupt); return -1; }
      out.name[i] = entry[8 + i];
    }
    cookie = cursor;
    return 1;
  }
  cookie = cursor;
  return 0;
}
bool Volume::lookup(const Inode &directory, const char *name, Inode &out)
{
  u64 cursor = 0;
  DirectoryEntry entry;
  int result;
  while ((result = next(directory, cursor, entry)) > 0)
  {
    unsigned length = 0;
    while (name[length]) ++length;
    if (length == entry.length && equal(name, entry.name, length)) return inode(entry.inode, out);
  }
  return result < 0 ? false : fail(Error::NotFound);
}
}
