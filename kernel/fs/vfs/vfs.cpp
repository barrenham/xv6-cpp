module;
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
module xv.fs;
import :vfs_initialize_flow;
import :vfs_mount_flow;
import :vfs_path_flow;
import :vfs;
import :platform;

namespace xv::fs
{
void VirtualFileSystem::initialize_inodes()
{
  InitializeFlow::Context context{*this, 0};
  (void)InitializeFlow::InodesGraph::run<0>(context);
}

void VirtualFileSystem::initialize(int device)
{
  InitializeFlow::Context context{*this, device};
  (void)InitializeFlow::BackendsGraph::run<0>(context);
  Transaction transaction;
  (void)InitializeFlow::MountsGraph::run<0>(context);
}

bool VirtualFileSystem::mount(const char *path, FileSystem &backend, unsigned device)
{
  char name[DIRSIZ];
  MountFlow::Context context{{*this, path, name}, backend, device};
  (void)MountFlow::Graph::run<0>(context);
  return context.result == 0;
}

bool VirtualFileSystem::same(Inode *first, Inode *second)
{
  return first && second && &first->owner() == &second->owner() &&
         first->device() == second->device() && first->number() == second->number();
}

Inode *VirtualFileSystem::enter_mount(Inode *node)
{
  for (unsigned i = 0; i < mount_count_; ++i)
  {
    if (same(node, mounts_[i].covered))
    {
      release(node);
      return retain(mounts_[i].root);
    }
  }
  return node;
}

Inode *VirtualFileSystem::walk(const char *path, bool parent, char *name, bool real,
                               bool follow, bool creating)
{
  char pending[128], target[128];
  WalkFlow::Context context{*this,  path,     parent,  name,  real,
                            follow, creating, pending, target};
  (void)WalkFlow::Graph::run<0>(context);
  return context.result;
}

Inode *VirtualFileSystem::lookup(const char *path, bool real, bool follow)
{
  xv::sync::ScopedLock guard(namespace_);
  char name[DIRSIZ];
  return walk(path, false, name, real, follow);
}

Inode *VirtualFileSystem::lookup_parent(const char *path, char *name)
{
  xv::sync::ScopedLock guard(namespace_);
  return walk(path, true, name);
}

Inode *VirtualFileSystem::create(char *path, short type, short major, short minor,
                                 bool follow)
{
  char name[DIRSIZ];
  CreateFlow::Context context{*this, path, type, major, minor, follow, name};
  (void)CreateFlow::Graph::run<0>(context);
  return context.result;
}

int VirtualFileSystem::link(char *old_path, char *new_path)
{
  Transaction transaction;
  char name[DIRSIZ];
  LinkFlow::Context context{*this, old_path, new_path, name};
  (void)LinkFlow::Graph::run<0>(context);
  return context.result;
}

int VirtualFileSystem::unlink(char *path)
{
  Transaction transaction;
  char name[DIRSIZ];
  UnlinkFlow::Context context{*this, path, name};
  (void)UnlinkFlow::Graph::run<0>(context);
  return context.result;
}

Inode *VirtualFileSystem::allocate(unsigned device, short type)
{
  return disk_.allocate(device, type);
}

Inode *VirtualFileSystem::retain(Inode *node)
{
  return node->owner().retain(node);
}

void VirtualFileSystem::release(Inode *node)
{
  node->owner().release(node);
}

void VirtualFileSystem::lock(Inode *node)
{
  node->owner().lock(node);
}

void VirtualFileSystem::unlock(Inode *node)
{
  node->owner().unlock(node);
}

void VirtualFileSystem::unlock_release(Inode *node)
{
  node->owner().unlock_release(node);
}

void VirtualFileSystem::update(Inode *node)
{
  node->owner().update(node);
}

void VirtualFileSystem::truncate(Inode *node)
{
  node->owner().truncate(node);
}

int VirtualFileSystem::read(Inode *node, int user, unsigned long destination,
                            unsigned offset, unsigned count)
{
  return node->owner().read(node, user, destination, offset, count);
}

int VirtualFileSystem::write(Inode *node, int user, unsigned long source,
                             unsigned offset, unsigned count)
{
  return node->owner().write(node, user, source, offset, count);
}

bool VirtualFileSystem::allowed(Inode *node, unsigned requested, bool real)
{
  return detail::allowed(node, requested, real);
}

void VirtualFileSystem::stat(Inode *node, Stat *result)
{
  node->owner().stat(node, result);
}

Inode *VirtualFileSystem::lookup_entry(Inode *directory, const char *name,
                                       unsigned *offset)
{
  return directory->owner().lookup_entry(directory, name, offset);
}

Inode *VirtualFileSystem::create_at(Inode *directory, const char *name, short type,
                                    short major, short minor)
{
  xv::sync::ScopedLock guard(namespace_);
  return directory->owner().create_at(directory, name, type, major, minor);
}

int VirtualFileSystem::link_at(Inode *directory, const char *name, Inode *target)
{
  xv::sync::ScopedLock guard(namespace_);
  if (&directory->owner() != &target->owner())
  {
    return -1;
  }
  return directory->owner().link_at(directory, name, target);
}

int VirtualFileSystem::unlink_at(Inode *directory, const char *name)
{
  xv::sync::ScopedLock guard(namespace_);
  UnlinkAtFlow::Context context{*this, directory, name};
  (void)UnlinkAtFlow::Graph::run<0>(context);
  return context.result;
}

bool VirtualFileSystem::ancestor(Inode *possible, Inode *node)
{
  AncestorFlow::Context context{*this, possible, node};
  (void)AncestorFlow::Graph::run<0>(context);
  return context.result;
}

int VirtualFileSystem::mount_ram(const char *path)
{
  Transaction transaction;
  xv::sync::ScopedLock guard(namespace_);
  char name[DIRSIZ];
  MountRamFlow::Context context{{*this, path, name}};
  (void)MountRamFlow::Graph::run<0>(context);
  return context.result;
}

int VirtualFileSystem::unmount(const char *path)
{
  Transaction transaction;
  xv::sync::ScopedLock guard(namespace_);
  char name[DIRSIZ];
  UnmountFlow::Context context{*this, path, name};
  (void)UnmountFlow::Graph::run<0>(context);
  return context.result;
}

int VirtualFileSystem::mounts(unsigned index, char *path, char *type)
{
  xv::sync::ScopedLock guard(namespace_);
  if (!index)
  {
    detail::copy_fixed(path, "/", 128);
    detail::copy_fixed(type, disk_.name(), 32);
    return 1;
  }
  --index;
  if (index >= mount_count_)
  {
    return 0;
  }
  detail::copy_fixed(path, mounts_[index].path, 128);
  detail::copy_fixed(type, mounts_[index].root->owner().name(), 32);
  return 1;
}

int VirtualFileSystem::rename(const char *old_path, const char *new_path)
{
  Transaction transaction;
  xv::sync::ScopedLock guard(namespace_);
  char old_name[DIRSIZ]{}, new_name[DIRSIZ]{};
  RenameFlow::Context context{*this, old_path, new_path, old_name, new_name};
  (void)RenameFlow::Graph::run<0>(context);
  return context.result;
}

int VirtualFileSystem::symlink(const char *target, const char *path)
{
  const unsigned size = SymlinkFlow::measure(target);
  if (!size)
  {
    return -1;
  }
  Transaction transaction;
  xv::sync::ScopedLock guard(namespace_);
  char name[DIRSIZ];
  SymlinkFlow::Context context{*this, target, path, size, name};
  (void)SymlinkFlow::Graph::run<0>(context);
  return context.result;
}

int VirtualFileSystem::mount_disk(const char *path)
{
  if (!MountDiskFlow::ready(*this, path))
  {
    return -1;
  }
  Transaction transaction;
  xv::sync::ScopedLock guard(namespace_);
  char name[DIRSIZ];
  MountDiskFlow::Context context{*this, path, name};
  (void)MountDiskFlow::Graph::run<0>(context);
  return context.result;
}

} // namespace xv::fs
