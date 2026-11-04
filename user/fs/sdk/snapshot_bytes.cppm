module;
#include "kernel/fs/userfs/protocol.h"
#include "kernel/support/types.h"
#include "user/runtime/user.h"
export module xv.userfs.snapshot;

export namespace userfs
{
class SnapshotBytes final
{
public:
  ~SnapshotBytes() { clear(); }
  SnapshotBytes() = default;
  SnapshotBytes(const SnapshotBytes &) = delete;
  SnapshotBytes &operator=(const SnapshotBytes &) = delete;
  void clear()
  {
    if (data_) free(data_);
    data_ = nullptr; size_ = capacity_ = 0; failed_ = false;
  }
  bool append(const void *data, unsigned count)
  {
    if (failed_ || count > ~0U - size_) { failed_ = true; return false; }
    if (!count) return true;
    unsigned needed = size_ + count;
    if (needed > capacity_)
    {
      unsigned capacity = capacity_ ? capacity_ : 512;
      while (capacity < needed)
      {
        if (capacity > ~0U / 2) { capacity = needed; break; }
        capacity *= 2;
      }
      auto *next = static_cast<unsigned char *>(malloc(capacity));
      if (!next) { failed_ = true; return false; }
      if (size_) memmove(next, data_, size_);
      if (data_) free(data_);
      data_ = next; capacity_ = capacity;
    }
    memmove(data_ + size_, data, count); size_ += count;
    return true;
  }
  int read(unsigned long offset, void *data, unsigned count) const
  {
    if (offset >= size_) return 0;
    if (count > size_ - offset) count = size_ - offset;
    memmove(data, data_ + offset, count);
    return count;
  }
  unsigned size() const { return size_; }
  bool good() const { return !failed_; }
  void text(const char *value) { append(value, strlen(value)); }
  void number(unsigned long value, unsigned radix = 10)
  {
    char digits[65]; unsigned count = 0;
    do { digits[count++] = "0123456789abcdef"[value % radix]; value /= radix; } while (value);
    while (count) append(digits + --count, 1);
  }
  void field(const char *name, unsigned long value, const char *suffix = "\n")
  { text(name); number(value); text(suffix); }
  void hex(const char *name, unsigned long value)
  { text(name); text("0x"); number(value, 16); text("\n"); }
private:
  unsigned char *data_ = nullptr;
  unsigned size_ = 0, capacity_ = 0;
  bool failed_ = false;
};
} // namespace userfs
