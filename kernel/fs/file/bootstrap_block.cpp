module;
#include "kernel/support/placement_new.hpp"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "kernel/io/block.h"
#include "kernel/io/fs_service.h"
#include "kernel/fs/stat.h"
#include "kernel/support/param.h"
module xv.fs;
import :file;
import :platform;

namespace xv::fs
{
namespace
{
// A descriptor is the capability. Only the boot supervisor receives one; fork
// deliberately shares its offset and lock until it transfers ownership.
class BootDisk final : public xv::io::IoObject, public xv::io::DeviceControl
{
public:

  BootDisk(BlockCache &cache, xv::drivers::BlockDevice &disk, unsigned device)
      : cache_(cache), disk_(disk), device_(device)
  {
    lock_.init("boot block descriptor");
  }

  int read(xv::io::Buffer buffer, int count) override
  {
    return transfer(buffer, count, false);
  }

  int write(xv::io::Buffer buffer, int count) override
  {
    return transfer(buffer, count, true);
  }

  long seek(long delta, int origin) override
  {
    xv::sync::ScopedLock guard(lock_);
    if (origin < SEEK_SET || origin > SEEK_END) return -1;
    unsigned long capacity = disk_.capacity_sectors() * 512;
    long base = origin == SEEK_SET ? 0 : origin == SEEK_CUR ? offset_ : capacity;
    if (delta < -base || delta > long(capacity) - base) return -1;
    offset_ = base + delta;
    return offset_;
  }

  unsigned long position() const override
  {
    return __atomic_load_n(&offset_, __ATOMIC_RELAXED);
  }

  int sync() override
  {
    xv::sync::ScopedLock guard(lock_);
    cache_.flush();
    return 0;
  }

  int stat(xv::io::Metadata &value) override
  {
    value = {};
    value.type = T_BLOCK;
    value.size = disk_.capacity_sectors() * 512;
    value.mode = 0600;
    value.nlink = 1;
    return 0;
  }

  unsigned poll(xv::sync::PollTable *) override
  {
    return POLLIN | POLLOUT;
  }

  int ioctl(unsigned request, xv::io::Buffer argument) override
  {
    if (request == FS_SERVICE_EPOCH_SECONDS)
    {
      unsigned long seconds = xv::drivers::realtime_seconds();
      return detail::copy_out(true, argument.address, &seconds, sizeof(seconds));
    }
    if (request == BLKGETSIZE64)
    {
      unsigned long bytes = disk_.capacity_sectors() * 512;
      return detail::copy_out(true, argument.address, &bytes, sizeof(bytes));
    }
    int value;
    if (request == BLKSSZGET) value = 512;
    else if (request == BLKROGET) value = 0;
    else return -1;
    return detail::copy_out(true, argument.address, &value, sizeof(value));
  }

  xv::io::Capability *capability(xv::io::CapabilityId id) override
  {
    return id == xv::io::DeviceControl::Id
               ? static_cast<xv::io::DeviceControl *>(this) : nullptr;
  }

  void close() override
  {
    this->~BootDisk();
    detail::free_page(this);
  }

private:

  int transfer(xv::io::Buffer buffer, int count, bool writing)
  {
    if (count < 0) return -1;
    if (!count) return 0;
    xv::sync::ScopedLock guard(lock_);
    unsigned long capacity = disk_.capacity_sectors() * 512;
    if (offset_ >= capacity) return writing ? -1 : 0;
    if (unsigned(count) > capacity - offset_) count = capacity - offset_;
    unsigned done = 0;
    while (done < unsigned(count))
    {
      unsigned at = offset_ % BlockSize;
      unsigned take = BlockSize - at;
      if (take > unsigned(count) - done) take = count - done;
      // Copy the whole proposed modification before publishing any bytes.
      if (writing && detail::copy_in(scratch_, true, buffer.address + done, take) < 0)
        return done ? int(done) : -1;
      auto block = cache_.read(device_, offset_ / BlockSize);
      if (writing)
      {
        detail::move(block->bytes() + at, scratch_, take);
        cache_.write(*block);
      }
      else if (detail::copy_out(true, buffer.address + done,
                                block->bytes() + at, take) < 0)
        return done ? int(done) : -1;
      done += take;
      __atomic_store_n(&offset_, offset_ + take, __ATOMIC_RELAXED);
    }
    return done;
  }

  BlockCache &cache_;
  xv::drivers::BlockDevice &disk_;
  const unsigned device_;
  xv::sync::SleepLock lock_;
  unsigned long offset_ = 0;
  unsigned char scratch_[BlockSize]{};
};
} // namespace

File *create_boot_disk(unsigned index)
{
  // Additional disks will receive separate cache/device capabilities as their
  // backends migrate. No filesystem service can obtain this through a pathname.
  if (index) return nullptr;
  auto &disk = xv::drivers::block_device();
  auto sectors = disk.capacity_sectors();
  if (!sectors || sectors % 2 || sectors > 0x100000000UL * (BlockSize / 512))
    return nullptr;
  auto *file = files().allocate();
  if (!file) return nullptr;
  auto *page = detail::allocate_page();
  if (!page)
  {
    files().release(file);
    return nullptr;
  }
  static_assert(sizeof(BootDisk) <= 4096);
  auto *object = new (page) BootDisk(cache(), disk, ROOTDEV);
  files().bind_object(*file, *object);
  return file;
}
} // namespace xv::fs
