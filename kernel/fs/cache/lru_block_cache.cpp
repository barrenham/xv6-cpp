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
  eviction_lock_.init("cache eviction");
  eviction_statistics_ = {};
  candidate_count_ = 0;
  for (auto &bucket : buckets_)
  {
    bucket.lock.init("cache bucket");
    bucket.head = nullptr;
    bucket.statistics = {};
  }
  for (unsigned i = 0; i < NBUF; ++i)
  {
    auto &buffer = buffers_[i];
    buffer.assigned_ = false;
    buffer.valid = 0;
    buffer.dev = buffer.blockno = 0;
    __atomic_store_n(&buffer.refcnt, 0U, __ATOMIC_RELAXED);
    __atomic_store_n(&buffer.last_use_, 0UL, __ATOMIC_RELAXED);
    buffer.bucket_ = i % BucketCount;
    buffer.next = nullptr;
    buffer.lock.init("buffer");
  }
}

Buffer *LruBlockCache::get(unsigned device, unsigned block)
{
  GetFlow::Context context{*this, device, block};
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
    increment(buckets_[b->bucket_].statistics.reads);
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
  increment(buckets_[b->bucket_].statistics.writes);
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
  // The caller owns an existing handle/pin, so identity cannot be replaced.
  auto &bucket = buckets_[buffer.bucket_];
  acquire(bucket.lock, bucket.statistics);
  auto refs = __atomic_load_n(&buffer.refcnt, __ATOMIC_RELAXED);
  if (!refs)
  {
    detail::fail("cache: pin without reference");
  }
  __atomic_store_n(&buffer.refcnt, refs + 1, __ATOMIC_RELAXED);
  bucket.lock.release();
}

void LruBlockCache::unpin(Buffer &buffer)
{
  auto &bucket = buckets_[buffer.bucket_];
  acquire(bucket.lock, bucket.statistics);
  auto refs = __atomic_load_n(&buffer.refcnt, __ATOMIC_RELAXED);
  if (!refs)
  {
    detail::fail("cache: unpin without reference");
  }
  if (refs == 1)
  {
    __atomic_store_n(&buffer.last_use_, detail::cache_ticks(), __ATOMIC_RELAXED);
  }
  __atomic_store_n(&buffer.refcnt, refs - 1, __ATOMIC_RELAXED);
  bucket.lock.release();
}

} // namespace xv::fs

namespace xv::fs
{
void LruBlockCache::acquire(xv::sync::SpinLock &lock, CacheStatistics &statistics)
{
  // Avoid reading the clock on every uncontended cache operation. Only a
  // failed exchange starts the sampled wait interval; that failure also counts.
  if (lock.try_acquire())
  {
    increment(statistics.locks);
    return;
  }
  const auto start = detail::cache_ticks();
  const auto spins = lock.acquire_spins();
  increment(statistics.locks);
  increment(statistics.contentions);
  increment(statistics.spins, spins + 1);
  increment(statistics.wait_ticks, detail::cache_ticks() - start);
}

CacheStatistics LruBlockCache::statistics() const
{
  CacheStatistics result;
  auto accumulate = [&](const CacheStatistics &statistics)
  {
#define ADD(field) result.field += __atomic_load_n(&statistics.field, __ATOMIC_RELAXED)
    ADD(hits);
    ADD(misses);
    ADD(evictions);
    ADD(lookup_steps);
    ADD(reads);
    ADD(writes);
    ADD(eviction_steps);
    ADD(locks);
    ADD(contentions);
    ADD(spins);
    ADD(wait_ticks);
#undef ADD
  };
  accumulate(eviction_statistics_);
  for (const auto &bucket : buckets_)
  {
    accumulate(bucket.statistics);
  }
  return result;
}
} // namespace xv::fs
