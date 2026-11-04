#pragma once
#include "kernel/fs/ext3/volume.h"

namespace userfs::ext4
{
using namespace xv::ext3;
struct Inode
{
  u32 number = 0, seed = 0;
  u8 raw[256]{};
  u16 mode() const { return le16(raw); }
  u32 flags() const { return le32(raw + 32); }
  u64 size() const { return le32(raw + 4) | (u64(le32(raw + 108)) << 32); }
};
struct Geometry
{
  u64 blocks = 0;
  u32 inodes = 0, block_size = 0, blocks_per_group = 0, inodes_per_group = 0;
  u32 first_data = 0, groups = 0, inode_size = 0, descriptor_size = 0;
  u32 compatible = 0, incompatible = 0, readonly = 0;
};
class Volume final
{
public:
  explicit Volume(Io &io) : io_(io) {}
  bool mount();
  bool inode(u32 number, Inode &out);
  long read(const Inode &, u64 offset, void *, unsigned count);
  // A byte-offset cookie. One entry, EOF, or error.
  int next(const Inode &, u64 &cookie, DirectoryEntry &);
  bool lookup(const Inode &, const char *, Inode &);
  const Geometry &geometry() const { return geometry_; }
  Error error() const { return error_; }
  static u32 crc(u32 seed, const void *, unsigned);
private:
  Io &io_;
  Geometry geometry_{};
  u32 checksum_seed_ = 0;
  u8 uuid_[16]{};
  bool ready_ = false, metadata_ = false, gdt_checksum_ = false;
  Error error_ = Error::None;
  bool fail(Error error) { error_ = error; return false; }
  bool block(u64 number, void *data);
  bool group(u32 number, u8 *descriptor);
  bool mapping(const Inode &, u32 logical, u64 &physical);
  bool directory_block(const Inode &, u32 logical, u8 *data);
  static u32 word_crc(u32 seed, u32 value);
};
}
