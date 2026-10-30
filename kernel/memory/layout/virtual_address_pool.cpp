module;
#include "kernel/support/placement_new.hpp"
#include "kernel/arch/riscv/register_constants.h"
module xv.vm_pool;
import xv.memory;

namespace xv::memory
{
namespace
{
#if XV_DEFAULT_SLAB_BITMAP
constinit BitmapSlabCache pool_nodes;
#else
constinit FreeListSlabCache pool_nodes;
#endif
int pool_nodes_ready = 0;
void prepare_pool_nodes(unsigned long bytes, unsigned long alignment)
{
  if (__atomic_load_n(&pool_nodes_ready, __ATOMIC_ACQUIRE) == 2) return;
  int expected = 0;
  if (__atomic_compare_exchange_n(&pool_nodes_ready, &expected, 1, false,
                                   __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
  {
    if (!pool_nodes.initialize(allocator(), bytes, alignment, 0)) __builtin_trap();
    __atomic_store_n(&pool_nodes_ready, 2, __ATOMIC_RELEASE);
  }
  else while (__atomic_load_n(&pool_nodes_ready, __ATOMIC_ACQUIRE) != 2) {}
}
}

VirtualAddressPool::Node *VirtualAddressPool::take_node()
{
  prepare_pool_nodes(sizeof(Node), alignof(Node));
  auto *storage = pool_nodes.allocate();
  return storage ? new (storage) Node{} : nullptr;
}

void VirtualAddressPool::put_node(Node *node)
{
  node->~Node();
  if (!pool_nodes.deallocate(node)) __builtin_trap();
}

bool VirtualAddressPool::initialize(unsigned long begin, unsigned long end,
                                    unsigned long granule)
{
  if (granule_ || begin >= end || end > (1UL << 38) || !granule ||
      (granule & (granule - 1)) || begin % granule || end % granule)
    return false;
  auto *node = take_node();
  if (!node) return false;
  node->begin = begin;
  node->end = end;
  free_.insert(*node);
  begin_ = begin;
  end_ = end;
  granule_ = granule;
  return true;
}

bool VirtualAddressPool::valid(unsigned long begin, unsigned long end) const
{
  return granule_ && begin >= begin_ && begin < end && end <= end_ &&
         !(begin % granule_) && !(end % granule_);
}

bool VirtualAddressPool::attributes(unsigned permissions, RegionKind kind, bool demand)
{
  return !(permissions & ~(PTE_R | PTE_W | PTE_X | PTE_U)) &&
         (!(permissions & PTE_W) || (permissions & PTE_R)) &&
         (kind != RegionKind::Guard || (!permissions && !demand));
}

VirtualAddressPool::Node *VirtualAddressPool::containing(const Tree &tree,
                                                        unsigned long address)
{
  Node key;
  key.begin = address;
  auto *node = tree.lower_bound(key);
  if (!node || node->begin != address) node = node ? Tree::previous(node) : tree.last();
  return node && address < node->end ? node : nullptr;
}

const VirtualRegion *VirtualAddressPool::find(unsigned long address) const
{
  return containing(allocated_, address);
}

bool VirtualAddressPool::reserve(unsigned long begin, unsigned long end,
                                 unsigned permissions, RegionKind kind, bool demand)
{
  if (!valid(begin, end) || !attributes(permissions, kind, demand)) return false;
  auto *free = containing(free_, begin);
  if (!free || free->end < end) return false;
  const bool left = free->begin < begin, right = end < free->end;
  Node *owned = free, *prefix = nullptr;
  if (left || right)
  {
    owned = take_node();
    if (!owned) return false;
    if (left && right)
    {
      prefix = take_node();
      if (!prefix) { put_node(owned); return false; }
    }
  }
  // All resources are acquired. Keep the existing free tail's descriptor so
  // temporary allocations do not strand an otherwise unused metadata slab.
  free_.erase(*free);
  if (prefix)
  {
    prefix->begin = free->begin;
    prefix->end = begin;
    free_.insert(*prefix);
  }
  if (right) free->begin = end;
  else if (left) free->end = begin;
  if (left || right) free_.insert(*free);
  static_cast<VirtualRegion &>(*owned) = {begin, end, permissions, kind, demand};
  allocated_.insert(*owned);
  return true;
}

VirtualAddressPool::Node *VirtualAddressPool::fit(Node *node, unsigned long bytes,
    unsigned long minimum, unsigned long maximum, unsigned long alignment,
    unsigned long &address)
{
  if (!node || node->largest < bytes) return nullptr;
  if (node->end <= minimum) return fit(node->right, bytes, minimum, maximum, alignment, address);
  if (node->begin >= maximum) return fit(node->left, bytes, minimum, maximum, alignment, address);
  if (auto *left = fit(node->left, bytes, minimum, maximum, alignment, address)) return left;
  auto begin = node->begin > minimum ? node->begin : minimum;
  auto end = node->end < maximum ? node->end : maximum;
  begin = (begin + alignment - 1) & ~(alignment - 1);
  if (begin < end && bytes <= end - begin) { address = begin; return node; }
  return fit(node->right, bytes, minimum, maximum, alignment, address);
}

unsigned long VirtualAddressPool::allocate(unsigned long bytes, unsigned permissions,
                                          RegionKind kind, bool demand)
{
  return allocate_in(bytes, granule_, end_, granule_, permissions, kind, demand);
}

unsigned long VirtualAddressPool::allocate_in(unsigned long bytes, unsigned long minimum,
    unsigned long maximum, unsigned long alignment, unsigned permissions,
    RegionKind kind, bool demand)
{
  if (!granule_ || !bytes || bytes % granule_ || !alignment ||
      alignment < granule_ || alignment > (1UL << 38) || (alignment & (alignment - 1)) ||
      minimum >= maximum || maximum > end_ || !attributes(permissions, kind, demand)) return 0;
  if (minimum < begin_) minimum = begin_;
  if (!minimum) minimum = granule_;
  unsigned long address = 0;
  if (!fit(free_.root(), bytes, minimum, maximum, alignment, address)) return 0;
  return reserve(address, address + bytes, permissions, kind, demand) ? address : 0;
}

bool VirtualAddressPool::protect(unsigned long begin, unsigned long end,
                                 unsigned permissions, RegionKind kind, bool demand)
{
  if (!valid(begin, end) || !attributes(permissions, kind, demand)) return false;
  auto *node = containing(allocated_, begin);
  if (!node || node->end < end) return false;
  Node *left = nullptr, *right = nullptr;
  if (node->begin < begin)
  {
    left = take_node();
    if (!left) return false;
  }
  if (end < node->end)
  {
    right = take_node();
    if (!right) { if (left) put_node(left); return false; }
  }
  allocated_.erase(*node);
  if (left)
  {
    static_cast<VirtualRegion &>(*left) = *node;
    left->end = begin;
    allocated_.insert(*left);
  }
  if (right)
  {
    static_cast<VirtualRegion &>(*right) = *node;
    right->begin = end;
    allocated_.insert(*right);
  }
  static_cast<VirtualRegion &>(*node) = {begin, end, permissions, kind, demand};
  allocated_.insert(*node);
  return true;
}

void VirtualAddressPool::return_free(Node *node)
{
  auto *right = free_.lower_bound(*node);
  auto *left = right ? Tree::previous(right) : free_.last();
  if (left && left->end == node->begin)
  {
    free_.erase(*left);
    left->end = node->end;
    put_node(node);
    node = left;
  }
  if (right && node->end == right->begin)
  {
    free_.erase(*right);
    right->begin = node->begin;
    put_node(node);
    node = right;
  }
  // Free nodes do not retain business attributes.
  node->permissions = 0;
  node->kind = RegionKind::Guard;
  node->demand = false;
  free_.insert(*node);
}

bool VirtualAddressPool::release(unsigned long begin)
{
  Node key;
  key.begin = begin;
  auto *node = allocated_.find(key);
  if (!node) return false;
  allocated_.erase(*node);
  return_free(node);
  return true;
}

bool VirtualAddressPool::erase(unsigned long begin, unsigned long end)
{
  if (!valid(begin, end)) return false;
  Node key;
  key.begin = begin;
  auto *first = containing(allocated_, begin);
  if (!first) first = allocated_.lower_bound(key);
  if (!first || first->begin >= end) return true;
  auto *last = containing(allocated_, end - 1);
  unsigned needed = (first->begin < begin) + (last && last->end > end);
  Node *spares[2]{};
  for (unsigned i = 0; i < needed; ++i)
  {
    spares[i] = take_node();
    if (!spares[i])
    {
      while (i) put_node(spares[--i]);
      return false;
    }
  }
  auto *node = first;
  while (node && node->begin < end)
  {
    auto *next = Tree::next(node);
    const auto old = static_cast<const VirtualRegion &>(*node);
    auto low = old.begin < begin ? begin : old.begin;
    auto high = old.end > end ? end : old.end;
    allocated_.erase(*node);
    Node *freed = node;
    if (old.begin < low)
    {
      node->end = low;
      allocated_.insert(*node);
      freed = spares[--needed];
      if (high < old.end)
      {
        auto *tail = spares[--needed];
        static_cast<VirtualRegion &>(*tail) = old;
        tail->begin = high;
        allocated_.insert(*tail);
      }
    }
    else if (high < old.end)
    {
      node->begin = high;
      allocated_.insert(*node);
      freed = spares[--needed];
    }
    freed->begin = low;
    freed->end = high;
    return_free(freed);
    node = next;
  }
  return true;
}

bool VirtualAddressPool::allocation(unsigned long begin, unsigned long &end) const
{
  auto *node = find(begin);
  if (!node || node->begin != begin) return false;
  end = node->end;
  return true;
}

bool VirtualAddressPool::copy_to(VirtualAddressPool &other) const
{
  if (&other == this) return true;
  VirtualAddressPool candidate;
  if (granule_)
  {
    if (!candidate.initialize(begin_, end_, granule_)) return false;
    for (auto *node = allocated_.first(); node; node = Tree::next(node))
      if (!candidate.reserve(node->begin, node->end, node->permissions, node->kind, node->demand))
      { candidate.reset(); return false; }
  }
  other.swap(candidate);
  candidate.reset();
  return true;
}

void VirtualAddressPool::swap(VirtualAddressPool &other)
{
  free_.swap(other.free_);
  allocated_.swap(other.allocated_);
  auto exchange = []<class T>(T &a, T &b) { auto value = a; a = b; b = value; };
  exchange(begin_, other.begin_);
  exchange(end_, other.end_);
  exchange(granule_, other.granule_);
}

unsigned long VirtualAddressPool::largest_free() const
{
  return free_.root() ? free_.root()->largest : 0;
}

void VirtualAddressPool::reset()
{
  while (auto *node = free_.first()) { free_.erase(*node); put_node(node); }
  while (auto *node = allocated_.first()) { allocated_.erase(*node); put_node(node); }
  begin_ = end_ = granule_ = 0;
}
} // namespace xv::memory
