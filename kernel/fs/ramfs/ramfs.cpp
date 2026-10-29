module;
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"
#include "kernel/support/placement_new.hpp"
module xv.fs;
import :ramfs_flow;
import :ramfs;
import :platform;

namespace xv::fs
{
void RamFileSystem::initialize_inodes()
{
  tree_.init("ramfs tree");
  references_.init("ramfs references");
}

void RamFileSystem::initialize(int device)
{
  if (!initialize_mount(device))
  {
    detail::fail("ramfs: root allocation");
  }
}

bool RamFileSystem::initialize_mount(unsigned device)
{
  device_ = device;
  tree_.acquire();
  InitializeMountFlow::Context context{*this};
  (void)InitializeMountFlow::Graph::run<0>(context);
  return context.result;
}

Inode *RamFileSystem::root()
{
  return retain(&root_->inode);
}

RamFileSystem::Node *RamFileSystem::make(short type, Node *parent)
{
  static_assert(sizeof(Node) <= 4096 && sizeof(DirectoryPage) <= 4096);
  MakeFlow::Context context{*this, type, parent};
  (void)MakeFlow::Graph::run<0>(context);
  return context.result;
}

Inode *RamFileSystem::retain(Inode *node)
{
  xv::sync::ScopedLock guard(references_);
  ++node->ref;
  ++live_references_;
  return node;
}

void RamFileSystem::release(Inode *node)
{
  references_.acquire();
  ReleaseFlow::Context context{*this, node};
  (void)ReleaseFlow::Graph::run<0>(context);
}

void RamFileSystem::reclaim(Node *node)
{
  ReclaimFlow::Context context{*this, node};
  (void)ReclaimFlow::Graph::run<0>(context);
}

void RamFileSystem::unlock_release(Inode *node)
{
  unlock(node);
  release(node);
}

RamFileSystem::Slot *RamFileSystem::slot(Node *node, unsigned index, bool create)
{
  constexpr unsigned per_page = sizeof(DirectoryPage::slots) / sizeof(Slot);
  auto **link = &node->directory;
  for (;;)
  {
    if (!*link)
    {
      if (!create)
      {
        return nullptr;
      }
      auto *memory = detail::allocate_page();
      if (!memory)
      {
        return nullptr;
      }
      *link = new (memory) DirectoryPage{};
    }
    if (index < per_page)
    {
      return &(*link)->slots[index];
    }
    index -= per_page;
    link = &(*link)->next;
  }
}

RamFileSystem::Slot *RamFileSystem::find(Node *node, const char *name, unsigned *index)
{
  for (unsigned i = 0; i < node->slots; ++i)
  {
    auto *entry = slot(node, i, false);
    if (entry->target && detail::compare(entry->name, name, DIRSIZ) == 0)
    {
      if (index)
      {
        *index = i;
      }
      return entry;
    }
  }
  return nullptr;
}

bool RamFileSystem::insert(Node *parent, const char *name, Node *target)
{
  unsigned index;
  for (index = 0; index < parent->slots; ++index)
  {
    if (!slot(parent, index, false)->target)
    {
      break;
    }
  }
  auto *entry = slot(parent, index, true);
  if (!entry)
  {
    return false;
  }
  entry->target = target;
  detail::copy_fixed(entry->name, name, DIRSIZ);
  if (index == parent->slots)
  {
    ++parent->slots;
  }
  parent->inode.size = (parent->slots + 2) * sizeof(dirent);
  return true;
}

bool RamFileSystem::empty(Node *node)
{
  for (unsigned i = 0; i < node->slots; ++i)
  {
    if (slot(node, i, false)->target)
    {
      return false;
    }
  }
  return true;
}

Inode *RamFileSystem::lookup_entry(Inode *directory, const char *name, unsigned *offset)
{
  if (directory->type != T_DIR)
  {
    return nullptr;
  }
  auto *node = state(directory);
  if (detail::compare(name, ".", DIRSIZ) == 0)
  {
    if (offset)
    {
      *offset = 0;
    }
    return retain(directory);
  }
  if (detail::compare(name, "..", DIRSIZ) == 0)
  {
    if (offset)
    {
      *offset = sizeof(dirent);
    }
    return retain(&node->parent->inode);
  }
  unsigned index;
  auto *entry = find(node, name, &index);
  if (!entry)
  {
    return nullptr;
  }
  if (offset)
  {
    *offset = (index + 2) * sizeof(dirent);
  }
  return retain(&entry->target->inode);
}

Inode *RamFileSystem::create_at(Inode *directory, const char *name, short type, short,
                                short)
{
  tree_.acquire();
  CreateFlow::Context context{*this, directory, name, type, state(directory)};
  (void)CreateFlow::Graph::run<0>(context);
  return context.result;
}

int RamFileSystem::link_at(Inode *directory, const char *name, Inode *target)
{
  xv::sync::ScopedLock guard(tree_);
  LinkFlow::Context context{*this, directory, name, target};
  (void)LinkFlow::Graph::run<0>(context);
  return context.result;
}

int RamFileSystem::unlink_at(Inode *directory, const char *name)
{
  xv::sync::ScopedLock guard(tree_);
  UnlinkFlow::Context context{*this, directory, name};
  (void)UnlinkFlow::Graph::run<0>(context);
  return context.result;
}

void RamFileSystem::truncate(Inode *node)
{
  if (node->type != T_FILE)
  {
    detail::fail("ramfs: truncate non-file");
  }
  state(node)->contents.clear();
  node->size = 0;
}

int RamFileSystem::read(Inode *node, int user, unsigned long destination,
                        unsigned offset, unsigned count)
{
  ReadFlow::Context context{*this, node, user, destination, offset, count};
  (void)ReadFlow::Graph::run<0>(context);
  return context.result;
}

int RamFileSystem::write(Inode *node, int user, unsigned long source, unsigned offset,
                         unsigned count)
{
  if (node->type != T_FILE && node->type != T_SYMLINK)
  {
    return -1;
  }
  int result = state(node)->contents.write(offset, user, source, count);
  node->size = state(node)->contents.size();
  return result;
}

void RamFileSystem::stat(Inode *node, Stat *result)
{
  *result = {};
  result->dev = node->dev;
  result->ino = node->inum;
  result->type = node->type;
  result->nlink = node->nlink;
  result->size = node->size;
  node->security_stat(result);
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
    auto *node = nodes_;
    nodes_ = node->next;
    while (node->directory)
    {
      auto *page = node->directory;
      node->directory = page->next;
      page->~DirectoryPage();
      detail::free_page(page);
    }
    node->~Node();
    detail::free_page(node);
  }
  detail::fill(numbers_, 0, sizeof(numbers_));
  root_ = nullptr;
  live_references_ = 0;
}

int RamFileSystem::rename_at(Inode *from, const char *old_name, Inode *to,
                             const char *new_name)
{
  xv::sync::ScopedLock guard(tree_);
  RenameFlow::Context context{*this, from, old_name, to, new_name};
  (void)RenameFlow::Graph::run<0>(context);
  return context.result;
}

int RamFileSystem::resize(Inode *node, unsigned size)
{
  if (node->kind() != T_FILE || !state(node)->contents.resize(size))
  {
    return -1;
  }
  node->size = size;
  return 0;
}

} // namespace xv::fs
