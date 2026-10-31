module;
#include "kernel/fs/ext3/volume.h"
#include "kernel/fs/format/layout.h"
#include "kernel/support/param.h"
module xv.fs:ext3;
import :filesystem;

namespace xv::fs
{
// Internal xv.fs adapter. The portable wire-format engine has no VFS or
// process dependencies; all kernel ownership and synchronization live here.
class Ext3FileSystem final : public FileSystem
{
  class DiskIo final : public xv::ext3::Io
  {
    BlockCache &cache_;
    void *reserve_[3]{};
    bool borrowed_[3]{};
    bool committing_=false;
  public:
    xv::drivers::BlockDevice *disk=nullptr;
    unsigned device=0;
    constexpr explicit DiskIo(BlockCache &cache):cache_(cache) {}
    bool prepare_reserve();
    void release_reserve();
    bool commit(xv::ext3::Volume &volume);
    bool read(xv::ext3::u64,void *,unsigned) override;
    bool write(xv::ext3::u64,const void *,unsigned) override;
    bool flush() override;
    xv::ext3::u64 size_bytes() const override;
    xv::ext3::u32 now_seconds() const override;
    void *allocate_page() override;
    void free_page(void *) override;
  };
  class Log final : public Journal
  {
    Ext3FileSystem &owner_;
    xv::sync::SpinLock state_;
    unsigned outstanding_=0,used_=0,capacity_=0;
    bool committing_=false;
    static constexpr unsigned Credits=32;
    void wait();
    void commit(bool flush);
  public:
    constexpr explicit Log(Ext3FileSystem &owner):owner_(owner) {}
    void initialize();
    void ready(unsigned capacity);
    void changed(unsigned used);
    void begin() override;
    void end() override;
    void record(Buffer &) override;
    void sync() override;
  };
public:
  constexpr explicit Ext3FileSystem(BlockCache &cache):io_(cache),volume_(io_),log_(*this) {}
  void attach(xv::drivers::BlockDevice &device) { io_.disk=&device; }
  unsigned name_max() const override {return 255;}
  const char *name() const override { return "ext3"; }
  void initialize(int device) override;
  void initialize_inodes() override;
  bool busy() override;
  void shutdown() override;
  Inode *root() override;
  Inode *lookup_entry(Inode *,const char *,unsigned *) override;
  Inode *create_at(Inode *,const char *,short,short,short) override;
  int link_at(Inode *,const char *,Inode *) override;
  int unlink_at(Inode *,const char *) override;
  int rename_at(Inode *,const char *,Inode *,const char *) override;
  Inode *allocate(unsigned,short) override;
  Inode *retain(Inode *) override;
  void release(Inode *) override;
  void lock(Inode *) override;
  void unlock(Inode *) override;
  void unlock_release(Inode *) override;
  void update(Inode *) override;
  void truncate(Inode *) override;
  int resize(Inode *,unsigned) override;
  // The VFS can request a device barrier inside its own transaction (unmount).
  // Journal::sync is the separate operation that waits for all participants.
  void flush() override { io_.flush(); }
  Journal *transaction_log() override { return &log_; }
  int next_directory(Inode *,unsigned long &,DirectoryRecord &) override;
  int read(Inode *,int,unsigned long,unsigned,unsigned) override;
  int write(Inode *,int,unsigned long,unsigned,unsigned) override;
  void stat(Inode *,Stat *) override;
  xv::drivers::BlockDevice *open_block_device(Inode *,bool) override;
  xv::drivers::CharacterDevice *open_device(Inode *,bool) override;
private:
  DiskIo io_;
  xv::ext3::Volume volume_;
  xv::sync::SleepLock core_;
  xv::sync::SpinLock references_;
  Log log_;
  Inode inodes_[NINODE]{};
  bool mounted_=false;
  bool garbage_pending_=false;
  struct CollectFlow;
  void collect_orphans();
  Inode *get(unsigned number);
  void refresh(Inode *,const xv::ext3::Inode &);
  bool wire(Inode *,xv::ext3::Inode &);
  void changed();
  static bool component(const char *,char (&)[256]);
};
}
