module xv.fs:disk;
import :filesystem;

namespace xv::fs
{
// Select the on-disk implementation at mount, once block I/O can sleep.
// Inodes retain the concrete backend as their owner; the router owns no inode
// cache and never changes an existing inode's dispatch or lock identity.
class DiskFileSystem final : public FileSystem
{
  FileSystem &legacy_,&ext3_;
  BlockCache &cache_;
  FileSystem *selected_;
public:
  constexpr DiskFileSystem(FileSystem &legacy,FileSystem &ext3,BlockCache &cache)
    :legacy_(legacy),ext3_(ext3),cache_(cache),selected_(&legacy) {}
  const char *name() const override {return selected_->name();}
  const char *type_name() const override {return selected_->type_name();}
  void initialize(int device) override;
  void initialize_inodes() override {legacy_.initialize_inodes();ext3_.initialize_inodes();}
  bool busy() override {return selected_->busy();}
  void shutdown() override {selected_->shutdown();}
  Inode *root() override {return selected_->root();}
  Inode *lookup_entry(Inode *d,const char *n,unsigned *o) override {return selected_->lookup_entry(d,n,o);}
  Inode *create_at(Inode *d,const char *n,short t,short a,short b) override {return selected_->create_at(d,n,t,a,b);}
  int link_at(Inode *d,const char *n,Inode *t) override {return selected_->link_at(d,n,t);}
  int unlink_at(Inode *d,const char *n) override {return selected_->unlink_at(d,n);}
  int rename_at(Inode *a,const char *n,Inode *b,const char *m) override {return selected_->rename_at(a,n,b,m);}
  Inode *allocate(unsigned d,short t) override {return selected_->allocate(d,t);}
  Inode *retain(Inode *n) override {return selected_->retain(n);}
  void release(Inode *n) override {selected_->release(n);}
  void lock(Inode *n) override {selected_->lock(n);}
  void unlock(Inode *n) override {selected_->unlock(n);}
  void unlock_release(Inode *n) override {selected_->unlock_release(n);}
  void update(Inode *n) override {selected_->update(n);}
  int next_directory(Inode *n,unsigned long &cookie,DirectoryRecord &entry) override {return selected_->next_directory(n,cookie,entry);}
  unsigned name_max() const override {return selected_->name_max();}
  void truncate(Inode *n) override {selected_->truncate(n);}
  int resize(Inode *n,unsigned s) override {return selected_->resize(n,s);}
  void flush() override {selected_->flush();}
  Journal *transaction_log() override {return selected_->transaction_log();}
  int read(Inode *n,int u,unsigned long p,unsigned o,unsigned c) override {return selected_->read(n,u,p,o,c);}
  int write(Inode *n,int u,unsigned long p,unsigned o,unsigned c) override {return selected_->write(n,u,p,o,c);}
  void stat(Inode *n,Stat *s) override {selected_->stat(n,s);}
  xv::drivers::CharacterDevice *open_device(Inode *n,bool b) override {return selected_->open_device(n,b);}
};
}
