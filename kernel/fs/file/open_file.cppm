module;
#include "kernel/support/param.h"

export module xv.fs:file;
export import :filesystem;
export import xv.drivers;

export namespace xv::fs
{
class Pipe;

class File final
{
public:

  constexpr File() = default;
  File(const File &) = delete;
  File &operator=(const File &) = delete;

private:

  friend class OpenFileTable;
  friend class InodeOperations;
  friend class DeviceOperations;
  friend class PipeOperations;
  enum class Kind
  {
    None,
    Pipe,
    Inode,
    Device
  };
  Kind type = Kind::None;
  int ref = 0;
  bool readable = false;
  bool writable = false;
  Pipe *pipe = nullptr;
  Inode *node = nullptr;
  unsigned offset = 0;
  short major = 0;
};

class OpenFileTable final
{
public:

  constexpr OpenFileTable() = default;
  void initialize();
  File *allocate();
  File *retain(File *file);
  void release(File *file);
  void bind_inode(File &file, Inode &node, bool readable, bool writable);
  void bind_pipe(File &file, Pipe &pipe, bool writable);
  int read(File *file, unsigned long address, int count);
  int write(File *file, unsigned long address, int count);
  int stat(File *file, unsigned long address);

private:

  xv::sync::SpinLock lock_;
  File files_[NFILE]{};
};

OpenFileTable &files();

class DeviceRegistry final
{
public:

  constexpr DeviceRegistry() = default;
  void attach(unsigned major, xv::drivers::CharacterDevice &device);
  xv::drivers::CharacterDevice *get(int major) const;

private:

  xv::drivers::CharacterDevice *devices_[NDEV]{};
};

DeviceRegistry &devices();
bool create_pipe(File **read_end, File **write_end);
} // namespace xv::fs
