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
  namespace_.init("vfs namespace");
  if (dynamic_) for (unsigned i = 0; i < 4; ++i) dynamic_[i]->initialize_inodes();
  disk_.initialize_inodes();
  devices_.initialize_inodes();
  if (processes_) processes_->initialize_inodes();
  if (temporary_) temporary_->initialize_inodes();
  if (storage_) storage_->initialize_inodes();
}
void VirtualFileSystem::initialize(int device)
{
  disk_.initialize(device);
  if (storage_) { storage_->initialize(5); Transaction::secondary = storage_->transaction_log(); storage_ready_ = true; }
  { Transaction transaction;
  if (!mount("/dev", devices_, device + 1)) detail::fail("VFS: cannot mount /dev");
  if (processes_ && !mount("/proc", *processes_, device + 2)) detail::fail("VFS: cannot mount /proc");
  if (temporary_ && !mount("/tmp", *temporary_, device + 3)) detail::fail("VFS: cannot mount /tmp");
  }

}
bool VirtualFileSystem::mount(const char *path, FileSystem &backend, unsigned device)
{
  if (mount_count_ == 8) return false;
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
  auto &record = mounts_[mount_count_++];
  record.covered = covered; record.root = backend.root(); record.parent = parent;
  detail::copy_fixed(record.path, path, sizeof(record.path));
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
Inode *VirtualFileSystem::walk(const char *path, bool parent, char *name, bool real, bool follow, bool creating)
{
  char pending[128]; unsigned length = 0;
  while (path[length]) { if (length == sizeof(pending) - 1) return nullptr; pending[length] = path[length]; ++length; }
  pending[length] = 0;
  bool trailing = length && pending[length - 1] == '/';
  path = pending;
  Inode *node = *path == '/' ? root() : detail::current_directory();
  if (!node) return nullptr;
  unsigned links = 0;
  for (;;)
  {
    while (*path == '/') ++path;
    if (!*path) break;
    unsigned component = 0;
    while (*path && *path != '/')
    {
      if (component < DIRSIZ) name[component++] = *path;
      ++path;
    }
    if (component < DIRSIZ) name[component] = 0;
    while (*path == '/') ++path;
    lock(node);
    if (node->kind() != T_DIR || !node->linked() || !detail::allowed(node, 1, real))
    { unlock_release(node); return nullptr; }
    if (parent && !*path) { unlock(node); return node; }
    Inode *next = nullptr;
    if (detail::compare(name, "..", DIRSIZ) == 0)
      for (unsigned i = 0; i < mount_count_; ++i)
        if (same(node, mounts_[i].root)) { next = retain(mounts_[i].parent); break; }
    if (!next) next = node->owner().lookup_entry(node, name, nullptr);
    unlock(node);
    if (!next && creating && !*path)
    {
      auto *created = node->owner().create_at(node, name, T_FILE, 0, 0);
      if (created) unlock(created);
      release(node); return created;
    }
    if (!next) { release(node); return nullptr; }
    lock(next);
    bool symbolic = next->kind() == T_SYMLINK && (follow || *path || trailing);
    if (symbolic)
    {
      char target[128]; unsigned size = next->size_bytes(), rest = 0;
      while (path[rest]) ++rest;
      bool valid = ++links <= 16 && size && size + (rest ? 1 : 0) + rest < sizeof(pending) &&
                   next->owner().read(next, 0, reinterpret_cast<unsigned long>(target), 0, size) == static_cast<int>(size);
      unlock_release(next);
      if (!valid) { release(node); return nullptr; }
      if (!rest && target[size - 1] == '/') trailing = true;
      // The remaining path overlaps pending, so move it before copying the target.
      if (rest) { detail::move(pending + size + 1, path, rest + 1); pending[size] = '/'; }
      else pending[size] = 0;
      detail::move(pending, target, size);
      if (target[0] == '/') { release(node); node = root(); }
      path = pending;
      continue;
    }
    unlock(next); release(node); node = enter_mount(next);
  }
  if (parent) { release(node); return nullptr; }
  if (trailing) { lock(node); bool directory = node->kind() == T_DIR; unlock(node); if (!directory) { release(node); return nullptr; } }
  return node;
}
Inode *VirtualFileSystem::lookup(const char *path, bool real, bool follow)
{
  xv::sync::ScopedLock guard(namespace_);
  char name[DIRSIZ];
  return walk(path, false, name, real, follow);
}
Inode *VirtualFileSystem::lookup_parent(const char *path, char *name)
{ xv::sync::ScopedLock guard(namespace_); return walk(path, true, name); }
Inode *VirtualFileSystem::create(char *path, short type, short major, short minor, bool follow)
{
  if (type == T_FILE)
  {
    xv::sync::ScopedLock guard(namespace_); char name[DIRSIZ];
    auto *existing = walk(path, false, name, false, follow, true);
    if (existing) { lock(existing); if (existing->kind() == T_FILE || existing->kind() == T_DEVICE) return existing; unlock_release(existing); }
    return nullptr;
  }
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
  auto *target = lookup(old_path, false, false);
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
{ xv::sync::ScopedLock guard(namespace_); return directory->owner().create_at(directory, name, type, major, minor); }
int VirtualFileSystem::link_at(Inode *directory, const char *name, Inode *target)
{
  xv::sync::ScopedLock guard(namespace_);
  if (&directory->owner() != &target->owner()) return -1;
  return directory->owner().link_at(directory, name, target);
}
int VirtualFileSystem::unlink_at(Inode *directory, const char *name)
{
  xv::sync::ScopedLock guard(namespace_);
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
bool VirtualFileSystem::ancestor(Inode *possible, Inode *node)
{
  node = retain(node);
  for (unsigned depth = 0; depth < 128; ++depth)
  {
    if (same(possible, node)) { release(node); return true; }
    Inode *parent = nullptr;
    for (unsigned i = 0; i < mount_count_; ++i)
      if (same(node, mounts_[i].root)) { parent = retain(mounts_[i].parent); break; }
    if (!parent) { lock(node); parent = lookup_entry(node, "..", nullptr); unlock(node); }
    bool end = !parent || same(node, parent);
    release(node);
    if (end) { if (parent) release(parent); return false; }
    node = parent;
  }
  release(node); return true;
}
int VirtualFileSystem::mount_ram(const char *path)
{
  Transaction transaction;
  xv::sync::ScopedLock guard(namespace_);
  // Canonical absolute names keep the listing stable and unambiguous.
  if (!dynamic_ || !path || path[0] != '/' || !path[1] || mount_count_ == 8) return -1;
  unsigned length = 0, component = 0;
  for (; path[length]; ++length)
  {
    if (length >= 127) return -1;
    if (path[length] == '/')
    { if (length && !component) return -1; component = 0; }
    else { if (++component > DIRSIZ) return -1; }
  }
  if (!component) return -1;
  for (unsigned i = 1; i < length; ++i)
    if (path[i - 1] == '/' && path[i] == '.' && (!path[i + 1] || path[i + 1] == '/' ||
        (path[i + 1] == '.' && (!path[i + 2] || path[i + 2] == '/')))) return -1;
  unsigned slot = 0; while (slot < 4 && used_[slot]) ++slot;
  if (slot == 4) return -1;
  char name[DIRSIZ];
  auto *covered = walk(path, false, name);
  if (!covered) return -1;
  lock(covered); bool valid = covered->kind() == T_DIR; unlock(covered);
  for (unsigned i = 0; i < mount_count_; ++i) valid &= !same(covered, mounts_[i].root);
  auto *parent = valid ? walk(path, true, name) : nullptr;
  if (!parent) { release(covered); return -1; }
  auto &backend = *dynamic_[slot];
  if (!backend.initialize_mount(next_device_++)) { release(covered); release(parent); return -1; }
  auto *root = backend.root();
  backend.lock(root); backend.attributes(root, 0, 0, 0755); backend.unlock(root);
  auto &record = mounts_[mount_count_++];
  record.covered = covered; record.parent = parent; record.root = root; record.dynamic = slot;
  detail::copy_fixed(record.path, path, sizeof(record.path)); used_[slot] = true;
  return 0;
}
int VirtualFileSystem::unmount(const char *path)
{
  Transaction transaction;
  xv::sync::ScopedLock guard(namespace_);
  char name[DIRSIZ]; auto *node = walk(path, false, name);
  if (!node) return -1;
  int index = -1;
  for (unsigned i = 0; i < mount_count_; ++i) if (same(node, mounts_[i].root)) index = i;
  release(node);
  if (index < 0 || mounts_[index].dynamic == -1) return -1;
  auto record = mounts_[index]; auto &backend = record.dynamic == -2 ? *storage_ : *dynamic_[record.dynamic];
  // Child mount pins also count as live references to this backend.
  if (backend.busy()) return -1;
  mounts_[index] = mounts_[--mount_count_]; mounts_[mount_count_] = {};
  release(record.covered); release(record.parent);
  if (record.dynamic == -2) { backend.flush(); release(record.root); }
  else { backend.shutdown(); used_[record.dynamic] = false; }
  return 0;
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
  if (index >= mount_count_) return 0;
  detail::copy_fixed(path, mounts_[index].path, 128);
  detail::copy_fixed(type, mounts_[index].root->owner().name(), 32);
  return 1;
}
int VirtualFileSystem::rename(const char *old_path, const char *new_path)
{
  Transaction transaction;
  xv::sync::ScopedLock guard(namespace_);
  char old_name[DIRSIZ]{}, new_name[DIRSIZ]{};
  auto *from = walk(old_path, true, old_name);
  auto *to = walk(new_path, true, new_name);
  int result = -1;
  auto dot = [](const char *name) { return detail::compare(name, ".", DIRSIZ) == 0 || detail::compare(name, "..", DIRSIZ) == 0; };
  if (from && to && &from->owner() == &to->owner() && !dot(old_name) && !dot(new_name))
  {
    lock(from); auto *source = lookup_entry(from, old_name, nullptr); unlock(from);
    lock(to); auto *target = lookup_entry(to, new_name, nullptr); unlock(to);
    bool valid = source != nullptr;
    for (unsigned i = 0; i < mount_count_; ++i)
      if (same(source, mounts_[i].covered) || same(target, mounts_[i].covered) ||
          (source && ancestor(source, mounts_[i].parent)) || (target && ancestor(target, mounts_[i].parent))) valid = false;
    if (source) { lock(source); bool directory = source->kind() == T_DIR; unlock(source); if (directory && ancestor(source, to)) valid = false; }
    if (valid) result = from->owner().rename_at(from, old_name, to, new_name);
    if (source) release(source);
    if (target) release(target);
  }
  if (from) release(from);
  if (to) release(to);
  return result;
}

int VirtualFileSystem::symlink(const char *target, const char *path)
{
  unsigned size = 0; while (target[size]) { if (++size >= 128) return -1; }
  if (!size) return -1;
  Transaction transaction;
  xv::sync::ScopedLock guard(namespace_);
  char name[DIRSIZ]; auto *parent = walk(path, true, name);
  if (!parent) return -1;
  auto *node = parent->owner().create_at(parent, name, T_SYMLINK, 0, 0);
  int result = -1;
  if (node)
  {
    result = node->owner().write(node, 0, reinterpret_cast<unsigned long>(target), 0, size) == static_cast<int>(size) ? 0 : -1;
    unlock_release(node);
    if (result < 0) parent->owner().unlink_at(parent, name);
  }
  release(parent); return result;
}

int VirtualFileSystem::mount_disk(const char *path)
{
  if (!storage_ready_ || !path || path[0] != '/' || !path[1]) return -1;
  Transaction transaction; xv::sync::ScopedLock guard(namespace_);
  if (mount_count_ == 8) return -1;
  for (unsigned i = 0; i < mount_count_; ++i) if (&mounts_[i].root->owner() == storage_) return -1;
  char name[DIRSIZ]; auto *covered = walk(path, false, name);
  if (!covered) return -1;
  lock(covered); bool valid = covered->kind() == T_DIR; unlock(covered);
  for (unsigned i = 0; i < mount_count_; ++i) valid &= !same(covered, mounts_[i].root);
  auto *parent = valid ? walk(path, true, name) : nullptr;
  if (!parent) { release(covered); return -1; }
  auto &record = mounts_[mount_count_++]; record.covered = covered; record.parent = parent; record.root = storage_->root(); record.dynamic = -2;
  detail::copy_fixed(record.path, path, sizeof(record.path)); return 0;
}

}
