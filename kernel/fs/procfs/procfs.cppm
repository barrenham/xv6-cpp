module xv.fs:procfs;
import :filesystem;
import :proc_providers;

namespace xv::fs
{
class ProcFileSystem final : public FileSystem
{
public:

  constexpr ProcFileSystem() = default;

  const char *name() const override
  {
    return "procfs";
  }
  const char *type_name() const override {return "proc";}

  bool persistent() const override
  {
    return false;
  }

  bool read_only() const override
  {
    return true;
  }

  bool snapshot_reads() const override
  {
    return true;
  }

  bool snapshot(Inode *node, PageBuffer &buffer) override;
  bool register_file(const char *name,ProcProviderRegistry::Render render,unsigned mode=0444)
  {return providers_.add(name,render,mode);}
  bool register_process_file(const char *name,ProcProcessRegistry::Render render,unsigned mode=0400)
  {return process_providers_.add(name,render,mode);}
  void initialize(int device) override;
  void initialize_inodes() override;

  Inode *root() override
  {
    return retain(&nodes_[0]);
  }

  Inode *lookup_entry(Inode *directory, const char *name, unsigned *offset) override;

  Inode *create_at(Inode *, const char *, short, short, short) override
  {
    return nullptr;
  }

  int link_at(Inode *, const char *, Inode *) override
  {
    return -1;
  }

  int unlink_at(Inode *, const char *) override
  {
    return -1;
  }

  Inode *allocate(unsigned, short) override
  {
    return nullptr;
  }

  Inode *retain(Inode *node) override;
  void release(Inode *node) override;
  void lock(Inode *node) override;
  void unlock(Inode *node) override;
  void unlock_release(Inode *node) override;
  void update(Inode *) override;
  int next_directory(Inode *node,unsigned long &cookie,DirectoryRecord &entry) override
  {return legacy_next_directory(node,cookie,entry);}
  void truncate(Inode *) override;
  int read(Inode *node, int user, unsigned long destination, unsigned offset,
           unsigned count) override;

  int write(Inode *, int, unsigned long, unsigned, unsigned) override
  {
    return -1;
  }

  void stat(Inode *node, Stat *result) override;

private:

  struct GetFlow;
  struct LookupFlow;
  struct SnapshotFlow;
  static constexpr unsigned NodeCount = 256;
  static constexpr unsigned RootSlots=1+ProcProviderRegistry::Capacity;
  static constexpr unsigned RootFile=7;
  static constexpr unsigned ProcessFile=8;
  static constexpr unsigned FdInfoDirectory=9,FdInfo=10;
  ProcProviderRegistry providers_;
  ProcProcessRegistry process_providers_;
  unsigned next_inode_=64;
  Inode *get(unsigned kind, unsigned pid, unsigned descriptor = 0);
  xv::sync::SpinLock references_;
  Inode nodes_[NodeCount]{};
  unsigned device_ = 0;
};
void install_proc_providers(ProcFileSystem &filesystem);
} // namespace xv::fs
