module;
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
module xv.fs;
import :vfs;
import :platform;
namespace xv::fs
{
void VirtualFileSystem::initialize_inodes()
{
  disk_.initialize_inodes();
  devices_.initialize_inodes();
  if (processes_) processes_->initialize_inodes();
  if (temporary_) temporary_->initialize_inodes();
}
void VirtualFileSystem::initialize(int device)
{
  disk_.initialize(device);
  Transaction transaction;
  if (!mount("/dev", devices_, device + 1)) detail::fail("VFS: cannot mount /dev");
  if (processes_ && !mount("/proc", *processes_, device + 2)) detail::fail("VFS: cannot mount /proc");
  if (temporary_ && !mount("/tmp", *temporary_, device + 3)) detail::fail("VFS: cannot mount /tmp");
  frozen_ = true;
}
bool VirtualFileSystem::mount(const char *path, FileSystem &backend, unsigned device)
{
  if (frozen_ || mount_count_ == 3) return false;
  auto *covered = lookup(path);
  if (!covered)
  {
    covered = create(const_cast<char *>(path), T_DIR, 0, 0);
    if (covered) unlock(covered);
  }
  if (!covered) return false;
  lock(covered);
  bool directory = covered->kind() == T_DIR;
  unlock(covered);
  if (!directory) { release(covered); return false; }
  for (unsigned i = 0; i < mount_count_; ++i)
    if (same(covered, mounts_[i].root)) { release(covered); return false; }
  char name[DIRSIZ];
  auto *parent = lookup_parent(path, name);
  if (!parent) { release(covered); return false; }
  backend.initialize(device);
  mounts_[mount_count_++] = {covered, backend.root(), parent};
  return true;
}

bool VirtualFileSystem::same(Inode *first, Inode *second)
{
  return first && second && &first->owner() == &second->owner() &&
         first->device() == second->device() && first->number() == second->number();
}
Inode *VirtualFileSystem::enter_mount(Inode *node)
{
  for (unsigned i = 0; i < mount_count_; ++i)
    if (same(node, mounts_[i].covered))
    {
      release(node);
      return retain(mounts_[i].root);
    }
  return node;
}
Inode *VirtualFileSystem::walk(const char *path, bool parent, char *name, bool real)
{
  Inode *node = *path == '/' ? root() : detail::current_directory();
  if (!node) return nullptr;
  for (;;)
  {
    while (*path == '/') ++path;
    if (!*path) break;
    unsigned length = 0;
    while (*path && *path != '/')
    {
      if (length < DIRSIZ) name[length++] = *path;
      ++path;
    }
    if (length < DIRSIZ) name[length] = 0;
    while (*path == '/') ++path;
    lock(node);
    if (node->kind() != T_DIR || !node->linked() || !detail::allowed(node, 1, real))
    {
      unlock_release(node);
      return nullptr;
    }
    if (parent && !*path)
    {
      unlock(node);
      return node;
    }
    Inode *next;
    next = nullptr;
    if (detail::compare(name, "..", DIRSIZ) == 0)
      for (unsigned i = 0; i < mount_count_; ++i)
        if (same(node, mounts_[i].root)) { next = retain(mounts_[i].parent); break; }
    if (!next) next = node->owner().lookup_entry(node, name, nullptr);
    unlock_release(node);
    if (!next) return nullptr;
    node = enter_mount(next);
  }
  if (!parent) return node;
  release(node);
  return nullptr;
}
Inode *VirtualFileSystem::lookup(const char *path, bool real)
{
  char name[DIRSIZ];
  return walk(path, false, name, real);
}
Inode *VirtualFileSystem::lookup_parent(const char *path, char *name) { return walk(path, true, name); }
Inode *VirtualFileSystem::create(char *path, short type, short major, short minor)
{
  char name[DIRSIZ];
  auto *parent = lookup_parent(path, name);
  if (!parent) return nullptr;
  auto *node = create_at(parent, name, type, major, minor);
  release(parent);
  return node;
}
int VirtualFileSystem::link(char *old_path, char *new_path)
{
  Transaction transaction;
  char name[DIRSIZ];
  auto *target = lookup(old_path);
  if (!target) return -1;
  auto *parent = lookup_parent(new_path, name);
  int result = parent ? link_at(parent, name, target) : -1;
  if (parent) release(parent);
  release(target);
  return result;
}
int VirtualFileSystem::unlink(char *path)
{
  Transaction transaction;
  char name[DIRSIZ];
  auto *parent = lookup_parent(path, name);
  if (!parent) return -1;
  int result = unlink_at(parent, name);
  release(parent);
  return result;
}
Inode *VirtualFileSystem::allocate(unsigned device, short type) { return disk_.allocate(device, type); }
Inode *VirtualFileSystem::retain(Inode *node) { return node->owner().retain(node); }
void VirtualFileSystem::release(Inode *node) { node->owner().release(node); }
void VirtualFileSystem::lock(Inode *node) { node->owner().lock(node); }
void VirtualFileSystem::unlock(Inode *node) { node->owner().unlock(node); }
void VirtualFileSystem::unlock_release(Inode *node) { node->owner().unlock_release(node); }
void VirtualFileSystem::update(Inode *node) { node->owner().update(node); }
void VirtualFileSystem::truncate(Inode *node) { node->owner().truncate(node); }
int VirtualFileSystem::read(Inode *node, int user, unsigned long destination, unsigned offset, unsigned count)
{ return node->owner().read(node, user, destination, offset, count); }
int VirtualFileSystem::write(Inode *node, int user, unsigned long source, unsigned offset, unsigned count)
{ return node->owner().write(node, user, source, offset, count); }
bool VirtualFileSystem::allowed(Inode *node, unsigned requested, bool real)
{ return detail::allowed(node, requested, real); }
void VirtualFileSystem::stat(Inode *node, Stat *result) { node->owner().stat(node, result); }
Inode *VirtualFileSystem::lookup_entry(Inode *directory, const char *name, unsigned *offset)
{ return directory->owner().lookup_entry(directory, name, offset); }
Inode *VirtualFileSystem::create_at(Inode *directory, const char *name, short type, short major, short minor)
{ return directory->owner().create_at(directory, name, type, major, minor); }
int VirtualFileSystem::link_at(Inode *directory, const char *name, Inode *target)
{
  if (&directory->owner() != &target->owner()) return -1;
  return directory->owner().link_at(directory, name, target);
}
int VirtualFileSystem::unlink_at(Inode *directory, const char *name)
{
  lock(directory);
  if (directory->kind() != T_DIR) { unlock(directory); return -1; }
  auto *target = lookup_entry(directory, name, nullptr);
  bool mounted = false;
  for (unsigned i = 0; i < mount_count_; ++i)
    mounted |= same(target, mounts_[i].covered) || same(target, mounts_[i].root);
  if (target) release(target);
  unlock(directory);
  if (mounted) return -1;
  return directory->owner().unlink_at(directory, name);
}
}
