module xv.process:paged_store;

import :store;
import xv.sync;

namespace xv::process
{
// Each descriptor lives in its own allocator page. Nodes never move, and are
// removed only after callers have released all execution/resource ownership.
class PagedProcessStore final : public ProcessStore
{
public:

  constexpr PagedProcessStore() = default;

  const char *name() const override
  {
    return "paged-registry";
  }

  void initialize() override;
  Process *allocate() override;
  void release(Process *process) override;
  Process *find(ProcessPredicate &predicate) override;
  void visit(ProcessVisitor &visitor) override;
  unsigned long count() const override;

private:

  struct Node
  {
    Node *next = nullptr;
    alignas(Process) unsigned char storage[sizeof(Process)];
  };

  static Process *process_at(Node &node);
  static Node *node_at(Process &process);
  mutable xv::sync::SpinLock lock_;
  Node *head_ = nullptr;
  unsigned long count_ = 0;
};
} // namespace xv::process
