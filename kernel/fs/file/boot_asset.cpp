module;
#include "kernel/boot/asset_archive.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/stat.h"
module xv.fs;
import :file;
import :platform;
import xv.memory;

namespace xv::fs
{
namespace
{
class BootAsset final : public xv::io::IoObject, public xv::io::ExecutableData
{
public:
  explicit BootAsset(xv::bootdata::Archive::Asset asset) : asset_(asset)
  {
    lock_.init("immutable boot asset");
  }
  int read(xv::io::Buffer destination, int count) override
  {
    if (count < 0) return -1;
    xv::sync::ScopedLock guard(lock_);
    unsigned long take = count;
    if (take > asset_.size - offset_) take = asset_.size - offset_;
    if (detail::copy_out(true, destination.address, asset_.data + offset_, take) < 0) return -1;
    offset_ += take;
    return take;
  }
  int write(xv::io::Buffer, int) override { return -1; }
  unsigned poll(xv::sync::PollTable *) override { return POLLIN; }
  long seek(long delta, int origin) override
  {
    xv::sync::ScopedLock guard(lock_);
    long base = origin == SEEK_SET ? 0 : origin == SEEK_CUR ? offset_ : asset_.size;
    if (origin < SEEK_SET || origin > SEEK_END || delta < -base || delta > long(asset_.size) - base) return -1;
    offset_ = base + delta;
    return offset_;
  }
  unsigned long position() const override { return __atomic_load_n(&offset_, __ATOMIC_RELAXED); }
  int stat(xv::io::Metadata &out) override
  {
    out = {};
    out.type = T_FILE;
    out.nlink = 1;
    out.size = asset_.size;
    out.mode = asset_.flags & xv::bootdata::Executable ? 0755 : 0644;
    return 0;
  }
  xv::io::Capability *capability(xv::io::CapabilityId id) override
  {
    return id == xv::io::ExecutableData::Id && (asset_.flags & xv::bootdata::Executable)
               ? static_cast<xv::io::ExecutableData *>(this) : nullptr;
  }
  const void *bytes() const override { return asset_.data; }
  unsigned long length() const override { return asset_.size; }
  const char *name() const override { return asset_.name; }
  void close() override { this->~BootAsset(); detail::free_page(this); }
private:
  xv::bootdata::Archive::Asset asset_;
  xv::sync::SleepLock lock_;
  unsigned long offset_ = 0;
};
}

File *open_boot_asset(unsigned long address)
{
  xv::bootdata::Request request;
  if (detail::copy_in(&request, true, address, sizeof(request)) < 0 ||
      request.size != sizeof(request) || request.version != xv::bootdata::Version ||
      request.name[xv::bootdata::NameSize - 1]) return nullptr;
  auto image = xv::memory::boot_image();
  xv::bootdata::Archive archive(image.begin().pointer(), image.size());
  xv::bootdata::Archive::Asset asset;
  if (!archive.get(request.index, request.name, asset)) return nullptr;
  request.flags = asset.flags;
  request.bytes = asset.size;
  detail::fill(request.name, 0, sizeof(request.name));
  for (unsigned i = 0; asset.name[i]; ++i) request.name[i] = asset.name[i];
  if (detail::copy_out(true, address, &request, sizeof(request)) < 0) return nullptr;
  auto *file = files().allocate();
  if (!file) return nullptr;
  auto *page = detail::allocate_page();
  if (!page) { files().release(file); return nullptr; }
  static_assert(sizeof(BootAsset) <= 4096);
  files().bind_object(*file, *new (page) BootAsset(asset));
  return file;
}
}
