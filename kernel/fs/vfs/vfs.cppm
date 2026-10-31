export module xv.fs:vfs;
export import :filesystem;

export namespace xv::fs
{
struct MountSnapshot
{
  char path[128]{},type[32]{};
  bool readonly=false,nodev=true;
};
class VirtualFileSystem final
{
public:

  constexpr VirtualFileSystem(FileSystem &disk, FileSystem &devices,
                              FileSystem *processes = nullptr,
                              FileSystem *temporary = nullptr,
                              FileSystem **dynamic = nullptr,
                              FileSystem *storage = nullptr)
      : disk_(disk), devices_(devices), processes_(processes), temporary_(temporary),
        dynamic_(dynamic), storage_(storage)
  {
  }

  void attach_storage(FileSystem &storage)
  {
    storage_ = &storage;
  }

  void initialize(int device);
  void initialize_inodes();

  Inode *root()
  {
    return disk_.root();
  }

  Inode *allocate(unsigned device, short type);
  Inode *retain(Inode *node);
  void release(Inode *node);
  void lock(Inode *node);
  void unlock(Inode *node);
  void unlock_release(Inode *node);
  void update(Inode *node);
  void truncate(Inode *node);
  int read(Inode *node, int user, unsigned long destination, unsigned offset,
           unsigned count);
  int write(Inode *node, int user, unsigned long source, unsigned offset,
            unsigned count);
  void stat(Inode *node, Stat *result);
  bool allowed(Inode *node, unsigned requested, bool real = false);
  Inode *lookup(const char *path, bool real = false, bool follow = true);
  // Default preserves the old raw 14-byte caller buffer; internal callers pass 256.
  Inode *lookup_parent(const char *path, char *name,unsigned capacity=14);
  Inode *create(char *path, short type, short major, short minor, bool follow = true);
  int link(char *old_path, char *new_path);
  int unlink(char *path);
  int rename(const char *old_path, const char *new_path);
  int symlink(const char *target, const char *path);
  int mount_ram(const char *path);
  int mount_disk(const char *path,const char *type=nullptr);
  int unmount(const char *path);
  int mounts(unsigned index, char *path, char *type,bool canonical=false,bool *readonly=nullptr);
  unsigned snapshot_mounts(MountSnapshot *records,unsigned capacity) const;
  Inode *lookup_entry(Inode *directory, const char *name, unsigned *offset);
  Inode *create_at(Inode *directory, const char *name, short type, short major,
                   short minor);
  int link_at(Inode *directory, const char *name, Inode *target);
  int unlink_at(Inode *directory, const char *name);

private:

  struct InitializeFlow;

  struct MountProtocol;
  struct MountFlow;
  struct MountRamFlow;
  struct MountDiskFlow;
  struct UnmountFlow;

  struct WalkFlow;
  struct AncestorFlow;
  struct CreateFlow;
  struct LinkFlow;
  struct UnlinkFlow;
  struct UnlinkAtFlow;
  struct RenameFlow;
  struct SymlinkFlow;

  struct Mount
  {
    Inode *covered = nullptr;
    Inode *root = nullptr;
    Inode *parent = nullptr;
    char path[128]{};
    int dynamic = -1;
  };

  static bool same(Inode *first, Inode *second);
  Inode *walk(const char *path, bool parent, char *name, bool real = false,
              bool follow = true, bool creating = false,unsigned capacity=256,bool exact_names=false);
  Inode *enter_mount(Inode *node);
  FileSystem &disk_;
  FileSystem &devices_;
  FileSystem *processes_;
  FileSystem *temporary_;
  xv::sync::SleepLock namespace_;
  mutable xv::sync::SpinLock mount_snapshot_lock_;
  MountSnapshot mount_snapshots_[9]{};
  unsigned snapshot_count_=0;
  void publish_mount_snapshot();
  FileSystem **dynamic_ = nullptr;
  FileSystem *storage_ = nullptr;
  bool storage_ready_ = false;
  bool used_[4]{};
  Mount mounts_[8]{};
  unsigned next_device_ = 16;
  bool ancestor(Inode *possible, Inode *node);
  unsigned mount_count_ = 0;

public:

  bool mount(const char *path, FileSystem &backend, unsigned device);
};

VirtualFileSystem &filesystem();
} // namespace xv::fs
