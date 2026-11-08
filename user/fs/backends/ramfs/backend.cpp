module;
#include "kernel/support/types.h"
#include "kernel/fs/userfs/protocol.h"
#include "kernel/fs/stat.h"
#include "user/fs/backends/xv6/format.h"
#include "kernel/support/placement_new.hpp"
module xv.userfs.ramfs;
import xv.userfs;

namespace userfs
{
namespace
{
constexpr unsigned PageBytes = 4096;
void clear(void *data, unsigned count)
{
  auto *out = static_cast<unsigned char *>(data);
  while (count--) *out++ = 0;
}
void copy(void *data, const void *source, unsigned count)
{
  auto *out = static_cast<unsigned char *>(data);
  auto *in = static_cast<const unsigned char *>(source);
  while (count--) *out++ = *in++;
}
bool same(const char *left, const char *right)
{
  while (*left && *left == *right) { ++left; ++right; }
  return *left == *right;
}
void text(char *destination, const char *source)
{
  do { *destination++ = *source; } while (*source++);
}
}

struct MemoryBackend::Page
{
  Page *next = nullptr;
  unsigned index = 0;
  unsigned char bytes[PageBytes]{};
};
struct MemoryBackend::Name
{
  Name *next = nullptr;
  Node *node = nullptr;
  unsigned long cookie = 0;
  char text[NameSize]{};
};
struct MemoryBackend::Node
{
  Node *next = nullptr, *parent = nullptr;
  Attributes attributes;
  Page *pages = nullptr;
  Name *names = nullptr, *tail = nullptr;
  unsigned long next_cookie = 2;
  unsigned children = 0;
  bool borrowed = false;
};

MemoryBackend::MemoryBackend(MemoryStorage &storage, unsigned root_mode)
    : storage_(storage) {
  auto *memory = storage_.allocate(sizeof(Node));
  if (!memory || root_mode > 07777) { if (memory) storage_.release(memory); return; }
  root_ = nodes_ = new (memory) Node{};
  root_->parent = root_;
  root_->attributes = {1, 2 * sizeof(dirent), 0, 0, root_mode, T_DIR, 2};
}

MemoryBackend::~MemoryBackend()
{
  while (nodes_)
  {
    auto *node = nodes_;
    nodes_ = node->next;
    while (node->pages)
    {
      auto *page = node->pages;
      node->pages = page->next;
      storage_.release(page);
    }
    while (node->names)
    {
      auto *name = node->names;
      node->names = name->next;
      storage_.release(name);
    }
    storage_.release(node);
  }
}

MemoryBackend::Node *MemoryBackend::find(unsigned identity) const
{
  for (auto *node = nodes_; node; node = node->next)
    if (node->attributes.node == identity) return node;
  return nullptr;
}
bool MemoryBackend::directory(const Node &node)
{
  return node.attributes.type == T_DIR;
}
bool MemoryBackend::valid_name(const char *name)
{
  if (!name || !*name || same(name, ".") || same(name, "..")) return false;
  for (unsigned i = 0; i < NameSize; ++i)
  {
    if (!name[i]) return true;
    if (name[i] == '/') return false;
  }
  return false;
}
MemoryBackend::Name **MemoryBackend::entry(Node &parent, const char *name)
{
  auto **link = &parent.names;
  while (*link && !same((*link)->text, name)) link = &(*link)->next;
  return link;
}
MemoryBackend::Page **MemoryBackend::page(Node &node, unsigned index)
{
  auto **link = &node.pages;
  while (*link && (*link)->index < index) link = &(*link)->next;
  return link;
}
void MemoryBackend::directory_size(Node &node)
{
  node.attributes.size = node.next_cookie > ~0U / sizeof(dirent)
                             ? ~0U : unsigned(node.next_cookie * sizeof(dirent));
}
Attributes MemoryBackend::root()
{
  return root_ ? root_->attributes : Attributes{};
}
int MemoryBackend::getattr(unsigned identity, Attributes &out)
{
  auto *node = find(identity);
  if (!node) return -1;
  out = node->attributes;
  return 0;
}
int MemoryBackend::lookup(unsigned parent, const char *name, Attributes &out)
{
  auto *directory = find(parent);
  if (!directory || !MemoryBackend::directory(*directory)) return -1;
  Node *node = nullptr;
  if (same(name, ".")) node = directory;
  else if (same(name, "..")) node = directory->parent;
  else if (auto *found = *entry(*directory, name)) node = found->node;
  if (!node) return -1;
  node->borrowed = true;
  out = node->attributes;
  return 0;
}

int MemoryBackend::create(unsigned parent, const char *name,
                           const Attributes &requested, Attributes &out)
{
  auto *directory = find(parent);
  if (!directory || !MemoryBackend::directory(*directory) ||
      !directory->attributes.links || !valid_name(name) || requested.mode > 07777)
    return -1;
  auto *found = *entry(*directory, name);
  if (found)
  {
    if (requested.type != T_FILE || found->node->attributes.type != T_FILE) return -1;
    found->node->borrowed = true;
    out = found->node->attributes;
    return 0;
  }
  if ((requested.type != T_DIR && requested.type != T_FILE && requested.type != T_SYMLINK && requested.type != T_SOCKET) ||
      !next_node_ || directory->next_cookie == ~0UL ||
      (requested.type == T_DIR && directory->attributes.links == NLINK_MAX)) return -1;
  auto *node_memory = storage_.allocate(sizeof(Node));
  if (!node_memory) return -1;
  auto *name_memory = storage_.allocate(sizeof(Name));
  if (!name_memory) { storage_.release(node_memory); return -1; }
  auto *node = new (node_memory) Node{};
  auto *slot = new (name_memory) Name{};
  node->attributes = {next_node_++, 0, requested.uid, requested.gid,
                      requested.mode, requested.type, requested.type == T_DIR ? 2U : 1U};
  node->borrowed = true;
  if (MemoryBackend::directory(*node))
  {
    node->parent = directory;
    ++directory->children;
    ++directory->attributes.links;
    directory_size(*node);
  }
  node->next = nodes_;
  nodes_ = node;
  slot->node = node;
  slot->cookie = directory->next_cookie++;
  text(slot->text, name);
  append(*directory, slot);
  directory_size(*directory);
  out = node->attributes;
  return 0;
}

void MemoryBackend::reclaim(Node *node)
{
  // An unlinked cwd keeps its entire parent chain valid until its last borrow.
  while (node && node != root_ && !node->borrowed && !node->attributes.links && !node->children)
  {
    auto *parent = node->parent;
    auto **link = &nodes_;
    while (*link != node) link = &(*link)->next;
    *link = node->next;
    while (node->pages)
    {
      auto *page = node->pages;
      node->pages = page->next;
      storage_.release(page);
    }
    if (parent) --parent->children;
    storage_.release(node);
    node = parent;
  }
}
void MemoryBackend::forget(unsigned identity)
{
  auto *node = find(identity);
  if (node) { node->borrowed = false; reclaim(node); }
}
void MemoryBackend::append(Node &parent, Name *slot)
{
  slot->next = nullptr;
  if (parent.tail) parent.tail->next = slot;
  else parent.names = slot;
  parent.tail = slot;
}
void MemoryBackend::remove(Node &parent, Name *slot)
{
  Name *previous = nullptr;
  auto **link = &parent.names;
  while (*link != slot) { previous = *link; link = &(*link)->next; }
  *link = slot->next;
  if (parent.tail == slot) parent.tail = previous;
}
void MemoryBackend::drop(Node &parent, Node &node)
{
  if (directory(node))
  {
    node.attributes.links = 0;
    --parent.attributes.links;
  }
  else --node.attributes.links;
}
int MemoryBackend::unlink(unsigned parent, const char *name, Attributes &out)
{
  auto *directory = find(parent);
  if (!directory || !MemoryBackend::directory(*directory) || !directory->attributes.links ||
      !valid_name(name)) return -1;
  auto *found = *entry(*directory, name);
  if (!found || (MemoryBackend::directory(*found->node) && found->node->names)) return -1;
  auto *node = found->node;
  remove(*directory, found);
  drop(*directory, *node);
  out = node->attributes;
  storage_.release(found);
  reclaim(node);
  return 0;
}
int MemoryBackend::link(unsigned parent, const char *name, unsigned target)
{
  auto *directory = find(parent), *node = find(target);
  if (!directory || !node || !MemoryBackend::directory(*directory) ||
      !directory->attributes.links || MemoryBackend::directory(*node) ||
      !node->attributes.links || node->attributes.links == NLINK_MAX ||
      !valid_name(name) || *entry(*directory, name) || directory->next_cookie == ~0UL) return -1;
  auto *memory = storage_.allocate(sizeof(Name));
  if (!memory) return -1;
  auto *slot = new (memory) Name{};
  slot->node = node;
  slot->cookie = directory->next_cookie++;
  text(slot->text, name);
  append(*directory, slot);
  ++node->attributes.links;
  directory_size(*directory);
  return 0;
}

int MemoryBackend::rename(unsigned from, const char *old_name, unsigned to,
                           const char *new_name)
{
  auto *source_parent = find(from), *target_parent = find(to);
  if (!source_parent || !target_parent || !directory(*source_parent) ||
      !directory(*target_parent) || !source_parent->attributes.links ||
      !target_parent->attributes.links || !valid_name(old_name) || !valid_name(new_name)) return -1;
  auto *source = *entry(*source_parent, old_name), *target = *entry(*target_parent, new_name);
  if (!source) return -1;
  if (target && source->node == target->node) return 0;
  if (target && (directory(*source->node) != directory(*target->node) ||
                 (directory(*target->node) && target->node->names))) return -1;
  if (directory(*source->node))
  {
    for (auto *ancestor = target_parent;; ancestor = ancestor->parent)
    {
      if (ancestor == source->node) return -1;
      if (ancestor == root_) break;
    }
    if (source_parent != target_parent && !target &&
        target_parent->attributes.links == NLINK_MAX) return -1;
  }
  if (target_parent->next_cookie == ~0UL) return -1;
  if (target)
  {
    auto *replaced = target->node;
    remove(*target_parent, target);
    drop(*target_parent, *replaced);
    storage_.release(target);
    reclaim(replaced);
  }
  remove(*source_parent, source);
  if (directory(*source->node) && source_parent != target_parent)
  {
    --source_parent->attributes.links;
    ++target_parent->attributes.links;
    --source_parent->children;
    ++target_parent->children;
    source->node->parent = target_parent;
  }
  source->cookie = target_parent->next_cookie++;
  text(source->text, new_name);
  append(*target_parent, source);
  directory_size(*target_parent);
  return 0;
}

int MemoryBackend::readdir(unsigned identity, unsigned long offset, Entry &out)
{
  auto *node = find(identity);
  if (!node || !directory(*node)) return -1;
  if (offset < 2)
  {
    out.attributes = offset ? node->parent->attributes : node->attributes;
    text(out.name, offset ? ".." : ".");
    out.next = offset + 1;
    return 1;
  }
  auto *found = node->names;
  while (found && found->cookie < offset) found = found->next;
  if (!found) return 0;
  out.attributes = found->node->attributes;
  text(out.name, found->text);
  out.next = found->cookie + 1;
  return 1;
}

int MemoryBackend::read(unsigned identity, unsigned long offset, void *data, unsigned count)
{
  auto *node = find(identity);
  if (!node) return -1;
  if (directory(*node))
  {
    // Entries follow cookie order. Skip the legacy record prefix once rather
    // than re-enumerating and rescanning every earlier entry for each byte range.
    unsigned copied = 0;
    unsigned long index = offset / sizeof(dirent);
    unsigned start = offset % sizeof(dirent);
    auto *slot = node->names;
    for (unsigned long skip = 2; slot && skip < index; ++skip) slot = slot->next;
    while (copied < count)
    {
      dirent record{};
      if (index < 2)
      {
        record.inum = index ? node->parent->attributes.node : node->attributes.node;
        text(record.name, index ? ".." : ".");
      }
      else
      {
        if (!slot) break;
        record.inum = slot->node->attributes.node;
        for (unsigned i = 0; i < DIRSIZ && slot->text[i]; ++i) record.name[i] = slot->text[i];
        slot = slot->next;
      }
      unsigned take = sizeof(record) - start;
      if (take > count - copied) take = count - copied;
      copy(static_cast<unsigned char *>(data) + copied,
            reinterpret_cast<unsigned char *>(&record) + start, take);
      copied += take;
      start = 0;
      ++index;
    }
    return copied;
  }
  if (offset >= node->attributes.size) return 0;
  if (count > node->attributes.size - offset) count = node->attributes.size - offset;
  unsigned done = 0;
  while (done < count)
  {
    unsigned index = (offset + done) / PageBytes, start = (offset + done) % PageBytes;
    unsigned take = PageBytes - start;
    if (take > count - done) take = count - done;
    auto *found = *page(*node, index);
    auto *out = static_cast<unsigned char *>(data) + done;
    if (found && found->index == index) copy(out, found->bytes + start, take);
    else clear(out, take);
    done += take;
  }
  return done;
}

int MemoryBackend::write(unsigned identity, unsigned long offset, const void *data, unsigned count)
{
  auto *node = find(identity);
  if (!node || directory(*node) || count > xv::userfs::BulkSize || offset > ~0U ||
      count > ~0U - offset)
  {
    return -1;
  }
  if (!count) return 0;
  Page *created[xv::userfs::BulkSize / PageBytes + 1]{};
  unsigned allocated = 0;
  unsigned first = offset / PageBytes, last = (offset + count - 1) / PageBytes;
  static_assert(xv::userfs::BulkSize % PageBytes == 0);
  for (unsigned index = first; index <= last; ++index)
  {
    auto **link = page(*node, index);
    if (*link && (*link)->index == index) continue;
    auto *memory = storage_.allocate(sizeof(Page));
    if (!memory)
    {
      for (unsigned i = 0; i < allocated; ++i)
      {
        auto **link = page(*node, created[i]->index);
        *link = created[i]->next;
        storage_.release(created[i]);
      }
      return -1;
    }
    auto *next = new (memory) Page{};
    next->index = index;
    next->next = *link;
    *link = next;
    created[allocated++] = next;
  }
  unsigned done = 0;
  while (done < count)
  {
    auto *found = *page(*node, (offset + done) / PageBytes);
    unsigned start = (offset + done) % PageBytes, take = PageBytes - start;
    if (take > count - done) take = count - done;
    copy(found->bytes + start, static_cast<const unsigned char *>(data) + done, take);
    done += take;
  }
  if (offset + count > node->attributes.size) node->attributes.size = offset + count;
  return count;
}

int MemoryBackend::resize(unsigned identity, unsigned size)
{
  auto *node = find(identity);
  if (!node || directory(*node)) return -1;
  auto **link = &node->pages;
  while (*link)
  {
    auto *current = *link;
    if (current->index >= (static_cast<unsigned long>(size) + PageBytes - 1) / PageBytes)
    {
      *link = current->next;
      storage_.release(current);
    }
    else
    {
      if (size % PageBytes && current->index == size / PageBytes)
        clear(current->bytes + size % PageBytes, PageBytes - size % PageBytes);
      link = &current->next;
    }
  }
  node->attributes.size = size;
  return 0;
}
int MemoryBackend::setattr(unsigned identity, const Attributes &attributes)
{
  auto *node = find(identity);
  if (!node || attributes.mode > 07777) return -1;
  node->attributes.uid = attributes.uid;
  node->attributes.gid = attributes.gid;
  node->attributes.mode = attributes.mode;
  return 0;
}
} // namespace userfs
