export module xv.fs:vfs;
export import :filesystem;
export namespace xv::fs
{
class VirtualFileSystem final
{
public:
  constexpr VirtualFileSystem(FileSystem &disk, FileSystem &devices, FileSystem *processes = nullptr)
      : disk_(disk), devices_(devices), processes_(processes) {}
  void initialize(int device);
  void initialize_inodes();
  Inode *root() { return disk_.root(); }
  Inode *allocate(unsigned device, short type);
  Inode *retain(Inode *node);
  void release(Inode *node);
  void lock(Inode *node);
  void unlock(Inode *node);
  void unlock_release(Inode *node);
  void update(Inode *node);
  void truncate(Inode *node);
  int read(Inode *node, int user, unsigned long destination, unsigned offset, unsigned count);
  int write(Inode *node, int user, unsigned long source, unsigned offset, unsigned count);
  void stat(Inode *node, Stat *result);
  Inode *lookup(const char *path);
  Inode *lookup_parent(const char *path, char *name);
  Inode *create(char *path, short type, short major, short minor);
  int link(char *old_path, char *new_path);
  int unlink(char *path);
  Inode *lookup_entry(Inode *directory, const char *name, unsigned *offset);
  Inode *create_at(Inode *directory, const char *name, short type, short major, short minor);
  int link_at(Inode *directory, const char *name, Inode *target);
  int unlink_at(Inode *directory, const char *name);
private:
  struct Mount
  {
    Inode *covered = nullptr;
    Inode *root = nullptr;
    Inode *parent = nullptr;
  };
  static bool same(Inode *first, Inode *second);
  Inode *walk(const char *path, bool parent, char *name);
  Inode *enter_mount(Inode *node);
  FileSystem &disk_;
  FileSystem &devices_;
  FileSystem *processes_;
  bool frozen_ = false;
  Mount mounts_[3]{};
  unsigned mount_count_ = 0;
public:
  bool mount(const char *path, FileSystem &backend, unsigned device);
};
VirtualFileSystem &filesystem();
}
