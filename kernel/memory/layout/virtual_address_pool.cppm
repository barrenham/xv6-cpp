module;
#include "kernel/support/rb_tree.h"
export module xv.vm_pool;
export import xv.vm_types;

export namespace xv::memory
{
// One owner of free intervals and attributed occupied intervals. Page tables
// and physical frames remain external. Callers serialize mutations and keep
// find() pointers local; lifetime ends with explicit reset().
class VirtualAddressPool final
{
public:
  constexpr VirtualAddressPool() = default;
  VirtualAddressPool(const VirtualAddressPool &) = delete;
  VirtualAddressPool &operator=(const VirtualAddressPool &) = delete;
  bool initialize(unsigned long begin = 0, unsigned long end = 1UL << 38,
                  unsigned long granule = 4096);
  // Zero is the failure sentinel for automatic allocation. Fixed reservations
  // may include address zero. allocate_in() additionally restricts the window.
  unsigned long allocate(unsigned long bytes, unsigned permissions = 0,
                         RegionKind kind = RegionKind::Guard, bool demand = false);
  unsigned long allocate_in(unsigned long bytes, unsigned long minimum,
                            unsigned long maximum, unsigned long alignment,
                            unsigned permissions = 0, RegionKind kind = RegionKind::Guard,
                            bool demand = false);
  bool reserve(unsigned long begin, unsigned long end, unsigned permissions,
               RegionKind kind, bool demand = false);
  bool protect(unsigned long begin, unsigned long end, unsigned permissions,
               RegionKind kind, bool demand = false);
  // Exact release never allocates. erase() may split occupied boundary nodes;
  // it acquires all needed metadata before mutation, so OOM changes nothing.
  bool release(unsigned long begin);
  bool erase(unsigned long begin, unsigned long end);
  const VirtualRegion *find(unsigned long address) const;
  bool allocation(unsigned long begin, unsigned long &end) const;
  bool copy_to(VirtualAddressPool &other) const;
  void swap(VirtualAddressPool &other);
  unsigned long largest_free() const;
  void reset();

private:
  struct Node : xv::support::RbLinks<Node>, VirtualRegion
  {
    unsigned long largest = 0;
    void update()
    {
      largest = end - begin;
      if (left && left->largest > largest) largest = left->largest;
      if (right && right->largest > largest) largest = right->largest;
    }
  };
  struct Less
  {
    bool operator()(const Node &a, const Node &b) const { return a.begin < b.begin; }
  };
  using Tree = xv::support::RbTree<Node, Less>;
  Tree free_, allocated_;
  unsigned long begin_ = 0, end_ = 0, granule_ = 0;
  Node *take_node();
  void put_node(Node *node);
  static Node *containing(const Tree &tree, unsigned long address);
  static Node *fit(Node *node, unsigned long bytes, unsigned long minimum,
                   unsigned long maximum, unsigned long alignment, unsigned long &address);
  bool valid(unsigned long begin, unsigned long end) const;
  static bool attributes(unsigned permissions, RegionKind kind, bool demand);
  void return_free(Node *node);
};
} // namespace xv::memory
