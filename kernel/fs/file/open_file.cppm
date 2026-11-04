module;
#include "kernel/support/param.h"
#include "kernel/fs/userfs/protocol.h"

export module xv.fs:file;
export import :filesystem;
import :cache;
export import xv.drivers;
export import xv.io;

export namespace xv::fs
{
class Pipe;
enum class FileKind : unsigned
{
  Unknown,
  Node,
  Pipe,
  Terminal,
  Socket
};

struct FileInfo
{
  unsigned long identity = 0;
  unsigned flags = 0;
  FileKind kind = FileKind::Unknown;
  short type = 0;
  unsigned device = 0;
  unsigned inode = 0;
  unsigned long position = 0;
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
  bool bind_inode(File &file, Inode &node, bool readable, bool writable,
                  bool nonblocking = false, bool append = false);
  void bind_pipe(File &file, Pipe &pipe, bool writable);
  void bind_endpoint(File &file, xv::drivers::CharacterDevice &endpoint);
  void bind_object(File &file, xv::io::IoObject &object, bool nonblocking = false);
  xv::io::IoObject *object(File *file) const;
  bool describe(File *file, FileInfo &result) const;
  int read(File *file, unsigned long address, int count);
  int write(File *file, unsigned long address, int count);
  int stat(File *file, unsigned long address);
  int getdents64(File *file, unsigned long address, int count);
  long seek(File *file, long offset, int origin);
  int resize(File *file, unsigned size);
  int sync(File *file);
  int ioctl(File *file, unsigned request, unsigned long argument);
  int fcntl(File *file, int command, unsigned flags);
  unsigned poll(File *file, xv::sync::PollTable *table = nullptr);

private:

  struct ReleaseFlow;
  struct BindInodeFlow;

  void configure(File &file, xv::io::IoObject &object, bool readable, bool writable,
                 bool nonblocking, FileKind kind, const Inode *node = nullptr);
  xv::sync::SpinLock lock_;
  File files_[NFILE]{};
  unsigned long next_identity_ = 1;
};

OpenFileTable &files();

class DeviceRegistry final
{
public:

  constexpr DeviceRegistry() = default;
  static constexpr unsigned Capacity = 32;

  struct Entry
  {
    char name[14]{};
    unsigned major = 0;
    unsigned minor = 0;
    xv::drivers::CharacterDevice *device = nullptr;
    xv::drivers::BlockDevice *block = nullptr;
    bool read_only = true;
    unsigned mode = 0666;
  };

  void attach(unsigned major, xv::drivers::CharacterDevice &device);
  void attach(const char *name, unsigned major, unsigned minor,
              xv::drivers::CharacterDevice &device, unsigned mode = 0666);
  xv::drivers::CharacterDevice *get(int major, int minor = 0) const;
  void attach_block(const char *name, unsigned major, unsigned minor,
                    xv::drivers::BlockDevice &device, bool read_only = true);
  bool attach_dynamic_block(const char *name, unsigned major, unsigned minor,
                            xv::drivers::BlockDevice &device);
  bool attach_dynamic_character(const char *name, unsigned major, unsigned minor,
                                unsigned mode, xv::drivers::CharacterDevice &device);
  const Entry *block(int major, int minor) const;

  unsigned
  count() const {
    return __atomic_load_n(&count_, __ATOMIC_ACQUIRE);
  }

  const Entry &
  entry(unsigned index) const {
    return entries_[index];
  }

  void
  seal() {
    sealed_ = true;
  }

private:

  struct AttachFlow;

  Entry entries_[Capacity]{};
  unsigned count_ = 0;
  bool sealed_ = false;
};

DeviceRegistry &devices();
void initialize_device_registry();
File *create_fs_resources();
File *open_boot_asset(unsigned long request_address);
xv::drivers::CharacterDevice *open_registered_device(Inode *node, bool nonblocking);
xv::drivers::BlockDevice *open_registered_block(Inode *node, bool writable);
bool create_pipe(File **read_end, File **write_end);
void initialize_userfs_plugins();
// Bootstrap capability: no path lookup and no dependency on a mounted FS.
File *create_boot_disk(unsigned index);
BlockCache *bootstrap_cache(unsigned index);
bool publish_data_device(xv::drivers::BlockDevice &device);
void initialize_mount_manager();
File *create_mount_manager();
int request_managed_mount(const char *type, const char *path,
                          const char *source = nullptr, const char *options = nullptr);
bool managed_mounts_available();
File *create_userfs();
File *register_userfs(const xv::userfs::Registration &);
int mount_registered_userfs(const char *type, const char *path);
bool userfs_provider(unsigned index, xv::userfs::ProviderInfo &);
bool userfs_type(unsigned index, char *name);
bool mount_userfs(File &endpoint, const char *path);
} // namespace xv::fs
