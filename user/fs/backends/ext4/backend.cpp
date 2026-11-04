module;
#include "volume.h"
#include "user/runtime/user.h"
#include "kernel/fs/stat.h"
#include "user/fs/backends/xv6/format.h"
#include "user/fs/sdk/include/xvfs/ext4.h"
module xv.userfs.ext4;

namespace userfs
{
bool Ext4Backend::attributes(const ext4::Inode &inode, Attributes &out)
{
  unsigned type;
  switch (inode.mode() & 0xf000)
  {
  case xv::ext3::Directory: type = T_DIR; break;
  case xv::ext3::Regular: type = T_FILE; break;
  case xv::ext3::Symlink: type = T_SYMLINK; break;
  default: return false;
  }
  if (inode.size() > 0xffffffffU || xv::ext3::le16(inode.raw + 26) > 32767) return false;
  out = {wire_node(inode.number), unsigned(inode.size()),
         unsigned(xv::ext3::le16(inode.raw + 2)) | (unsigned(xv::ext3::le16(inode.raw + 120)) << 16),
         unsigned(xv::ext3::le16(inode.raw + 24)) | (unsigned(xv::ext3::le16(inode.raw + 122)) << 16),
         unsigned(inode.mode() & 07777), type, unsigned(xv::ext3::le16(inode.raw + 26)), 0, 0};
  return true;
}
Attributes Ext4Backend::root()
{
  Attributes out{};
  getattr(1, out);
  return out;
}
int Ext4Backend::getattr(unsigned node, Attributes &out)
{
  ext4::Inode inode;
  return ready_ && volume_.inode(disk_node(node), inode) && attributes(inode, out) ? 0 : -1;
}
int Ext4Backend::lookup(unsigned parent, const char *name, Attributes &out)
{
  ext4::Inode directory, found;
  return ready_ && volume_.inode(disk_node(parent), directory) &&
         volume_.lookup(directory, name, found) && attributes(found, out) ? 0 : -1;
}
int Ext4Backend::readdir(unsigned node, unsigned long offset, Entry &out)
{
  ext4::Inode directory, found;
  if (!ready_ || !volume_.inode(disk_node(node), directory)) return -1;
  xv::ext3::u64 cookie = offset;
  xv::ext3::DirectoryEntry entry;
  int result = volume_.next(directory, cookie, entry);
  if (result <= 0) return result;
  if (!volume_.inode(entry.inode, found) || !attributes(found, out.attributes)) return -1;
  out.next = cookie;
  memmove(out.name, entry.name, entry.length + 1);
  return 1;
}
int Ext4Backend::read(unsigned node, unsigned long offset, void *data, unsigned count)
{
  ext4::Inode inode;
  if (!ready_ || !volume_.inode(disk_node(node), inode)) return -1;
  if ((inode.mode() & 0xf000) != xv::ext3::Directory)
    return volume_.read(inode, offset, data, count);
  unsigned copied = 0;
  unsigned long index = 0;
  xv::ext3::u64 cookie = 0;
  xv::ext3::DirectoryEntry entry;
  int result = 0;
  while (copied < count && (result = volume_.next(inode, cookie, entry)) > 0)
  {
    dirent record{};
    if (wire_node(entry.inode) > 65535) return -1; // Use getdents64 for large inode numbers.
    record.inum = wire_node(entry.inode);
    memmove(record.name, entry.name, entry.length < DIRSIZ ? entry.length : DIRSIZ);
    unsigned long at = index++ * sizeof(record);
    if (offset >= at + sizeof(record)) continue;
    unsigned start = offset > at ? offset - at : 0, take = sizeof(record) - start;
    if (take > count - copied) take = count - copied;
    memmove(static_cast<unsigned char *>(data) + copied,
            reinterpret_cast<unsigned char *>(&record) + start, take);
    copied += take;
  }
  return result < 0 ? -1 : int(copied);
}
int Ext4Backend::control(unsigned, const char *name, unsigned version, unsigned command,
                        unsigned access, const void *, unsigned input, void *output, unsigned capacity)
{
  if (!ready_ || strcmp(name, XVFS_EXT4_INTERFACE) || version != XVFS_EXT4_VERSION ||
      command != XVFS_EXT4_GEOMETRY || input || !(access & XVFS_ACCESS_READABLE) ||
      capacity < sizeof(xvfs_ext4_geometry)) return -1;
  const auto &geometry = volume_.geometry();
  xvfs_ext4_geometry result{geometry.blocks, geometry.block_size, geometry.inodes,
      geometry.compatible, geometry.incompatible, geometry.readonly, 1};
  memmove(output, &result, sizeof(result));
  return sizeof(result);
}
}
