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
constinit xv::sync::SpinLock read_context_lock;
ReadLockedInode *read_contexts = nullptr;

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
    return owner_.resident_page(index);
  }

  bool
  demand_paged() const override {
    return true;
  }

  void *
  load_page(unsigned index, int &error) override {
    return owner_.load_page(index, error);
  }

private:

  FileMapping &owner_;
};
} // namespace

ReadLockedInode::ReadLockedInode(FileSystem &fs, Inode &node) : fs_(fs), node_(node) {
  fs_.lock_read(&node_);
  link(*this);
}

ReadLockedInode::~ReadLockedInode() {
  unlink(*this);
  fs_.unlock_read(&node_);
}

void
ReadLockedInode::link(ReadLockedInode &guard) {
  xv::sync::ScopedLock lock(read_context_lock);
  guard.pid_ = detail::current_pid();
  guard.next_ = read_contexts;
  read_contexts = &guard;
}

void
ReadLockedInode::unlink(ReadLockedInode &guard) {
  xv::sync::ScopedLock lock(read_context_lock);
  auto **cursor = &read_contexts;
  while (*cursor && *cursor != &guard)
  {
    cursor = &(*cursor)->next_;
  }
  if (!*cursor)
  {
    detail::fail("file read context: missing guard");
  }
  *cursor = guard.next_;
}

bool
ReadLockedInode::held() {
  xv::sync::ScopedLock lock(read_context_lock);
  for (auto *guard = read_contexts; guard; guard = guard->next_)
  {
    if (guard->pid_ == detail::current_pid())
    {
      return true;
    }
  }
  return false;
}

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
    unsigned long used = 0;
    bool loading = false;
    int error = 0;
  };

  MappedPages *next = nullptr;
  Entry entries[100];

  static Entry *
  find_locked(Inode *node, unsigned index) {
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

  static bool
  contains(Inode *node, unsigned index) {
    xv::sync::ScopedLock guard(node->cache_state_);
    auto *entry = find_locked(node, index);
    return entry && !entry->loading && !entry->error;
  }

  static bool
  covered(Inode *node, unsigned offset, unsigned count) {
    xv::sync::ScopedLock guard(node->cache_state_);
    unsigned last = (static_cast<unsigned long>(offset) + count - 1) / 4096;
    for (unsigned index = offset / 4096; index <= last; ++index)
    {
      auto *entry = find_locked(node, index);
      if (!entry || entry->loading || entry->error)
      {
        return false;
      }
    }
    return true;
  }

  static bool
  mapped_writers(Inode *node, unsigned offset, unsigned count) {
    xv::sync::ScopedLock guard(node->cache_state_);
    auto end = static_cast<unsigned long>(offset) + count;
    for (auto *block = node->mapped_pages_; block; block = block->next)
    {
      for (auto &entry : block->entries)
      {
        if (entry.frame && entry.writers && entry.index * 4096UL < end &&
            entry.index * 4096UL + 4096 > offset)
        {
          return true;
        }
      }
    }
    return false;
  }

  static void
  cleanup(Inode *node) {
    xv::sync::ScopedLock guard(node->cache_state_);
    for (auto *block = node->mapped_pages_; block; block = block->next)
    {
      for (auto &entry : block->entries)
      {
        if (entry.frame && !entry.leases &&
            entry.index * 4096UL + 4096 > node->size_bytes())
        {
          auto low = entry.index * 4096UL;
          auto valid = node->size_bytes() > low ? node->size_bytes() - low : 0;
          detail::fill(static_cast<char *>(entry.frame) + valid, 0, 4096 - valid);
        }
      }
    }
    // Mapped pages are pinned. Keep only eight unleased pages per inode; these
    // clean, write-through pages can always be reclaimed without an FS RPC.
    for (;;)
    {
      unsigned idle = 0;
      Entry *oldest = nullptr;
      for (auto *block = node->mapped_pages_; block; block = block->next)
      {
        for (auto &entry : block->entries)
        {
          if (entry.frame && !entry.leases)
          {
            ++idle;
            if (!oldest || entry.used < oldest->used)
            {
              oldest = &entry;
            }
          }
        }
      }
      if (idle <= 8)
      {
        break;
      }
      detail::release_frame(oldest->frame);
      *oldest = {};
    }
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
  drop(Inode *node, Entry *entry, bool writing) {
    xv::sync::ScopedLock guard(node->cache_state_);
    if (writing)
    {
      --entry->writers;
    }
    if (!entry->leases)
    {
      detail::fail("file cache: lease underflow");
    }
    if (!--entry->leases && entry->error)
    {
      detail::release_frame(entry->frame);
      *entry = {};
    }
  }

  static void
  add_writer(Inode *node, Entry *entry) {
    xv::sync::ScopedLock guard(node->cache_state_);
    ++entry->writers;
  }

  static Entry *
  acquire(Inode *node, unsigned index, int &error, unsigned long seed = 0,
          unsigned seed_bytes = 0, int seed_user = 0, bool supplied = false) {
    node->cache_state_.acquire();
    if (auto *found = find_locked(node, index))
    {
      ++found->leases; // Pin metadata before sleeping; cleanup cannot free it.
      while (found->loading)
      {
        detail::prepare_sleep(found);
        node->cache_state_.release();
        detail::sleep();
        node->cache_state_.acquire();
      }
      error = found->error;
      found->used = detail::cache_ticks();
      node->cache_state_.release();
      if (error)
      {
        drop(node, found, false);
        return nullptr;
      }
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
        node->cache_state_.release();
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
      node->cache_state_.release();
      error = -ENOMEM;
      return nullptr;
    }
    *slot = {frame, index, 1, 0, detail::cache_ticks(), true, 0};
    node->cache_state_.release();
    detail::fill(frame, 0, 4096);
    unsigned offset = index * 4096;
    unsigned bytes = offset < node->size_bytes() ? node->size_bytes() - offset : 0;
    if (bytes > 4096)
    {
      bytes = 4096;
    }
    // No cache spinlock is held across an RPC. Different pages may load in
    // parallel; duplicate misses sleep on the single in-flight page.
    error =
        supplied
            ? (detail::copy_in(frame, seed_user, seed, seed_bytes) < 0 ? -EFAULT : 0)
        : bytes &&
                node->owner().read_data(node, 0, reinterpret_cast<unsigned long>(frame),
                                        offset, bytes) != int(bytes)
            ? -EIO
            : 0;
    node->cache_state_.acquire();
    slot->error = error;
    slot->loading = false;
    detail::wakeup(slot);
    node->cache_state_.release();
    if (error)
    {
      drop(node, slot, false);
      return nullptr;
    }
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
      mapping->pages_ = pages;
      node->owner().retain(node);
      error = 0;
      return mapping;
    }
    else
    {
      error = -EIO;
    }
    for (unsigned i = 0; i < mapping->pages_; ++i)
    {
      MappedPages::drop(node, static_cast<MappedPages::Entry *>(mapping->entries_[i]),
                        false);
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
    if (entries_[i])
    {
      ++static_cast<MappedPages::Entry *>(entries_[i])->writers;
    }
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
      if (entries_[i])
      {
        MappedPages::drop(node, static_cast<MappedPages::Entry *>(entries_[i]),
                          writing_);
      }
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
FileMapping::resident_page(unsigned index) const {
  auto *entry =
      index < pages_ ? __atomic_load_n(&entries_[index], __ATOMIC_ACQUIRE) : nullptr;
  return entry ? static_cast<MappedPages::Entry *>(entry)->frame : nullptr;
}

void *
FileMapping::page(unsigned index) {
  int error = 0;
  return load_page(index, error);
}

void *
FileMapping::load_page(unsigned index, int &error) {
  error = 0;
  if (index >= pages_)
  {
    error = -EINVAL;
    return nullptr;
  }
  if (auto *entry = __atomic_load_n(&entries_[index], __ATOMIC_ACQUIRE))
  {
    return static_cast<MappedPages::Entry *>(entry)->frame;
  }
  // User buffers are prefaulted before file locks. Never recursively enter a
  // provider from a copy while a VFS transaction/read guard is already held.
  if (ReadLockedInode::held() || !detail::mapping_fault_safe())
  {
    error = -EFAULT;
    return nullptr;
  }
  ReadLockedInode guard(node_->owner(), *node_);
  if (!__atomic_load_n(&entries_[index], __ATOMIC_ACQUIRE))
  {
    auto *entry = MappedPages::acquire(node_, offset_ / 4096 + index, error);
    if (!entry)
    {
      MappedPages::cleanup(node_);
      return nullptr;
    }
    if (writing_)
    {
      MappedPages::add_writer(node_, entry);
    }
    void *expected = nullptr;
    if (!__atomic_compare_exchange_n(&entries_[index], &expected,
                                     static_cast<void *>(entry), false,
                                     __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
    {
      MappedPages::drop(node_, entry, writing_);
    }
  }
  return resident_page(index);
}

int
FileSystem::read_file(Inode *node, int user, unsigned long address, unsigned offset,
                      unsigned count) {
  if (node->kind() != T_FILE || (!mappable() && !node->mapped_pages_))
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
  if (!count)
  {
    return 0;
  }
  if (count >= 4096 && mappable() && !MappedPages::covered(node, offset, count) &&
      !MappedPages::mapped_writers(node, offset, count))
  {
    // Preserve bulk transport for cold sequential IO. Seed the last eight
    // complete pages from the delivered bytes instead of rereading every page.
    int result = read_data(node, user, address, offset, count);
    if (result <= 0)
    {
      return result;
    }
    auto end = static_cast<unsigned long>(offset) + result;
    unsigned first = offset / 4096, last = (end - 1) / 4096;
    for (unsigned index = first; index <= last; ++index)
    {
      if (!MappedPages::contains(node, index))
      {
        continue;
      }
      int error = 0;
      auto *entry = MappedPages::acquire(node, index, error);
      if (!entry)
      {
        continue;
      }
      auto low = index * 4096UL > offset ? index * 4096UL : offset;
      auto high = (index + 1UL) * 4096 < end ? (index + 1UL) * 4096 : end;
      bool good = detail::copy_out(user, address + low - offset,
                                   static_cast<char *>(entry->frame) + low % 4096,
                                   high - low) >= 0;
      MappedPages::drop(node, entry, false);
      if (!good)
      {
        return -1;
      }
      record_read_hit();
    }
    unsigned start = last > first + 7 ? last - 7 : first;
    for (unsigned index = start; index <= last; ++index)
    {
      auto low = index * 4096UL;
      unsigned bytes = node->size_bytes() - low;
      if (bytes > 4096)
      {
        bytes = 4096;
      }
      if (low < offset || low + bytes > end)
      {
        continue;
      }
      int error = 0;
      if (auto *entry = MappedPages::acquire(node, index, error, address + low - offset,
                                             bytes, user, true))
      {
        MappedPages::drop(node, entry, false);
      }
    }
    MappedPages::cleanup(node);
    return result;
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
    bool hit = MappedPages::contains(node, position / 4096);
    int error = 0;
    if (auto *entry = (hit || mappable())
                          ? MappedPages::acquire(node, position / 4096, error)
                          : nullptr)
    {
      if (hit)
      {
        record_read_hit();
      }
      received = detail::copy_out(user, address + done,
                                  static_cast<char *>(entry->frame) + position % 4096,
                                  take) < 0
                     ? -1
                     : int(take);
      MappedPages::drop(node, entry, false);
      MappedPages::cleanup(node);
    }
    else
    {
      received = read_data(node, user, address + done, position, take);
      MappedPages::cleanup(node);
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
FileSystem::cache_written(Inode *node, unsigned old_size, unsigned offset,
                          unsigned count, CacheCopy copy, void *context) {
  if (!count)
  {
    return 0;
  }
  auto end = static_cast<unsigned long>(offset) + count;
  for (auto *block = node->mapped_pages_; block; block = block->next)
  {
    for (auto &entry : block->entries)
    {
      if (!entry.frame)
      {
        continue;
      }
      auto start = entry.index * 4096UL, limit = start + 4096;
      if (offset > old_size)
      {
        auto low = start > old_size ? start : old_size;
        auto high = limit < offset ? limit : offset;
        if (low < high)
        {
          detail::fill(static_cast<char *>(entry.frame) + low % 4096, 0, high - low);
        }
      }
      auto low = start > offset ? start : offset;
      auto high = limit < end ? limit : end;
      if (low < high)
      {
        entry.used = detail::cache_ticks();
      }
      if (low < high && copy(context, static_cast<char *>(entry.frame) + low % 4096,
                             low - offset, high - low) < 0)
      {
        entry.error = -EIO;
        return -EIO;
      }
    }
  }
  return 0;
}

int
FileSystem::write_file(Inode *node, int user, unsigned long address, unsigned offset,
                       unsigned count) {
  if (!node->mapped_pages_)
  {
    int result = write_cached_data(node, user, address, offset, count);
    __atomic_store_n(&node->mapped_length_, node->size_bytes(), __ATOMIC_RELEASE);
    return result;
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
  int result = write_cached_data(node, user, address, offset, count);
  if (result >= 0 && publishes_cached_writes(node))
  {
    __atomic_store_n(&node->mapped_length_, node->size_bytes(), __ATOMIC_RELEASE);
    return result;
  }
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
      if (low >= high)
      {
        continue;
      }
      if (read_data(node, 0, reinterpret_cast<unsigned long>(entry.frame) + low % 4096,
                    low, high - low) != int(high - low))
      {
        entry.error = -EIO;
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

void
FileSystem::discard_file_cache(Inode *node) {
  while (auto *block = node->mapped_pages_)
  {
    node->mapped_pages_ = block->next;
    for (auto &entry : block->entries)
    {
      if (entry.leases)
      {
        detail::fail("file cache: discard pinned page");
      }
      if (entry.frame)
      {
        detail::release_frame(entry.frame);
      }
    }
    block->~MappedPages();
    detail::free_page(block);
  }
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
