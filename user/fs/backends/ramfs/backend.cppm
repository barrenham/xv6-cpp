module;
#include "kernel/support/types.h"
#include "kernel/fs/userfs/protocol.h"
export module xv.userfs.ramfs;
export import xv.userfs;

export namespace userfs
{
class MemoryStorage
{
public:
  virtual void *allocate(unsigned bytes) = 0;
  virtual void release(void *) = 0;
protected:
  ~MemoryStorage() = default;
};
MemoryStorage &heap_storage();

// Sparse file pages, directory entries and inodes grow independently. The only
// lifetime borrow from the kernel is one aggregate reference until Forget.
class MemoryBackend final : public Backend
{
public:
  unsigned cache_flags() const override { return xv::userfs::CacheMetadata; }
  explicit MemoryBackend(MemoryStorage &, unsigned root_mode = 0755);
  ~MemoryBackend();
  bool ready() const { return root_ != nullptr; }
  Attributes root() override;
  int lookup(unsigned, const char *, Attributes &) override;
  int getattr(unsigned, Attributes &) override;
  int readdir(unsigned, unsigned long, Entry &) override;
  int read(unsigned, unsigned long, void *, unsigned) override;
  int write(unsigned, unsigned long, const void *, unsigned) override;
  int create(unsigned, const char *, const Attributes &, Attributes &) override;
  int resize(unsigned, unsigned) override;
  int setattr(unsigned, const Attributes &) override;
  int link(unsigned, const char *, unsigned) override;
  int unlink(unsigned, const char *, Attributes &) override;
  int rename(unsigned, const char *, unsigned, const char *) override;
  void forget(unsigned) override;

private:
  struct Node;
  struct Name;
  struct Page;
  Node *find(unsigned) const;
  static bool valid_name(const char *);
  static Name **entry(Node &, const char *);
  static Page **page(Node &, unsigned);
  static bool directory(const Node &);
  static void directory_size(Node &);
  void reclaim(Node *);
  static void append(Node &, Name *);
  void remove(Node &, Name *);
  void drop(Node &, Node &);
  MemoryStorage &storage_;
  Node *nodes_ = nullptr, *root_ = nullptr;
  unsigned next_node_ = 2;
};
} // namespace userfs
