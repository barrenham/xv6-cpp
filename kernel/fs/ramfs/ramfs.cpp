module;
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/support/placement_new.hpp"
module xv.fs;
import :ramfs;
import :platform;
namespace xv::fs
{
void RamFileSystem::initialize_inodes()
{ tree_.init("ramfs tree"); references_.init("ramfs references"); }
void RamFileSystem::initialize(int device)
{ if (!initialize_mount(device)) detail::fail("ramfs: root allocation"); }
bool RamFileSystem::initialize_mount(unsigned device)
{
  device_ = device;
  tree_.acquire();
  root_ = make(T_DIR, nullptr);
  if (!root_) { tree_.release(); return false; }
  root_->parent = root_;
  root_->inode.nlink = 1;
  root_->inode.uid_ = root_->inode.gid_ = 0; root_->inode.mode_ = 01777;
  tree_.release();
  return true;
}
Inode *RamFileSystem::root() { return retain(&root_->inode); }
RamFileSystem::Node *RamFileSystem::make(short type, Node *parent)
{
  static_assert(sizeof(Node) <= 4096 && sizeof(DirectoryPage) <= 4096);
  unsigned number;
  for (number = 1; number < 65536; ++number)
    if (!(numbers_[number / 64] & (1UL << (number % 64)))) break;
  if (number == 65536) return nullptr;
  auto *memory = detail::allocate_page();
  if (!memory) return nullptr;
  auto *node = new (memory) Node{};
  node->next = nodes_; nodes_ = node;
  { xv::sync::ScopedLock guard(references_); ++live_references_; }
  numbers_[number / 64] |= 1UL << (number % 64);
  node->inode.owner_ = this; node->inode.context_ = node;
  node->inode.dev = device_; node->inode.inum = number;
  auto who = detail::identity();
  node->inode.uid_ = who.uid; node->inode.gid_ = who.gid;
  node->inode.mode_ = (type == T_DIR ? 0777 : 0666) & ~who.mask;
  node->inode.type = type; node->inode.valid = 1; node->inode.ref = 1;
  if (type == T_DIR)
  {
    node->inode.size = 2 * sizeof(dirent);
    node->parent = parent;
    if (parent) { retain(&parent->inode); xv::sync::ScopedLock guard(references_); --live_references_; }
  }
  return node;
}
Inode *RamFileSystem::retain(Inode *node)
{ xv::sync::ScopedLock guard(references_); ++node->ref; ++live_references_; return node; }
void RamFileSystem::release(Inode *node)
{
  references_.acquire();
  if (node->ref < 1) detail::fail("ramfs: release dead reference");
  if (node->ref > 1) { --live_references_; --node->ref; references_.release(); return; }
  references_.release();
  bool held = tree_.holding();
  if (!held) tree_.acquire();
  references_.acquire();
  --live_references_;
  bool destroy = --node->ref == 0 && node->nlink == 0;
  references_.release();
  if (destroy) reclaim(state(node));
  if (!held) tree_.release();
}
void RamFileSystem::reclaim(Node *node)
{
  while (node)
  {
    auto **link = &nodes_; while (*link != node) link = &(*link)->next; *link = node->next;
    Node *parent = node->parent != node ? node->parent : nullptr;
    numbers_[node->inode.inum / 64] &= ~(1UL << (node->inode.inum % 64));
    while (node->directory)
    {
      auto *page = node->directory;
      node->directory = page->next;
      page->~DirectoryPage(); detail::free_page(page);
    }
    node->~Node(); detail::free_page(node);
    if (!parent) break;
    references_.acquire();
    bool destroy = --parent->inode.ref == 0 && parent->inode.nlink == 0;
    references_.release();
    node = destroy ? parent : nullptr;
  }
}
void RamFileSystem::unlock_release(Inode *node) { unlock(node); release(node); }
RamFileSystem::Slot *RamFileSystem::slot(Node *node, unsigned index, bool create)
{
  constexpr unsigned per_page = sizeof(DirectoryPage::slots) / sizeof(Slot);
  auto **link = &node->directory;
  for (;;)
  {
    if (!*link)
    {
      if (!create) return nullptr;
      auto *memory = detail::allocate_page();
      if (!memory) return nullptr;
      *link = new (memory) DirectoryPage{};
    }
    if (index < per_page) return &(*link)->slots[index];
    index -= per_page; link = &(*link)->next;
  }
}
RamFileSystem::Slot *RamFileSystem::find(Node *node, const char *name, unsigned *index)
{
  for (unsigned i = 0; i < node->slots; ++i)
  {
    auto *entry = slot(node, i, false);
    if (entry->target && detail::compare(entry->name, name, DIRSIZ) == 0)
    { if (index) *index = i; return entry; }
  }
  return nullptr;
}
bool RamFileSystem::insert(Node *parent, const char *name, Node *target)
{
  unsigned index;
  for (index = 0; index < parent->slots; ++index) if (!slot(parent, index, false)->target) break;
  auto *entry = slot(parent, index, true);
  if (!entry) return false;
  entry->target = target; detail::copy_fixed(entry->name, name, DIRSIZ);
  if (index == parent->slots) ++parent->slots;
  parent->inode.size = (parent->slots + 2) * sizeof(dirent);
  return true;
}
bool RamFileSystem::empty(Node *node)
{
  for (unsigned i = 0; i < node->slots; ++i) if (slot(node, i, false)->target) return false;
  return true;
}
Inode *RamFileSystem::lookup_entry(Inode *directory, const char *name, unsigned *offset)
{
  if (directory->type != T_DIR) return nullptr;
  auto *node = state(directory);
  if (detail::compare(name, ".", DIRSIZ) == 0) { if (offset) *offset = 0; return retain(directory); }
  if (detail::compare(name, "..", DIRSIZ) == 0)
  { if (offset) *offset = sizeof(dirent); return retain(&node->parent->inode); }
  unsigned index;
  auto *entry = find(node, name, &index);
  if (!entry) return nullptr;
  if (offset) *offset = (index + 2) * sizeof(dirent);
  return retain(&entry->target->inode);
}
Inode *RamFileSystem::create_at(Inode *directory, const char *name, short type, short, short)
{
  tree_.acquire();
  auto *parent = state(directory);
  if (directory->type != T_DIR || !directory->nlink || !detail::allowed(directory, 3) || (type != T_FILE && type != T_DIR) ||
      (type == T_DIR && directory->nlink == NLINK_MAX))
  { tree_.release(); return nullptr; }
  auto *existing = lookup_entry(directory, name, nullptr);
  if (existing)
  {
    if (type == T_FILE && existing->type == T_FILE) return existing;
    release(existing); tree_.release(); return nullptr;
  }
  auto *node = make(type, type == T_DIR ? parent : nullptr);
  if (!node) { tree_.release(); return nullptr; }
  if (!insert(parent, name, node))
  { release(&node->inode); tree_.release(); return nullptr; }
  node->inode.nlink = 1;
  if (type == T_DIR) ++directory->nlink;
  return &node->inode;
}
int RamFileSystem::link_at(Inode *directory, const char *name, Inode *target)
{
  xv::sync::ScopedLock guard(tree_);
  if (&target->owner() != this || directory->type != T_DIR || !directory->nlink || !detail::allowed(directory, 3) ||
      target->type == T_DIR || !target->nlink || target->nlink == NLINK_MAX ||
      detail::compare(name, ".", DIRSIZ) == 0 || detail::compare(name, "..", DIRSIZ) == 0 ||
      find(state(directory), name)) return -1;
  if (!insert(state(directory), name, state(target))) return -1;
  ++target->nlink;
  return 0;
}
int RamFileSystem::unlink_at(Inode *directory, const char *name)
{
  xv::sync::ScopedLock guard(tree_);
  if (directory->type != T_DIR || !directory->nlink || !detail::allowed(directory, 3) || detail::compare(name, ".", DIRSIZ) == 0 ||
      detail::compare(name, "..", DIRSIZ) == 0) return -1;
  auto *entry = find(state(directory), name);
  if (!entry || (entry->target->inode.type == T_DIR && !empty(entry->target))) return -1;
  if (!detail::may_remove(directory, &entry->target->inode)) return -1;
  auto *target = retain(&entry->target->inode);
  entry->target = nullptr;
  if (target->type == T_DIR) --directory->nlink;
  --target->nlink;
  release(target);
  return 0;
}
void RamFileSystem::truncate(Inode *node)
{
  if (node->type != T_FILE) detail::fail("ramfs: truncate non-file");
  state(node)->contents.clear(); node->size = 0;
}
int RamFileSystem::read(Inode *node, int user, unsigned long destination, unsigned offset, unsigned count)
{
  auto *data = state(node);
  if (node->type == T_FILE) return data->contents.read(offset, user, destination, count);
  if (node->type != T_DIR) return -1;
  if (offset >= node->size) return 0;
  if (count > node->size - offset) count = node->size - offset;
  unsigned done = 0;
  while (done < count)
  {
    unsigned index = (offset + done) / sizeof(dirent), inside = (offset + done) % sizeof(dirent);
    dirent record{};
    if (index < 2)
    { record.inum = index ? data->parent->inode.inum : node->inum; detail::copy_fixed(record.name, index ? ".." : ".", DIRSIZ); }
    else
    {
      auto *entry = slot(data, index - 2, false);
      if (entry->target) { record.inum = entry->target->inode.inum; detail::copy_fixed(record.name, entry->name, DIRSIZ); }
    }
    unsigned part = sizeof(record) - inside;
    if (part > count - done) part = count - done;
    if (detail::copy_out(user, destination + done, reinterpret_cast<const char *>(&record) + inside, part) < 0)
      return done ? static_cast<int>(done) : -1;
    done += part;
  }
  return done;
}
int RamFileSystem::write(Inode *node, int user, unsigned long source, unsigned offset, unsigned count)
{
  if (node->type != T_FILE) return -1;
  int result = state(node)->contents.write(offset, user, source, count);
  node->size = state(node)->contents.size();
  return result;
}
void RamFileSystem::stat(Inode *node, Stat *result)
{
  *result = {}; result->dev = node->dev; result->ino = node->inum;
  result->type = node->type; result->nlink = node->nlink; result->size = node->size; node->security_stat(result);
}
bool RamFileSystem::busy()
{
  xv::sync::ScopedLock guard(references_);
  return live_references_ != 2;
}
void RamFileSystem::shutdown()
{
  xv::sync::ScopedLock guard(tree_);
  while (nodes_)
  {
    auto *node = nodes_; nodes_ = node->next;
    while (node->directory)
    {
      auto *page = node->directory; node->directory = page->next;
      page->~DirectoryPage(); detail::free_page(page);
    }
    node->~Node(); detail::free_page(node);
  }
  detail::fill(numbers_, 0, sizeof(numbers_));
  root_ = nullptr; live_references_ = 0;
}
int RamFileSystem::rename_at(Inode *from, const char *old_name, Inode *to, const char *new_name)
{
  xv::sync::ScopedLock guard(tree_);
  auto *source = find(state(from), old_name);
  auto *destination = find(state(to), new_name);
  if (!source || !from->nlink || !to->nlink || !detail::allowed(from, 3) || !detail::allowed(to, 3) ||
      !detail::may_remove(from, &source->target->inode)) return -1;
  auto *moving = source->target;
  if (destination && destination->target == moving) return 0;
  if (destination && (!detail::may_remove(to, &destination->target->inode) ||
      (moving->inode.type == T_DIR) != (destination->target->inode.type == T_DIR) ||
      (destination->target->inode.type == T_DIR && !empty(destination->target)))) return -1;
  if (moving->inode.type == T_DIR && from != to && to->nlink == NLINK_MAX && !destination) return -1;
  if (destination)
  {
    auto *replaced = retain(&destination->target->inode);
    if (replaced->type == T_DIR) --to->nlink;
    --replaced->nlink; destination->target = moving; release(replaced);
  }
  else if (!insert(state(to), new_name, moving)) return -1;
  source->target = nullptr;
  if (moving->inode.type == T_DIR && from != to)
  {
    --from->nlink; ++to->nlink;
    retain(to);
    { xv::sync::ScopedLock references(references_); --live_references_; --from->ref; }
    moving->parent = state(to);
  }
  return 0;
}

}
