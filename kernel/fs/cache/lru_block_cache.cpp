module;
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/fs/stat.h"

module xv.fs;
import :cache_flow;
import :platform;
import :lru;

namespace xv::fs
{
void LruBlockCache::initialize()
{
  if (!device_)
  {
    detail::fail("cache: missing block device");
  }
  Buffer *b;

  lock_.init("bcache");

  // Create linked list of buffers
  head_.prev = &head_;
  head_.next = &head_;
  for (b = buffers_; b < buffers_ + NBUF; b++)
  {
    b->next = head_.next;
    b->prev = &head_;
    b->lock.init("buffer");
    head_.next->prev = b;
    head_.next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
Buffer *LruBlockCache::get(unsigned dev, unsigned blockno)
{
  lock_.acquire();
  GetFlow::Context context{*this, dev, blockno};
  (void)GetFlow::Graph::run<0>(context);
  return context.result;
}

// Return a locked buf with the contents of the indicated block.
BlockHandle LruBlockCache::read(unsigned dev, unsigned blockno)
{
  Buffer *b;

  b = get(dev, blockno);
  if (!b->valid)
  {
    device_->transfer(xv::drivers::BlockOperation::Read,
                      static_cast<unsigned long>(b->blockno) *
                          (BSIZE / xv::drivers::BlockDevice::SectorSize),
                      b->data, BSIZE);
    b->valid = 1;
  }
  return BlockHandle(*this, *b);
}

// Write b's contents to disk.  Must be locked.
// Only the log calls bwrite.
void LruBlockCache::write(Buffer &buffer)
{
  auto *b = &buffer;
  if (!b->lock.holding())
  {
    detail::fail("bwrite");
  }
  device_->transfer(xv::drivers::BlockOperation::Write,
                    static_cast<unsigned long>(b->blockno) *
                        (BSIZE / xv::drivers::BlockDevice::SectorSize),
                    b->data, BSIZE);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void LruBlockCache::release(Buffer &buffer)
{
  ReleaseFlow::Context context{*this, buffer};
  (void)ReleaseFlow::Graph::run<0>(context);
}

void LruBlockCache::pin(Buffer &buffer)
{
  auto *b = &buffer;
  lock_.acquire();
  b->refcnt++;
  lock_.release();
}

void LruBlockCache::unpin(Buffer &buffer)
{
  auto *b = &buffer;
  lock_.acquire();
  b->refcnt--;
  lock_.release();
}

} // namespace xv::fs
