module;
#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/io/block.h"
#include "kernel/io/fs_service.h"
#include "kernel/io/fs_resources.h"
#include "user/runtime/user.h"
#include "kernel/fs/ext3/volume.h"
module xv.userfs.block;

namespace userfs
{
struct BlockIo::ReadCache
{
  static constexpr unsigned BlockSize = 1024;
  struct Line
  {
    unsigned long block = 0, used = 0;
    bool valid = false;
    unsigned char bytes[BlockSize]{};
  } lines[32]{};
  unsigned long sequence = 0;
};

BlockIo::BlockIo(int descriptor, int resources) : descriptor_(descriptor), resources_(resources)
{
  if (ioctl(descriptor_, BLKGETSIZE64, reinterpret_cast<unsigned long>(&size_)) < 0)
  {
    long end = lseek(descriptor_, 0, SEEK_END);
    size_ = end > 0 ? end : 0;
    if (size_) lseek(descriptor_, 0, SEEK_SET);
  }
  // Metadata lookups often revisit the same inode/directory block. Each backend
  // owns its storage descriptor; all of its writes below invalidate overlaps.
  if (size_)
  {
    cache_ = static_cast<ReadCache *>(malloc(sizeof(ReadCache)));
    if (cache_) memset(cache_, 0, sizeof(ReadCache));
  }
}

BlockIo::~BlockIo() { if (cache_) free(cache_); }

bool BlockIo::read(xv::ext3::u64 offset, void *data, unsigned count)
{
  if (offset > size_ || count > size_ - offset) return false;
  if (!cache_)
    return lseek(descriptor_, offset, SEEK_SET) == static_cast<long>(offset) &&
           ::read(descriptor_, data, count) == static_cast<int>(count);
  auto *output = static_cast<unsigned char *>(data);
  while (count)
  {
    unsigned long block = offset / ReadCache::BlockSize;
    unsigned begin = offset % ReadCache::BlockSize;
    auto *selected = &cache_->lines[0];
    for (auto &line : cache_->lines)
    {
      if (line.valid && line.block == block) { selected = &line; break; }
      if ((!line.valid && selected->valid) ||
          (line.valid == selected->valid && line.used < selected->used)) selected = &line;
    }
    if (!selected->valid || selected->block != block)
    {
      selected->valid = false;
      unsigned long position = block * ReadCache::BlockSize;
      unsigned length = size_ - position < ReadCache::BlockSize ? size_ - position : ReadCache::BlockSize;
      if (lseek(descriptor_, position, SEEK_SET) != static_cast<long>(position) ||
          ::read(descriptor_, selected->bytes, length) != int(length)) return false;
      selected->block = block;
      selected->valid = true;
    }
    selected->used = ++cache_->sequence;
    unsigned take = ReadCache::BlockSize - begin;
    if (take > count) take = count;
    memmove(output, selected->bytes + begin, take);
    offset += take; output += take; count -= take;
  }
  return true;
}

void BlockIo::invalidate(xv::ext3::u64 offset, unsigned count)
{
  if (!cache_ || !count) return;
  unsigned long first = offset / ReadCache::BlockSize;
  unsigned long last = (offset + count - 1) / ReadCache::BlockSize;
  for (auto &line : cache_->lines)
    if (line.valid && line.block >= first && line.block <= last) line.valid = false;
}

bool BlockIo::write(xv::ext3::u64 offset, const void *data, unsigned count)
{
  if (offset > size_ || count > size_ - offset) return false;
  // Invalidate before IO too: a failed write may have copied a partial prefix.
  invalidate(offset, count);
  return lseek(descriptor_, offset, SEEK_SET) == static_cast<long>(offset) &&
         ::write(descriptor_, data, count) == static_cast<int>(count);
}

bool BlockIo::flush()
{
  return fsync(descriptor_) == 0;
}

xv::ext3::u32 BlockIo::now_seconds() const
{
  unsigned long seconds = 0;
  if (ioctl(descriptor_, FS_SERVICE_EPOCH_SECONDS,
            reinterpret_cast<unsigned long>(&seconds)) < 0)
  {
    xv::fsservice::Query request;
    request.operation = xv::fsservice::Operation::System;
    if (resources_ < 0 || ioctl(resources_, xv::fsservice::QueryIoctl,
          reinterpret_cast<unsigned long>(&request)) < 0 || request.result <= 0 ||
        request.count != sizeof(xv::fsservice::System)) return 0;
    xv::fsservice::System system;
    memmove(&system, request.data, sizeof(system));
    return system.epoch;
  }
  return seconds;
}

void *BlockIo::allocate_page()
{
  auto *storage = malloc(8192);
  if (!storage) return nullptr;
  auto address = (reinterpret_cast<unsigned long>(storage) + sizeof(void *) + 4095) & ~4095UL;
  auto **page = reinterpret_cast<void **>(address);
  page[-1] = storage;
  return page;
}

void BlockIo::free_page(void *page)
{
  if (page) free(static_cast<void **>(page)[-1]);
}
} // namespace userfs
