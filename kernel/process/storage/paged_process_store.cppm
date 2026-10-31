module;
#include "kernel/support/rb_tree.h"
module xv.process:paged_store;

import :store;
import xv.sync;

namespace xv::process
{
// Stable descriptors share backing pages through an exact-size slab cache.
// Removal follows release of all execution/resource ownership.
class PagedProcessStore final : public ProcessStore
{
public:

  constexpr PagedProcessStore() = default;

  const char *name() const override
  {
    return "slab-registry";
  }

  void initialize() override;
  Process *allocate(int pid = 0) override;
  void release(Process *process) override;
  Process *find(ProcessPredicate &predicate) override;
  Process *find_pid(int pid) override;
  void visit(ProcessVisitor &visitor) override;
  unsigned long count() const override;

private:

  struct AllocateFlow;
  struct ReleaseFlow;

  struct Node : xv::support::RbLinks<Node>
  {
    Node *next = nullptr, *previous = nullptr;
    int indexed_pid = 0;
    PagedProcessStore *owner = nullptr;
    alignas(Process) unsigned char storage[sizeof(Process)];
  };

  struct Less
  {
    bool operator()(const Node &a, const Node &b) const { return a.indexed_pid < b.indexed_pid; }
  };
  xv::support::RbTree<Node, Less> pids_;

  static Process *process_at(Node &node);
  static Node *node_at(Process &process);
  mutable xv::sync::SpinLock lock_;
  Node *head_ = nullptr;
  unsigned long count_ = 0;
};
} // namespace xv::process
