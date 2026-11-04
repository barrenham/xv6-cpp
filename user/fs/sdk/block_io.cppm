module;
#include "kernel/support/types.h"
#include "kernel/fs/userfs/protocol.h"
#include "user/fs/backends/ext3/volume.h"
export module xv.userfs.block;

export namespace userfs
{
// The descriptor is preopened by the supervisor; callbacks never resolve paths.
class BlockIo final : public xv::ext3::Io
{
public:

  explicit BlockIo(int descriptor, int resources = -1);
  ~BlockIo();
  BlockIo(const BlockIo &) = delete;
  BlockIo &operator=(const BlockIo &) = delete;
  bool read(xv::ext3::u64 offset, void *data, unsigned count) override;
  bool write(xv::ext3::u64 offset, const void *data, unsigned count) override;
  bool flush() override;
  xv::ext3::u64 size_bytes() const override { return size_; }
  xv::ext3::u32 now_seconds() const override;
  void *allocate_page() override;
  void free_page(void *page) override;

private:

  struct ReadCache;
  void invalidate(xv::ext3::u64 offset, unsigned count);

  int descriptor_, resources_;
  unsigned long size_ = 0;
  ReadCache *cache_ = nullptr;
};
} // namespace userfs
