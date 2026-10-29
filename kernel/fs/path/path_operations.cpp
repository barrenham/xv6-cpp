module;
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"

module xv.fs;
import :xv6_path_flow;
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
  LinkFlow::Context context{*this, dp, name, ip};
  (void)LinkFlow::Graph::run<0>(context);
  return context.result;
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
  lock(dp);
  UnlinkFlow::Context context{*this, dp, name};
  (void)UnlinkFlow::Graph::run<0>(context);
  return context.result;
}

Inode *Xv6FileSystem::create_at(Inode *dp, const char *name, short type, short major,
                                short minor)
{
  lock(dp);
  CreateFlow::Context context{*this, dp, name, type, major, minor};
  (void)CreateFlow::Graph::run<0>(context);
  return context.result;
}

int Xv6FileSystem::rename_at(Inode *from, const char *old_name, Inode *to,
                             const char *new_name)
{
  Inode *first = from->inum <= to->inum ? from : to;
  Inode *second = first == from ? to : from;
  dirent record;
  RenameFlow::Context context{*this,    from,  old_name, to,
                              new_name, first, second,   record};
  (void)RenameFlow::Graph::run<0>(context);
  return context.result;
}

} // namespace xv::fs
