module;
#include "kernel/support/param.h"

module xv.fs:lru;
import :cache;
import xv.drivers;

namespace xv::fs
{
class LruBlockCache final : public BlockCache
{
public:

  constexpr explicit LruBlockCache(xv::drivers::BlockDevice *device = nullptr)
      : device_(device)
  {
  }

  const char *name() const override
  {
    return "hashed LRU block cache";
  }

  void attach(xv::drivers::BlockDevice &device)
  {
    device_ = &device;
  }

  void initialize() override;
  CacheStatistics statistics() const override;

  void flush() override
  {
    device_->flush();
  }

  BlockHandle read(unsigned device, unsigned block) override;
  void write(Buffer &buffer) override;
  void release(Buffer &buffer) override;
  void pin(Buffer &buffer) override;
  void unpin(Buffer &buffer) override;

private:

  struct GetFlow;
  struct ReleaseFlow;

  static void increment(unsigned long &value, unsigned long amount = 1)
  {
    __atomic_fetch_add(&value, amount, __ATOMIC_RELAXED);
  }

  void acquire(xv::sync::SpinLock &lock, CacheStatistics &statistics);
  Buffer *get(unsigned device, unsigned block);
  static constexpr unsigned BucketCount = 31;

  static unsigned index(unsigned device, unsigned block)
  {
    // Include both halves of the cache identity, including device zero.
    return (block * 2654435761U ^ device * 2246822519U) % BucketCount;
  }

  struct alignas(64) Bucket
  {
    xv::sync::SpinLock lock;
    Buffer *head = nullptr;
    CacheStatistics statistics;
  };

  Bucket buckets_[BucketCount]{};
  xv::sync::SpinLock eviction_lock_;
  CacheStatistics eviction_statistics_;

  // Replacement snapshots are private to eviction_lock_. A candidate is
  // revalidated under its current bucket lock before its identity is changed.
  struct Candidate
  {
    Buffer *buffer = nullptr;
    unsigned long stamp = 0;
  };

  Candidate candidates_[NBUF]{};
  unsigned candidate_count_ = 0;
  xv::drivers::BlockDevice *device_;
  Buffer buffers_[NBUF]{};
};
} // namespace xv::fs
