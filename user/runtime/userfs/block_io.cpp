#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/io/block.h"
#include "kernel/io/fs_service.h"
#include "user/runtime/user.h"
#include "block_io.h"

namespace userfs
{
BlockIo::BlockIo(int descriptor) : descriptor_(descriptor)
{
  if (ioctl(descriptor_, BLKGETSIZE64, reinterpret_cast<unsigned long>(&size_)) < 0)
    size_ = 0;
}

bool BlockIo::read(xv::ext3::u64 offset, void *data, unsigned count)
{
  return offset <= size_ && count <= size_ - offset &&
         lseek(descriptor_, offset, SEEK_SET) == static_cast<long>(offset) &&
         ::read(descriptor_, data, count) == static_cast<int>(count);
}

bool BlockIo::write(xv::ext3::u64 offset, const void *data, unsigned count)
{
  return offset <= size_ && count <= size_ - offset &&
         lseek(descriptor_, offset, SEEK_SET) == static_cast<long>(offset) &&
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
            reinterpret_cast<unsigned long>(&seconds)) < 0) return 0;
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
