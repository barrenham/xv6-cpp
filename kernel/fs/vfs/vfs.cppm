export module xv.fs:vfs;
export import :filesystem;

export namespace xv::fs
{
struct MountSnapshot
{
  char path[128]{}, type[32]{};
  bool readonly = false, nodev = true;
};

class VirtualFileSystem final
{
public:

  constexpr VirtualFileSystem() = default;
  void initialize_inodes();
  bool namespace_held() const { return namespace_.holding(); }

  Inode *
  root() {
    return service_root_ ? service_root_->root() : nullptr;
  }

  Inode *retain(Inode *node);
  void release(Inode *node);
  void lock(Inode *node);
  void unlock(Inode *node);
  void unlock_release(Inode *node);
  int read(Inode *node, int user, unsigned long destination, unsigned offset,
           unsigned count);
  int write(Inode *node, int user, unsigned long source, unsigned offset,
            unsigned count);
  void stat(Inode *node, Stat *result);
  bool allowed(Inode *node, unsigned requested, bool real = false);
  Inode *lookup(const char *path, bool real = false, bool follow = true);
  // Checked pathname APIs preserve traversal errno and reject truncation aliases.
  Inode *lookup_checked(const char *, int &error, bool follow = true);
  Inode *lookup_parent_checked(const char *, char *, unsigned capacity, int &error);
  // Default preserves the old raw 14-byte caller buffer; internal callers pass 256.
  Inode *lookup_parent(const char *path, char *name, unsigned capacity = 14);
  Inode *create(char *path, short type, short major, short minor, bool follow = true);
  int link(char *old_path, char *new_path);
  int unlink(char *path);
  int rename(const char *old_path, const char *new_path);
  int symlink(const char *target, const char *path);
  bool mount_service(const char *path, FileSystem &backend);
  int unmount(const char *path);
  int synchronize_services();
  int mounts(unsigned index, char *path, char *type, bool canonical = false,
             bool *readonly = nullptr);
  unsigned snapshot_mounts(MountSnapshot *records, unsigned capacity) const;
  bool snapshot_mount(unsigned index, MountSnapshot &record) const;
  Inode *lookup_entry(Inode *directory, const char *name, unsigned *offset);
  Inode *create_at(Inode *directory, const char *name, short type, short major,
                   short minor);
  int link_at(Inode *directory, const char *name, Inode *target);
  int unlink_at(Inode *directory, const char *name);

private:

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
  };

  static bool same(Inode *first, Inode *second);
  Inode *walk(const char *path, bool parent, char *name, bool real = false,
              bool follow = true, bool creating = false, unsigned capacity = 256,
              bool exact_names = false, int *error = nullptr);
  Inode *enter_mount(Inode *node);

  FileSystem &
  root_backend() const {
    return *service_root_;
  }

  FileSystem *service_root_ = nullptr;
  xv::sync::SleepLock namespace_;
  mutable xv::sync::SpinLock mount_snapshot_lock_;
  MountSnapshot mount_snapshots_[9]{};
  unsigned snapshot_count_ = 0;
  void publish_mount_snapshot();
  Mount mounts_[8]{};
  bool ancestor(Inode *possible, Inode *node);
  unsigned mount_count_ = 0;
};

VirtualFileSystem &filesystem();
} // namespace xv::fs
