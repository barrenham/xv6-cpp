module;
#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/ext3/volume.h"
#include "user/fs/sdk/include/xvfs/ext3.h"
#include "user/runtime/user.h"
module xv.userfs.ext3;
import xv.userfs;

namespace userfs
{
using xv::ext3::Inode;

Ext3Backend::~Ext3Backend()
{
  volume_.reset();
}

bool Ext3Backend::initialize()
{
  ready_ = volume_.mount();
  return ready_;
}

bool Ext3Backend::attributes(const Inode &wire, Attributes &out)
{
  unsigned type;
  switch (wire.mode() & 0xf000)
  {
  case xv::ext3::Regular: type = T_FILE; break;
  case xv::ext3::Directory: type = T_DIR; break;
  case xv::ext3::Symlink: type = T_SYMLINK; break;
  case xv::ext3::Character: type = T_DEVICE; break;
  case xv::ext3::Block: type = T_BLOCK; break;
  default: return false;
  }
  out = {identity(wire.number),
         wire.size() > 0xffffffffU ? 0xffffffffU : unsigned(wire.size()),
         wire.uid(), wire.gid(), unsigned(wire.mode() & 07777), type,
         wire.links(),
         type == T_DEVICE || type == T_BLOCK ? wire.device_major() : 0,
         type == T_DEVICE || type == T_BLOCK ? wire.device_minor() : 0};
  return true;
}

bool Ext3Backend::inode(unsigned node, Inode &out)
{
  return ready_ && !failed_ && volume_.read_inode(number(node), out) && out.mode();
}

Attributes Ext3Backend::root()
{
  Attributes out{};
  Inode wire;
  if (inode(1, wire)) attributes(wire, out);
  return out;
}

bool Ext3Backend::retained(unsigned node) const
{
  if (node == 1) return true;
  for (auto live : live_) if (live == node) return true;
  return false;
}

bool Ext3Backend::borrow(unsigned node)
{
  if (retained(node)) return true;
  for (auto &live : live_)
  {
    if (!live)
    {
      live = node;
      return true;
    }
  }
  return false;
}

int Ext3Backend::lookup(unsigned parent, const char *name, Attributes &out)
{
  Inode directory, found;
  if (!inode(parent, directory) || !volume_.lookup(directory, name, found) ||
      !attributes(found, out) || !borrow(out.node)) return -1;
  return 0;
}

int Ext3Backend::getattr(unsigned node, Attributes &out)
{
  Inode wire;
  return inode(node, wire) && attributes(wire, out) ? 0 : -1;
}

int Ext3Backend::readdir(unsigned node, unsigned long offset, Entry &out)
{
  Inode directory, found;
  if (!inode(node, directory)) return -1;
  xv::ext3::DirectoryEntry entry;
  xv::ext3::u64 cursor = offset;
  if (!volume_.next_entry(directory, cursor, entry))
    return volume_.error() == xv::ext3::Error::None ? 0 : -1;
  if (!volume_.read_inode(entry.inode, found) || !attributes(found, out.attributes))
    return -1;
  out.next = cursor;
  for (unsigned i = 0; i <= entry.length; ++i) out.name[i] = entry.name[i];
  return 1;
}

int Ext3Backend::read(unsigned node, unsigned long offset, void *data, unsigned count)
{
  Inode wire;
  if (!inode(node, wire)) return -1;
  if ((wire.mode() & 0xf000) != xv::ext3::Directory)
    return volume_.read(wire, offset, data, count);
  // Keep the existing read(dir) ABI; Readdir independently exposes long names.
  unsigned copied = 0;
  unsigned long index = 0;
  xv::ext3::u64 cursor = 0;
  xv::ext3::DirectoryEntry entry;
  while (copied < count && volume_.next_entry(wire, cursor, entry))
  {
    dirent record{};
    record.inum = identity(entry.inode);
    for (unsigned i = 0; i < DIRSIZ && i < entry.length; ++i)
      record.name[i] = entry.name[i];
    unsigned long position = index++ * sizeof(record);
    if (position + sizeof(record) <= offset) continue;
    unsigned begin = offset > position ? offset - position : 0;
    unsigned take = sizeof(record) - begin;
    if (take > count - copied) take = count - copied;
    xv::ext3::copy(static_cast<unsigned char *>(data) + copied,
                   reinterpret_cast<unsigned char *>(&record) + begin, take);
    copied += take;
  }
  return volume_.error() == xv::ext3::Error::None ? int(copied) : -1;
}

bool Ext3Backend::commit()
{
  if (failed_) return false;
  if (!volume_.journal().commit())
  {
    // A failed durable commit leaves the server unavailable until recovery.
    failed_ = true;
    return false;
  }
  return true;
}

int Ext3Backend::write(unsigned node, unsigned long offset, const void *data,
                       unsigned count)
{
  Inode wire;
  if (!inode(node, wire)) return -1;
  long result = volume_.write(wire, offset, data, count);
  return result >= 0 && commit() ? result : -1;
}

int Ext3Backend::create(unsigned parent, const char *name, const Attributes &requested,
                        Attributes &out)
{
  Inode directory, found;
  if (!inode(parent, directory)) return -1;
  if (volume_.lookup(directory, name, found))
  {
    if (requested.type != T_FILE ||
        ((found.mode() & 0xf000) != xv::ext3::Regular &&
         (found.mode() & 0xf000) != xv::ext3::Character &&
         (found.mode() & 0xf000) != xv::ext3::Block)) return -1;
    return attributes(found, out) && borrow(out.node) ? 0 : -1;
  }
  if (volume_.error() != xv::ext3::Error::NotFound) return -1;
  bool available = false;
  for (auto live : live_) if (!live) available = true;
  if (!available) return -1;
  unsigned kind = requested.type == T_DIR ? xv::ext3::Directory :
                  requested.type == T_FILE ? xv::ext3::Regular :
                  requested.type == T_SYMLINK ? xv::ext3::Symlink :
                  requested.type == T_DEVICE ? xv::ext3::Character :
                  requested.type == T_BLOCK ? xv::ext3::Block : 0;
  if (!kind || !volume_.create(number(parent), name, kind | requested.mode,
                               requested.uid, requested.gid, found)) return -1;
  if ((requested.type == T_DEVICE || requested.type == T_BLOCK) &&
      !volume_.set_device(found, requested.major, requested.minor))
  {
    unsigned removed;
    if (!volume_.unlink(number(parent), name, removed) || !commit() ||
        !collect(identity(removed))) failed_ = true;
    return -1;
  }
  return attributes(found, out) && borrow(out.node) && commit() ? 0 : -1;
}

int Ext3Backend::resize(unsigned node, unsigned size)
{
  Inode wire;
  return inode(node, wire) && volume_.resize(wire, size) && commit() ? 0 : -1;
}

int Ext3Backend::setattr(unsigned node, const Attributes &requested)
{
  Inode wire;
  return inode(node, wire) && volume_.set_attributes(wire, requested.uid,
           requested.gid, requested.mode) && commit() ? 0 : -1;
}

int Ext3Backend::link(unsigned parent, const char *name, unsigned target)
{
  if (!ready_ || failed_) return -1;
  return volume_.link(number(parent), name, number(target)) && commit() ? 0 : -1;
}

// Unlink commits the namespace and durable orphan list before replying. Reclaim
// blocks between requests, so deleting a large file cannot time out its RPC.
bool Ext3Backend::collect(unsigned node)
{
  if (!node || retained(node)) return true;
  Inode wire;
  if (!inode(node, wire))
  {
    if (volume_.error() != xv::ext3::Error::NoMemory) return false;
    cleanup_pending_ = cleanup_retry_ = true;
    return true;
  }
  if (wire.links()) return true;
  cleanup_pending_ = true;
  return true;
}

bool Ext3Backend::maintenance_failure()
{
  if (volume_.error() == xv::ext3::Error::NoMemory)
  {
    // No mutation has been accepted: Volume rolls back its failed batch.
    // The durable orphan list remains the work queue; retry without retiring FS.
    cleanup_retry_ = true;
    return true;
  }
  failed_ = true;
  return false;
}

bool Ext3Backend::maintain()
{
  if (!ready_ || failed_) return false;
  cleanup_retry_ = false;
  unsigned at;
  if (!volume_.orphan_head(at)) return maintenance_failure();
  for (unsigned visited = 0; at; ++visited)
  {
    Inode wire;
    if (visited >= volume_.geometry().inodes)
    {
      failed_ = true;
      return false;
    }
    if (!volume_.read_inode(at, wire)) return maintenance_failure();
    if (!retained(identity(at)))
    {
      bool complete;
      // The volume bounds each atomic batch to four freed blocks and commits
      // their bitmap, parent paths and inode together. No in-memory work queue
      // is needed; interrupted cleanup resumes from the on-disk orphan list.
      if (wire.links()) { failed_ = true; return false; }
      if (!volume_.reclaim_step(at, complete)) return maintenance_failure();
      // Commit scratch is reserved at mount; no allocation can fail after
      // reclaim_step accepts its mutation. Durable I/O errors remain fatal.
      return commit();
    }
    at = xv::ext3::le32(wire.raw + 20);
  }
  // Remaining orphans are open. Their final Forget schedules another scan.
  cleanup_pending_ = false;
  return true;
}

int Ext3Backend::unlink(unsigned parent, const char *name, Attributes &out)
{
  if (!ready_ || failed_) return -1;
  unsigned removed = 0;
  if (!volume_.unlink(number(parent), name, removed)) return -1;
  Inode wire;
  if (!volume_.read_inode(removed, wire) || !attributes(wire, out) || !commit())
    return -1;
  return collect(out.node) ? 0 : -1;
}

int Ext3Backend::rename(unsigned from, const char *old_name, unsigned to,
                        const char *new_name)
{
  if (!ready_ || failed_) return -1;
  unsigned replaced = 0;
  if (!volume_.rename(number(from), old_name, number(to), new_name, replaced) ||
      !commit()) return -1;
  return !replaced || collect(identity(replaced)) ? 0 : -1;
}

int Ext3Backend::sync(unsigned)
{
  return ready_ && !failed_ && commit() && io_.flush() ? 0 : -1;
}

void Ext3Backend::forget(unsigned node)
{
  if (node == 1) return;
  for (auto &live : live_) if (live == node) live = 0;
  if (!collect(node)) failed_ = true;
}
} // namespace userfs

namespace userfs
{
int Ext3Backend::control(unsigned node, const char *interface_name, unsigned version,
                        unsigned command, unsigned access, const void *, unsigned input_size,
                        void *output, unsigned capacity)
{
  if (strcmp(interface_name, XVFS_EXT3_INTERFACE) || version != XVFS_EXT3_VERSION ||
      input_size || !(access & XVFS_ACCESS_READABLE) || !ready_ || failed_) return -1;
  if (command == XVFS_EXT3_GEOMETRY)
  {
    if (capacity < sizeof(xvfs_ext3_geometry)) return -1;
    unsigned char super[256];
    if (!io_.read(1024, super, sizeof(super))) return -1;
    using xv::ext3::le32;
    xvfs_ext3_geometry info{};
    info.blocks = le32(super + 4); info.free_blocks = le32(super + 12);
    info.inodes = le32(super); info.free_inodes = le32(super + 16);
    info.block_size = volume_.geometry().block_size;
    info.compatible = le32(super + 92); info.incompatible = le32(super + 96);
    info.readonly_compatible = le32(super + 100); info.journal_inode = le32(super + 224);
    memmove(info.uuid, super + 104, sizeof(info.uuid));
    memmove(output, &info, sizeof(info));
    return sizeof(info);
  }
  if (command == XVFS_EXT3_INODE)
  {
    if (capacity < sizeof(xvfs_ext3_inode)) return -1;
    Inode wire;
    if (!inode(node, wire)) return -1;
    using xv::ext3::le32;
    xvfs_ext3_inode info{};
    info.size = wire.size(); info.inode = wire.number; info.flags = wire.flags();
    info.sectors = wire.sectors(); info.access_time = le32(wire.raw + 8);
    info.change_time = le32(wire.raw + 12); info.modification_time = le32(wire.raw + 16);
    info.deletion_time = le32(wire.raw + 20); info.generation = le32(wire.raw + 100);
    info.file_acl_block = le32(wire.raw + 104);
    memmove(output, &info, sizeof(info));
    return sizeof(info);
  }
  return -1;
}
}
