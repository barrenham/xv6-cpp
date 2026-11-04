module;
#include "kernel/support/types.h"
#include "kernel/fs/userfs/protocol.h"
#include "kernel/io/fs_resources.h"
#include "user/runtime/user.h"
export module xv.userfs.procfs;
export import xv.userfs;
import xv.userfs.resources;
import xv.userfs.snapshot;

export namespace userfs
{
class ProcessBackend final : public Backend
{
public:
  explicit ProcessBackend(int resources) : resources_(resources) {}
  ~ProcessBackend();
  bool ready() const { return resources_.ready(); }
  unsigned flags() override { return xv::userfs::ReadOnly | xv::userfs::Snapshots; }
  Attributes root() override { return {1, 0, 0, 0, 0555, 1, 1}; }
  void context(const xv::userfs::RequestContext &context) override { actor_ = context; }
  int lookup(unsigned, const char *, Attributes &) override;
  int getattr(unsigned, Attributes &) override;
  int readdir(unsigned, unsigned long, Entry &) override;
  int read(unsigned, unsigned long, void *, unsigned) override;
  void forget(unsigned) override;
  int snapshot(unsigned, unsigned &) override;
  int snapshot_read(unsigned long, void *, unsigned) override;
  void snapshot_end() override { bytes_.clear(); }
private:
  enum Kind : unsigned { Root, Global, ProcessDirectory, Status, FdDirectory, Fd,
                          FdInfoDirectory, FdInfo, Command, Environment, Name };
  struct Node
  {
    Node *next = nullptr;
    unsigned id = 0, kind = Root, pid = 0, descriptor = 0;
    unsigned uid = 0, gid = 0;
  };
  enum GlobalFile : unsigned { Plugins, BlockCache, Memory, Uptime, Statistics,
      CpuInfo, Filesystems, Mounts, Devices, Partitions, Version, GlobalCount };
  static const char *const global_names_[GlobalCount];
  static bool decimal(const char *, unsigned &, bool = false);
  static void number_name(unsigned, char *);
  static bool directory(unsigned);
  Node *find(unsigned);
  bool key(unsigned, Node &);
  bool process(unsigned, xv::fsservice::Process &);
  bool attributes(const Node &, Attributes &);
  unsigned identity(const Node &) const;
  bool describe(unsigned kind, unsigned pid, unsigned descriptor, Attributes &, bool borrow);
  bool descriptors(unsigned pid);
  bool descriptor(unsigned pid, unsigned fd, xv::fsservice::Descriptor &);
  bool render_directory(const Node &);
  bool render_global(unsigned);
  bool render_process(const Node &);
  bool render_exec(const Node &);
  bool record(const char *, unsigned);
  Resources resources_;
  Node *nodes_ = nullptr;
  unsigned next_id_ = 64;
  xv::userfs::RequestContext actor_;
  SnapshotBytes bytes_;
};
} // namespace userfs
