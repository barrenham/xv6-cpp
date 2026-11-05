module;
#include "kernel/support/placement_new.hpp"
#include "kernel/fs/stat.h"
#include "kernel/io/posix_errno.h"
module xv.fs;
import :mapping;
import :platform;

namespace xv::fs
{
namespace
{
class FileMappingBacking final : public xv::memory::MappingBacking
{
public:

  explicit FileMappingBacking(FileMapping &owner) : owner_(owner) {
  }

  MappingBacking *
  retain() override {
    owner_.retain();
    return this;
  }

  void
  release() override {
    owner_.release();
  }

  int
  enable_write() override {
    return owner_.enable_write();
  }

  int
  synchronize() override {
    return owner_.synchronize();
  }

  bool
  page_available(unsigned index) const override {
    return owner_.page_available(index);
  }

  unsigned long
  length() const override {
    return owner_.length();
  }

  void *
  page(unsigned index) const override {
    return owner_.page(index);
  }

private:

  FileMapping &owner_;
};
} // namespace

xv::memory::MappingBacking *
FileMapping::backing() {
  return reinterpret_cast<FileMappingBacking *>(reinterpret_cast<char *>(this) + 4096 -
                                                sizeof(FileMappingBacking));
}

struct MappedPages
{
  struct Entry
  {
    void *frame = nullptr;
    unsigned index = 0, leases = 0, writers = 0;
  };

  MappedPages *next = nullptr;
  Entry entries[160];

  static Entry *
  find(Inode *node, unsigned index) {
    for (auto *block = node->mapped_pages_; block; block = block->next)
    {
      for (auto &entry : block->entries)
      {
        if (entry.frame && entry.index == index)
        {
          return &entry;
        }
      }
    }
    return nullptr;
  }

  static void
  cleanup(Inode *node) {
    auto **link = &node->mapped_pages_;
    while (auto *block = *link)
    {
      bool empty = true;
      for (auto &entry : block->entries)
      {
        empty &= !entry.frame;
      }
      if (empty)
      {
        *link = block->next;
        block->~MappedPages();
        detail::free_page(block);
      }
      else
      {
        link = &block->next;
      }
    }
  }

  static void
  drop(Entry *entry, bool writing) {
    if (writing)
    {
      --entry->writers;
    }
    if (!--entry->leases)
    {
      detail::release_frame(entry->frame);
      *entry = {};
    }
  }

  static Entry *
  acquire(Inode *node, unsigned index, int &error) {
    if (auto *found = find(node, index))
    {
      ++found->leases;
      return found;
    }
    Entry *slot = nullptr;
    for (auto *block = node->mapped_pages_; block && !slot; block = block->next)
    {
      for (auto &entry : block->entries)
      {
        if (!entry.frame)
        {
          slot = &entry;
          break;
        }
      }
    }
    if (!slot)
    {
      static_assert(sizeof(MappedPages) <= 4096);
      auto *page = detail::allocate_page();
      if (!page)
      {
        error = -ENOMEM;
        return nullptr;
      }
      auto *block = new (page) MappedPages{};
      block->next = node->mapped_pages_;
      node->mapped_pages_ = block;
      slot = block->entries;
    }
    auto *frame = detail::allocate_page();
    if (!frame)
    {
      error = -ENOMEM;
      return nullptr;
    }
    detail::fill(frame, 0, 4096);
    unsigned offset = index * 4096;
    unsigned bytes = offset < node->size_bytes() ? node->size_bytes() - offset : 0;
    if (bytes > 4096)
    {
      bytes = 4096;
    }
    if (bytes &&
        node->owner().read_data(node, 0, reinterpret_cast<unsigned long>(frame), offset,
                                bytes) != int(bytes))
    {
      detail::free_page(frame);
      error = -EIO;
      return nullptr;
    }
    *slot = {frame, index, 1, 0};
    return slot;
  }

  static int
  flush(Inode *node) {
    bool writable = false;
    for (auto *block = node->mapped_pages_; block; block = block->next)
    {
      for (auto &entry : block->entries)
      {
        writable |=
            entry.frame && entry.writers && entry.index * 4096UL < node->size_bytes();
      }
    }
    if (!writable)
    {
      return 0;
    }
    // A mode change after a writable mmap must not let later mapped writes
    // preserve set-id bits when they are committed to the filesystem.
    if (node->mapped_pages_ && (node->mode() & 06000) &&
        !node->owner().attributes(node, node->uid(), node->gid(),
                                  node->mode() & ~06000U))
    {
      return -EIO;
    }
    for (auto *block = node->mapped_pages_; block; block = block->next)
    {
      for (auto &entry : block->entries)
      {
        unsigned offset = entry.index * 4096;
        if (!entry.frame || !entry.writers || offset >= node->size_bytes())
        {
          continue;
        }
        unsigned count = node->size_bytes() - offset;
        if (count > 4096)
        {
          count = 4096;
        }
        if (node->owner().write_data(node, 0,
                                     reinterpret_cast<unsigned long>(entry.frame),
                                     offset, count) != int(count))
        {
          return -EIO;
        }
      }
    }
    return 0;
  }
};

FileMapping *
FileMapping::create(Inode *node, unsigned offset, unsigned pages, int &error) {
  error = -EINVAL;
  if (!node || node->kind() != T_FILE || !pages || pages > 256 || offset % 4096 ||
      static_cast<unsigned long>(offset) + pages * 4096UL > (1UL << 32))
  {
    return nullptr;
  }
  static_assert(sizeof(FileMapping) + sizeof(FileMappingBacking) <= 4096);
  auto *page = detail::allocate_page();
  if (!page)
  {
    error = -ENOMEM;
    return nullptr;
  }
  auto *mapping = new (page) FileMapping{};
  new (mapping->backing()) FileMappingBacking(*mapping);
  mapping->node_ = node;
  mapping->offset_ = offset;
  {
    LockedInode guard(node->owner(), *node);
    Stat attributes;
    if (node->owner().status(node, &attributes) >= 0)
    {
      __atomic_store_n(&node->mapped_length_, node->size_bytes(), __ATOMIC_RELEASE);
      while (mapping->pages_ < pages)
      {
        auto *entry =
            MappedPages::acquire(node, offset / 4096 + mapping->pages_, error);
        if (!entry)
        {
          break;
        }
        mapping->entries_[mapping->pages_++] = entry;
      }
      if (mapping->pages_ == pages)
      {
        node->owner().retain(node);
        error = 0;
        return mapping;
      }
    }
    else
    {
      error = -EIO;
    }
    for (unsigned i = 0; i < mapping->pages_; ++i)
    {
      MappedPages::drop(static_cast<MappedPages::Entry *>(mapping->entries_[i]), false);
    }
    MappedPages::cleanup(node);
  }
  static_cast<FileMappingBacking *>(mapping->backing())->~FileMappingBacking();
  mapping->~FileMapping();
  detail::free_page(mapping);
  return nullptr;
}

FileMapping *
FileMapping::retain() {
  __atomic_fetch_add(&references_, 1U, __ATOMIC_RELAXED);
  return this;
}

int
FileMapping::enable_write() {
  LockedInode guard(node_->owner(), *node_);
  if (writing_)
  {
    return 0;
  }
  if (node_->owner().read_only())
  {
    return -EROFS;
  }
  if ((node_->mode() & 06000) &&
      !node_->owner().attributes(node_, node_->uid(), node_->gid(),
                                 node_->mode() & ~06000U))
  {
    return -EIO;
  }
  for (unsigned i = 0; i < pages_; ++i)
  {
    ++static_cast<MappedPages::Entry *>(entries_[i])->writers;
  }
  writing_ = true;
  return 0;
}

int
FileMapping::synchronize() {
  LockedInode guard(node_->owner(), *node_);
  if (MappedPages::flush(node_) < 0)
  {
    return -EIO;
  }
  return node_->owner().synchronize(node_) < 0 ? -EIO : 0;
}

void
FileMapping::release() {
  if (__atomic_sub_fetch(&references_, 1U, __ATOMIC_ACQ_REL))
  {
    return;
  }
  auto *node = node_;
  {
    LockedInode guard(node->owner(), *node);
    if (writing_ && MappedPages::flush(node) < 0)
    {
      detail::report("mmap: final writeback failed");
    }
    for (unsigned i = 0; i < pages_; ++i)
    {
      MappedPages::drop(static_cast<MappedPages::Entry *>(entries_[i]), writing_);
    }
    MappedPages::cleanup(node);
  }
  node->owner().release(node);
  static_cast<FileMappingBacking *>(backing())->~FileMappingBacking();
  this->~FileMapping();
  detail::free_page(this);
}

unsigned long
FileMapping::length() const {
  auto size = __atomic_load_n(&node_->mapped_length_, __ATOMIC_ACQUIRE);
  if (size <= offset_)
  {
    return 0;
  }
  unsigned long bytes = size - offset_;
  return bytes > pages_ * 4096UL ? pages_ * 4096UL : bytes;
}

bool
FileMapping::page_available(unsigned page) const {
  return page < pages_ && static_cast<unsigned long>(offset_) + page * 4096UL <
                              __atomic_load_n(&node_->mapped_length_, __ATOMIC_ACQUIRE);
}

void *
FileMapping::page(unsigned index) const {
  return index < pages_ ? static_cast<MappedPages::Entry *>(entries_[index])->frame
                        : nullptr;
}

int
FileSystem::read_file(Inode *node, int user, unsigned long address, unsigned offset,
                      unsigned count) {
  if (!node->mapped_pages_)
  {
    return read_data(node, user, address, offset, count);
  }
  if (offset + count < offset)
  {
    return -EINVAL;
  }
  if (offset >= node->size_bytes())
  {
    return 0;
  }
  if (count > node->size_bytes() - offset)
  {
    count = node->size_bytes() - offset;
  }
  unsigned done = 0;
  while (done < count)
  {
    unsigned position = offset + done, take = 4096 - position % 4096;
    if (take > count - done)
    {
      take = count - done;
    }
    int received;
    if (auto *entry = MappedPages::find(node, position / 4096))
    {
      received = detail::copy_out(user, address + done,
                                  static_cast<char *>(entry->frame) + position % 4096,
                                  take) < 0
                     ? -1
                     : int(take);
    }
    else
    {
      received = read_data(node, user, address + done, position, take);
    }
    if (received < 0)
    {
      return done ? int(done) : received;
    }
    done += received;
    if (received != int(take))
    {
      break;
    }
  }
  return done;
}

int
FileSystem::write_file(Inode *node, int user, unsigned long address, unsigned offset,
                       unsigned count) {
  if (!node->mapped_pages_)
  {
    return write_data(node, user, address, offset, count);
  }
  if (offset + count < offset)
  {
    return -EINVAL;
  }
  if (!count)
  {
    return 0;
  }
  unsigned old_size = node->size_bytes();
  // Publish outstanding mapped stores first. A failed legacy partial write can
  // modify existing bytes without returning a positive count; reload its range.
  if (MappedPages::flush(node) < 0)
  {
    return -EIO;
  }
  int result = write_data(node, user, address, offset, count);
  auto end = static_cast<unsigned long>(offset) + count;
  for (auto *block = node->mapped_pages_; block; block = block->next)
  {
    for (auto &entry : block->entries)
    {
      unsigned long low = entry.index * 4096UL, high = low + 4096;
      unsigned refresh_begin =
          node->size_bytes() > old_size && old_size < offset ? old_size : offset;
      if (!entry.frame || low >= end || high <= refresh_begin)
      {
        continue;
      }
      if (low < refresh_begin)
      {
        low = refresh_begin;
      }
      if (high > end)
      {
        high = end;
      }
      if (high > node->size_bytes())
      {
        high = node->size_bytes();
      }
      if (low < high &&
          read_data(node, 0, reinterpret_cast<unsigned long>(entry.frame) + low % 4096,
                    low, high - low) != int(high - low))
      {
        return -EIO;
      }
    }
  }
  __atomic_store_n(&node->mapped_length_, node->size_bytes(), __ATOMIC_RELEASE);
  return result;
}

int
FileSystem::resize_file(Inode *node, unsigned size) {
  if (MappedPages::flush(node) < 0)
  {
    return -EIO;
  }
  unsigned old_size = node->size_bytes();
  int result = resize(node, size);
  if (result < 0)
  {
    return result;
  }
  for (auto *block = node->mapped_pages_; block; block = block->next)
  {
    for (auto &entry : block->entries)
    {
      unsigned long low = entry.index * 4096UL;
      if (!entry.frame)
      {
        continue;
      }
      unsigned long begin = size < old_size ? size : old_size;
      unsigned long end = size < old_size ? low + 4096 : size;
      if (begin < low)
      {
        begin = low;
      }
      if (end > low + 4096)
      {
        end = low + 4096;
      }
      if (begin < end)
      {
        detail::fill(static_cast<char *>(entry.frame) + begin - low, 0, end - begin);
      }
    }
  }
  __atomic_store_n(&node->mapped_length_, size, __ATOMIC_RELEASE);
  if (size < old_size && node->mapped_pages_)
  {
    detail::synchronize_file_mappings();
  }
  return result;
}

int
FileSystem::flush_mapped(Inode *node) {
  return MappedPages::flush(node);
}

int
FileSystem::sync_file(Inode *node) {
  if (flush_mapped(node) < 0)
  {
    return -EIO;
  }
  return synchronize(node);
}
} // namespace xv::fs
