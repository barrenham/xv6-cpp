#pragma once
#include "kernel/support/types.h"
#include "user/runtime/user.h"

namespace xv::user {
// Owns exactly one descriptor; ownership can move, but cannot be copied.
class File final {
public:
  enum class Access { read_only = 0, write_only = 1, read_write = 2 };
  explicit File(int descriptor = -1) : descriptor_(descriptor) {}
  ~File() { reset(); }
  File(const File &) = delete;
  File &operator=(const File &) = delete;
  File(File &&other) : descriptor_(other.release()) {}
  File &operator=(File &&other) {
    if (this != &other) reset(other.release());
    return *this;
  }
  static File open(const char *path, Access mode) {
    return File(::open(path, static_cast<int>(mode)));
  }
  bool valid() const { return descriptor_ >= 0; }
  int read(void *buffer, int size) const { return ::read(descriptor_, buffer, size); }
  int write(const void *buffer, int size) const { return ::write(descriptor_, buffer, size); }
  int release() { int fd = descriptor_; descriptor_ = -1; return fd; }
  void reset(int descriptor = -1) {
    if (descriptor_ == descriptor) return;
    if (valid()) ::close(descriptor_);
    descriptor_ = descriptor;
  }
private:
  int descriptor_;
};
}
