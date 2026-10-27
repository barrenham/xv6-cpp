module;
#include "kernel/fs/format/layout.h"
module xv.fs:ramfs;
import :filesystem;
namespace xv::fs
{
class RamFileSystem final : public FileSystem
{
public:
  constexpr RamFileSystem() = default;
  const char *name() const override { return "ramfs"; }
  bool persistent() const override { return false; }
  void initialize(int device) override;
  void initialize_inodes() override;
  Inode *root() override;
  Inode *lookup_entry(Inode *directory, const char *name, unsigned *offset) override;
  Inode *create_at(Inode *directory, const char *name, short type, short major, short minor) override;
  int link_at(Inode *directory, const char *name, Inode *target) override;
  int unlink_at(Inode *directory, const char *name) override;
  Inode *allocate(unsigned, short) override { return nullptr; }
  Inode *retain(Inode *node) override;
  void release(Inode *node) override;
  void lock(Inode *) override { tree_.acquire(); }
  void unlock(Inode *) override { tree_.release(); }
  void unlock_release(Inode *node) override;
  void update(Inode *) override {}
  void truncate(Inode *node) override;
  int read(Inode *node, int user, unsigned long destination, unsigned offset, unsigned count) override;
  int write(Inode *node, int user, unsigned long source, unsigned offset, unsigned count) override;
  void stat(Inode *node, Stat *result) override;
private:
  struct Node;
  struct Slot { Node *target = nullptr; char name[DIRSIZ]{}; };
  struct DirectoryPage
  {
    DirectoryPage *next = nullptr;
    Slot slots[(4096 - sizeof(DirectoryPage *)) / sizeof(Slot)]{};
  };
  struct Node
  {
    Inode inode;
    Node *parent = nullptr;
    PageBuffer contents;
    DirectoryPage *directory = nullptr;
    unsigned slots = 0;
  };
  static Node *state(Inode *node) { return static_cast<Node *>(node->context_); }
  Node *make(short type, Node *parent);
  Slot *slot(Node *node, unsigned index, bool create);
  Slot *find(Node *node, const char *name, unsigned *index = nullptr);
  bool insert(Node *parent, const char *name, Node *target);
  bool empty(Node *node);
  void reclaim(Node *node);
  xv::sync::SleepLock tree_;
  xv::sync::SpinLock references_;
  unsigned long numbers_[1024]{};
  Node *root_ = nullptr;
  unsigned device_ = 0;
};
}
