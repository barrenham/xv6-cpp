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
    return "LRU block cache";
  }

  void attach(xv::drivers::BlockDevice &device)
  {
    device_ = &device;
  }

  void initialize() override;

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

  Buffer *get(unsigned device, unsigned block);
  xv::drivers::BlockDevice *device_;
  xv::sync::SpinLock lock_;
  Buffer buffers_[NBUF]{};
  Buffer head_{};
};
} // namespace xv::fs
