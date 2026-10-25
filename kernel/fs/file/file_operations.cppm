module xv.fs:operations;

import :file;
import :pipe;

namespace xv::fs
{
class FileOperations
{
public:

  virtual void close(File &file) = 0;
  virtual int read(File &file, unsigned long address, int count) = 0;
  virtual int write(File &file, unsigned long address, int count) = 0;
  virtual int stat(File &file, unsigned long address) = 0;

protected:

  constexpr FileOperations() = default;
  ~FileOperations() = default;
};

class InodeOperations final : public FileOperations
{
public:

  constexpr InodeOperations() = default;
  void close(File &file) override;
  int read(File &file, unsigned long address, int count) override;
  int write(File &file, unsigned long address, int count) override;
  int stat(File &file, unsigned long address) override;
};

class DeviceOperations final : public FileOperations
{
public:

  constexpr DeviceOperations() = default;
  void close(File &file) override;
  int read(File &file, unsigned long address, int count) override;
  int write(File &file, unsigned long address, int count) override;
  int stat(File &file, unsigned long address) override;
};

class PipeOperations final : public FileOperations
{
public:

  constexpr PipeOperations() = default;
  void close(File &file) override;
  int read(File &file, unsigned long address, int count) override;
  int write(File &file, unsigned long address, int count) override;

  int stat(File &, unsigned long) override
  {
    return -1;
  }
};
} // namespace xv::fs
