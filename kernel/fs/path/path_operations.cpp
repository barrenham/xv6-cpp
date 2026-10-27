module;
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"

module xv.fs;
import :platform;
import :xv6;

namespace xv::fs
{
// Directories

int Xv6FileSystem::compare_names(const char *s, const char *t)
{
  return detail::compare(s, t, DIRSIZ);
}

// Look for a directory entry in a directory.
// If found, set *poff to byte offset of entry.
Inode *Xv6FileSystem::lookup_entry(Inode *dp, const char *name, uint *poff)
{
  uint off, inum;
  struct dirent de;

  if (dp->type != T_DIR)
  {
    detail::fail("dirlookup not DIR");
  }

  for (off = 0; off < dp->size; off += sizeof(de))
  {
    if (read(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
    {
      detail::fail("dirlookup read");
    }
    if (de.inum == 0)
    {
      continue;
    }
    if (compare_names(name, de.name) == 0)
    {
      // entry matches path element
      if (poff)
      {
        *poff = off;
      }
      inum = de.inum;
      return get(dp->dev, inum);
    }
  }

  return 0;
}

// Write a new directory entry (name, inum) into the directory dp.
// Returns 0 on success, -1 on failure (e.g. out of disk blocks).
int Xv6FileSystem::link_entry(Inode *dp, const char *name, uint inum)
{
  int off;
  struct dirent de;
  Inode *ip;

  // Check that name is not present.
  if ((ip = lookup_entry(dp, name, 0)) != 0)
  {
    release(ip);
    return -1;
  }

  // Look for an empty dirent.
  for (off = 0; off < dp->size; off += sizeof(de))
  {
    if (read(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
    {
      detail::fail("dirlink read");
    }
    if (de.inum == 0)
    {
      break;
    }
  }

  detail::copy_fixed(de.name, name, DIRSIZ);
  de.inum = inum;
  if (write(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
  {
    return -1;
  }

  return 0;
}

int Xv6FileSystem::link_at(Inode *dp, const char *name, Inode *ip)
{
  if (&ip->owner() != this || &dp->owner() != this) return -1;
  lock(ip);
  if (ip->type == T_DIR || ip->nlink == 0 || ip->nlink >= NLINK_MAX)
  {
    unlock(ip);
    return -1;
  }
  ++ip->nlink;
  update(ip);
  unlock(ip);
  lock(dp);
  int result = dp->nlink && detail::allowed(dp, 3) && dp->dev == ip->dev ? link_entry(dp, name, ip->inum) : -1;
  unlock(dp);
  if (result < 0)
  {
    lock(ip);
    --ip->nlink;
    update(ip);
    unlock(ip);
  }
  return result;
}

// Is the directory dp empty except for "." and ".." ?
int Xv6FileSystem::directory_empty(Inode *dp)
{
  int off;
  struct dirent de;

  for (off = 2 * sizeof(de); off < dp->size; off += sizeof(de))
  {
    if (read(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
    {
      detail::fail("isdirempty: readi");
    }
    if (de.inum != 0)
    {
      return 0;
    }
  }
  return 1;
}

int Xv6FileSystem::unlink_at(Inode *dp, const char *name)
{
  Inode *ip;
  struct dirent de;
  uint off;
  lock(dp);

  // Cannot unlink "." or "..".
  if (compare_names(name, ".") == 0 || compare_names(name, "..") == 0)
  {
    goto bad;
  }

  if ((ip = lookup_entry(dp, name, &off)) == 0)
  {
    goto bad;
  }
  lock(ip);

  if (!detail::may_remove(dp, ip))
  { unlock_release(ip); goto bad; }

  if (ip->nlink < 1)
  {
    detail::fail("unlink: nlink < 1");
  }
  if (ip->type == T_DIR && !directory_empty(ip))
  {
    unlock_release(ip);
    goto bad;
  }

  detail::fill(&de, 0, sizeof(de));
  if (write(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
  {
    detail::fail("unlink: writei");
  }
  if (ip->type == T_DIR)
  {
    dp->nlink--;
    update(dp);
  }
  unlock(dp);

  ip->nlink--;
  update(ip);
  unlock_release(ip);

  return 0;

bad:
  unlock(dp);
  return -1;
}

Inode *Xv6FileSystem::create_at(Inode *dp, const char *name, short type, short major, short minor)
{
  Inode *ip;
  lock(dp);

  if (dp->nlink == 0 || !detail::allowed(dp, 3) || (type == T_DEVICE && detail::identity().uid != 0))
  {
    unlock(dp);
    return 0;
  }

  // a new directory's ".." would push dp->nlink past its maximum
  if (type == T_DIR && dp->nlink >= NLINK_MAX)
  {
    unlock(dp);
    return 0;
  }

  if ((ip = lookup_entry(dp, name, 0)) != 0)
  {
    unlock(dp);
    lock(ip);
    if (type == T_FILE && (ip->type == T_FILE || ip->type == T_DEVICE))
    {
      return ip;
    }
    unlock_release(ip);
    return 0;
  }

  if ((ip = allocate(dp->dev, type)) == 0)
  {
    unlock(dp);
    return 0;
  }

  lock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  { auto who = detail::identity(); ip->uid_ = who.uid; ip->gid_ = who.gid;
    ip->mode_ = (type == T_DIR ? 0777 : type == T_DEVICE ? 0600 : 0666) & ~who.mask; }
  update(ip);

  if (type == T_DIR)
  { // Create . and .. entries.
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if (link_entry(ip, ".", ip->inum) < 0 || link_entry(ip, "..", dp->inum) < 0)
    {
      goto fail;
    }
  }

  if (link_entry(dp, name, ip->inum) < 0)
  {
    goto fail;
  }

  if (type == T_DIR)
  {
    // now that success is guaranteed:
    dp->nlink++; // for ".."
    update(dp);
  }

  unlock(dp);

  return ip;

fail:
  // something went wrong. de-allocate ip.
  ip->nlink = 0;
  update(ip);
  unlock_release(ip);
  unlock(dp);
  return 0;
}

int Xv6FileSystem::rename_at(Inode *from, const char *old_name, Inode *to, const char *new_name)
{
  Inode *first = from->inum <= to->inum ? from : to;
  Inode *second = first == from ? to : from;
  lock(first); if (second != first) lock(second);
  unsigned old_offset = 0, new_offset = 0;
  auto *source = lookup_entry(from, old_name, &old_offset);
  auto *target = lookup_entry(to, new_name, &new_offset);
  if (source == from || source == to || target == from || target == to)
  {
    if (second != first) unlock(second);
    unlock(first);
    if (source) release(source);
    if (target) release(target);
    return -1;
  }
  int result = -1;
  if (source) lock(source);
  if (target && target != source) lock(target);
  bool valid = source && from->nlink && to->nlink && detail::allowed(from, 3) && detail::allowed(to, 3) &&
               detail::may_remove(from, source);
  if (target && source && target != source)
    valid &= detail::may_remove(to, target) && ((source->type == T_DIR) == (target->type == T_DIR)) &&
             (target->type != T_DIR || directory_empty(target));
  if (valid && source->type == T_DIR && from != to && !target && to->nlink == NLINK_MAX) valid = false;
  if (valid && target == source) result = 0;
  else if (valid)
  {
    dirent record{}; record.inum = source->inum; detail::copy_fixed(record.name, new_name, DIRSIZ);
    bool installed;
    if (target) installed = write(to, 0, reinterpret_cast<uint64>(&record), new_offset, sizeof(record)) == sizeof(record);
    else if (from == to) installed = write(from, 0, reinterpret_cast<uint64>(&record), old_offset, sizeof(record)) == sizeof(record);
    else installed = link_entry(to, new_name, source->inum) == 0;
    if (installed)
    {
      if (target || from != to)
      {
        dirent blank{};
        if (write(from, 0, reinterpret_cast<uint64>(&blank), old_offset, sizeof(blank)) != sizeof(blank)) detail::fail("rename: source write");
      }
      if (target)
      {
        --target->nlink; update(target);
        if (target->type == T_DIR) --to->nlink;
      }
      if (source->type == T_DIR && from != to)
      {
        dirent parent{}; parent.inum = to->inum; detail::copy_fixed(parent.name, "..", DIRSIZ);
        if (write(source, 0, reinterpret_cast<uint64>(&parent), sizeof(dirent), sizeof(parent)) != sizeof(parent)) detail::fail("rename: parent write");
        --from->nlink; ++to->nlink;
      }
      update(from); if (from != to) update(to);
      result = 0;
    }
  }
  if (target && target != source) unlock(target);
  if (source) unlock(source);
  if (second != first) unlock(second); unlock(first);
  if (source) release(source);
  if (target) release(target);
  return result;
}

} // namespace xv::fs
