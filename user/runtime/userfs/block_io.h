#pragma once
#include "kernel/fs/ext3/volume.h"

namespace userfs
{
// The descriptor is preopened by the supervisor; callbacks never resolve paths.
class BlockIo final : public xv::ext3::Io
{
public:

  explicit BlockIo(int descriptor);
  bool read(xv::ext3::u64 offset, void *data, unsigned count) override;
  bool write(xv::ext3::u64 offset, const void *data, unsigned count) override;
  bool flush() override;
  xv::ext3::u64 size_bytes() const override { return size_; }
  xv::ext3::u32 now_seconds() const override;
  void *allocate_page() override;
  void free_page(void *page) override;

private:

  int descriptor_;
  unsigned long size_ = 0;
};
} // namespace userfs
