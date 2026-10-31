module;
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/support/param.h"
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
  publish_mount_snapshot();
}

bool VirtualFileSystem::mount(const char *path, FileSystem &backend, unsigned device)
{
  char name[256];
  MountFlow::Context context{{*this, path, name}, backend, device};
  (void)MountFlow::Graph::run<0>(context);
  if(context.result==0)publish_mount_snapshot();
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
                               bool follow, bool creating,unsigned capacity,bool exact_names)
{
  char pending[MAXPATH], target[MAXPATH];
  WalkFlow::Context context{*this,  path,     parent,  name,  real,
                            follow, creating, pending, target,capacity,exact_names};
  (void)WalkFlow::Graph::run<0>(context);
  return context.result;
}

Inode *VirtualFileSystem::lookup(const char *path, bool real, bool follow)
{
  xv::sync::ScopedLock guard(namespace_);
  char name[256];
  return walk(path, false, name, real, follow);
}

Inode *VirtualFileSystem::lookup_parent(const char *path, char *name,unsigned capacity)
{
  xv::sync::ScopedLock guard(namespace_);
  return walk(path, true, name,false,true,false,capacity);
}

Inode *VirtualFileSystem::create(char *path, short type, short major, short minor,
                                 bool follow)
{
  char name[256];
  CreateFlow::Context context{*this, path, type, major, minor, follow, name};
  (void)CreateFlow::Graph::run<0>(context);
  return context.result;
}

int VirtualFileSystem::link(char *old_path, char *new_path)
{
  Transaction transaction;
  char name[256];
  LinkFlow::Context context{*this, old_path, new_path, name};
  (void)LinkFlow::Graph::run<0>(context);
  return context.result;
}

int VirtualFileSystem::unlink(char *path)
{
  Transaction transaction;
  char name[256];
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
  char name[256];
  MountRamFlow::Context context{{*this, path, name}};
  (void)MountRamFlow::Graph::run<0>(context);
  if(context.result==0)publish_mount_snapshot();
  return context.result;
}

int VirtualFileSystem::unmount(const char *path)
{
  Transaction transaction;
  xv::sync::ScopedLock guard(namespace_);
  char name[256];
  UnmountFlow::Context context{*this, path, name};
  (void)UnmountFlow::Graph::run<0>(context);
  if(context.result==0)publish_mount_snapshot();
  return context.result;
}

void VirtualFileSystem::publish_mount_snapshot()
{
  // Called during boot publication or with namespace_ held. Build metadata
  // before taking the snapshot spinlock; readers never acquire namespace_.
  MountSnapshot records[9]{};
  detail::copy_fixed(records[0].path,"/",128);
  detail::copy_fixed(records[0].type,disk_.type_name(),32);
  records[0].readonly=disk_.read_only();records[0].nodev=!disk_.persistent();
  for(unsigned i=0;i<mount_count_;++i)
  {
    auto &backend=mounts_[i].root->owner();auto &record=records[i+1];
    detail::copy_fixed(record.path,mounts_[i].path,128);detail::copy_fixed(record.type,backend.type_name(),32);
    record.readonly=backend.read_only();record.nodev=!backend.persistent();
  }
  xv::sync::ScopedLock guard(mount_snapshot_lock_);snapshot_count_=mount_count_+1;
  for(unsigned i=0;i<snapshot_count_;++i)mount_snapshots_[i]=records[i];
}

unsigned VirtualFileSystem::snapshot_mounts(MountSnapshot *records,unsigned capacity) const
{
  xv::sync::ScopedLock guard(mount_snapshot_lock_);unsigned count=snapshot_count_<capacity ? snapshot_count_:capacity;
  for(unsigned i=0;i<count;++i)records[i]=mount_snapshots_[i];return count;
}

int VirtualFileSystem::mounts(unsigned index, char *path, char *type,bool canonical,bool *readonly)
{
  xv::sync::ScopedLock guard(namespace_);
  if (!index)
  {
    detail::copy_fixed(path, "/", 128);
    detail::copy_fixed(type, canonical ? disk_.type_name():disk_.name(), 32);
    if(readonly)*readonly=disk_.read_only();
    return 1;
  }
  --index;
  if (index >= mount_count_)
  {
    return 0;
  }
  detail::copy_fixed(path, mounts_[index].path, 128);
  auto &backend=mounts_[index].root->owner();
  detail::copy_fixed(type, canonical ? backend.type_name():backend.name(), 32);
  if(readonly)*readonly=backend.read_only();
  return 1;
}

int VirtualFileSystem::rename(const char *old_path, const char *new_path)
{
  Transaction transaction;
  xv::sync::ScopedLock guard(namespace_);
  char old_name[256]{}, new_name[256]{};
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
  char name[256];
  SymlinkFlow::Context context{*this, target, path, size, name};
  (void)SymlinkFlow::Graph::run<0>(context);
  return context.result;
}

int VirtualFileSystem::mount_disk(const char *path,const char *type)
{
  if (!MountDiskFlow::ready(*this, path))
  {
    return -1;
  }
  if(type && detail::compare(type,storage_->type_name(),16))return -1;
  Transaction transaction;
  xv::sync::ScopedLock guard(namespace_);
  char name[256];
  MountDiskFlow::Context context{*this, path, name};
  (void)MountDiskFlow::Graph::run<0>(context);
  if(context.result==0)publish_mount_snapshot();
  return context.result;
}

} // namespace xv::fs
