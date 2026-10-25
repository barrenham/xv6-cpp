module xv.fs:pipe;

import :file;

namespace xv::fs
{
class Pipe final
{
public:

  constexpr Pipe() = default;
  void initialize();
  void close(bool writable);
  int read(unsigned long address, int count);
  int write(unsigned long address, int count);

private:

  static constexpr unsigned Capacity = 512;
  xv::sync::SpinLock lock_;
  char data_[Capacity]{};
  unsigned read_ = 0;
  unsigned written_ = 0;
  bool reader_open_ = true;
  bool writer_open_ = true;
};
} // namespace xv::fs
