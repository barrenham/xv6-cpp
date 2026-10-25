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

// Paths

// Copy the next path element from path into name.
// Return a pointer to the element following the copied one.
// The returned path has no leading slashes,
// so the caller can check *path=='\0' to see if the name is the last one.
// If no name to remove, return 0.
//
// Examples:
//   skip_element("a/bb/c", name) = "bb/c", setting name = "a"
//   skip_element("///a//bb", name) = "bb", setting name = "a"
//   skip_element("a", name) = "", setting name = "a"
//   skip_element("", name) = skip_element("////", name) = 0
//
static const char *skip_element(const char *path, char *name)
{
  const char *s;
  int len;

  while (*path == '/')
  {
    path++;
  }
  if (*path == 0)
  {
    return 0;
  }
  s = path;
  while (*path != '/' && *path != 0)
  {
    path++;
  }
  len = path - s;
  if (len >= DIRSIZ)
  {
    detail::move(name, s, DIRSIZ);
  }
  else
  {
    detail::move(name, s, len);
    name[len] = 0;
  }
  while (*path == '/')
  {
    path++;
  }
  return path;
}

// Look up and return the inode for a path name.
// If parent != 0, return the inode for the parent and copy the final
// path element into name, which must have room for DIRSIZ bytes.
// Must be called inside a transaction since it calls release().
Inode *Xv6FileSystem::walk_path(const char *path, int nameiparent, char *name)
{
  Inode *ip, *next;

  if (*path == '/')
  {
    ip = get(ROOTDEV, ROOTINO);
  }
  else
  {
    ip = detail::current_directory();
  }

  while ((path = skip_element(path, name)) != 0)
  {
    lock(ip);
    if (ip->type != T_DIR)
    {
      unlock_release(ip);
      return 0;
    }
    if (ip->nlink == 0)
    {
      unlock_release(ip);
      return 0;
    }
    if (nameiparent && *path == '\0')
    {
      // Stop one level early.
      unlock(ip);
      return ip;
    }
    if ((next = lookup_entry(ip, name, 0)) == 0)
    {
      unlock_release(ip);
      return 0;
    }
    unlock_release(ip);
    ip = next;
  }
  if (nameiparent)
  {
    release(ip);
    return 0;
  }
  return ip;
}

Inode *Xv6FileSystem::lookup(const char *path)
{
  char name[DIRSIZ];
  return walk_path(path, 0, name);
}

Inode *Xv6FileSystem::lookup_parent(const char *path, char *name)
{
  return walk_path(path, 1, name);
}

int Xv6FileSystem::link(char *old, char *new_path)
{
  char name[DIRSIZ];
  Inode *dp, *ip;

  Transaction transaction(journal_);
  if ((ip = lookup(old)) == 0)
  {
    return -1;
  }

  lock(ip);
  if (ip->type == T_DIR)
  {
    unlock_release(ip);
    return -1;
  }

  if (ip->nlink >= NLINK_MAX)
  {
    unlock_release(ip);
    return -1;
  }

  ip->nlink++;
  update(ip);
  unlock(ip);

  if ((dp = lookup_parent(new_path, name)) == 0)
  {
    goto bad;
  }
  lock(dp);
  // dp may have been unlinked while we resolved it; linking into an
  // orphaned directory leaks ip (itrunc discards the record without
  // dropping ip->nlink).  create() has the same guard.
  if (dp->nlink == 0)
  {
    unlock_release(dp);
    goto bad;
  }
  if (dp->dev != ip->dev || link_entry(dp, name, ip->inum) < 0)
  {
    unlock_release(dp);
    goto bad;
  }
  unlock_release(dp);
  release(ip);

  return 0;

bad:
  lock(ip);
  ip->nlink--;
  update(ip);
  unlock_release(ip);
  return -1;
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

int Xv6FileSystem::unlink(char *path)
{
  Inode *ip, *dp;
  struct dirent de;
  char name[DIRSIZ];
  uint off;

  Transaction transaction(journal_);
  if ((dp = lookup_parent(path, name)) == 0)
  {
    return -1;
  }

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
  unlock_release(dp);

  ip->nlink--;
  update(ip);
  unlock_release(ip);

  return 0;

bad:
  unlock_release(dp);
  return -1;
}

Inode *Xv6FileSystem::create(char *path, short type, short major, short minor)
{
  Inode *ip, *dp;
  char name[DIRSIZ];

  if ((dp = lookup_parent(path, name)) == 0)
  {
    return 0;
  }

  lock(dp);

  if (dp->nlink == 0)
  {
    unlock_release(dp);
    return 0;
  }

  // a new directory's ".." would push dp->nlink past its maximum
  if (type == T_DIR && dp->nlink >= NLINK_MAX)
  {
    unlock_release(dp);
    return 0;
  }

  if ((ip = lookup_entry(dp, name, 0)) != 0)
  {
    unlock_release(dp);
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
    unlock_release(dp);
    return 0;
  }

  lock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
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

  unlock_release(dp);

  return ip;

fail:
  // something went wrong. de-allocate ip.
  ip->nlink = 0;
  update(ip);
  unlock_release(ip);
  unlock_release(dp);
  return 0;
}

} // namespace xv::fs
