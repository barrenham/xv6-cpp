module;
#include "kernel/support/param.h"

export module xv.fs:file;
export import :filesystem;
export import xv.drivers;
export import xv.io;

export namespace xv::fs
{
class Pipe;
enum class FileKind : unsigned { Unknown, Node, Pipe, Terminal, Socket };
struct FileInfo
{
  unsigned long identity = 0;
  unsigned flags = 0;
  FileKind kind = FileKind::Unknown;
  short type = 0;
  unsigned device = 0;
  unsigned inode = 0;
};

class File final
{
public:

  constexpr File() = default;
  bool nonblocking() const;
  File(const File &) = delete;
  File &operator=(const File &) = delete;

private:

  friend class OpenFileTable;
  static constexpr unsigned BackendBytes = 64;
  int ref = 0;
  unsigned status_flags_ = 0;
  bool ready_ = false;
  FileInfo identity_;
  bool readable = false;
  bool writable = false;
  xv::io::IoObject *backend_ = nullptr;
  alignas(16) unsigned char backend_storage_[BackendBytes]{};
};

class OpenFileTable final
{
public:

  constexpr OpenFileTable() = default;
  void initialize();
  File *allocate();
  File *retain(File *file);
  void release(File *file);
  bool bind_inode(File &file, Inode &node, bool readable, bool writable, bool nonblocking = false);
  void bind_pipe(File &file, Pipe &pipe, bool writable);
  void bind_endpoint(File &file, xv::drivers::CharacterDevice &endpoint);
  void bind_object(File &file, xv::io::IoObject &object, bool nonblocking = false);
  xv::io::IoObject *object(File *file) const;
  bool describe(File *file, FileInfo &result) const;
  int read(File *file, unsigned long address, int count);
  int write(File *file, unsigned long address, int count);
  int stat(File *file, unsigned long address);
  int ioctl(File *file, unsigned request, unsigned long argument);
  int fcntl(File *file, int command, unsigned flags);
  unsigned poll(File *file, xv::sync::PollTable *table = nullptr);

private:

  void configure(File &file, xv::io::IoObject &object, bool readable, bool writable, bool nonblocking, FileKind kind, const Inode *node = nullptr);
  xv::sync::SpinLock lock_;
  File files_[NFILE]{};
  unsigned long next_identity_ = 1;
};

OpenFileTable &files();

class DeviceRegistry final
{
public:

  constexpr DeviceRegistry() = default;
  struct Entry
  {
    char name[14]{};
    unsigned major = 0;
    unsigned minor = 0;
    xv::drivers::CharacterDevice *device = nullptr;
  };
  void attach(unsigned major, xv::drivers::CharacterDevice &device);
  void attach(const char *name, unsigned major, unsigned minor, xv::drivers::CharacterDevice &device);
  xv::drivers::CharacterDevice *get(int major, int minor = 0) const;
  unsigned count() const { return count_; }
  const Entry &entry(unsigned index) const { return entries_[index]; }
  void seal() { sealed_ = true; }

private:

  Entry entries_[NDEV]{};
  unsigned count_ = 0;
  bool sealed_ = false;
};

DeviceRegistry &devices();
xv::drivers::CharacterDevice *open_registered_device(Inode *node, bool nonblocking);
bool create_pipe(File **read_end, File **write_end);
} // namespace xv::fs
