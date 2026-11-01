#pragma once
#include "kernel/support/rb_tree.h"

namespace xv::time
{
// Caller supplies exclusion. An armed node must remain at a stable address.
// Removing a node before its owner is released prevents stale callback access.
struct Deadline : xv::support::RbLinks<Deadline>
{
  unsigned long at = 0;
  void (*notify)(void *) = nullptr;
  void *owner = nullptr;
  bool armed = false;
};

struct Earlier
{
  bool operator()(const Deadline &a, const Deadline &b) const
  {
    return a.at < b.at || (a.at == b.at && reinterpret_cast<unsigned long>(&a) <
                                               reinterpret_cast<unsigned long>(&b));
  }
};

class DeadlineQueue
{
public:

  bool empty() const
  {
    return tree_.root() == nullptr;
  }

  void cancel(Deadline &node)
  {
    if (!node.armed)
    {
      return;
    }
    tree_.erase(node);
    node.armed = false;
  }

  void reset(Deadline &node, unsigned long at, void (*notify)(void *), void *owner)
  {
    cancel(node);
    node.at = at;
    node.notify = notify;
    node.owner = owner;
    tree_.insert(node);
    node.armed = true;
  }

  unsigned long next() const
  {
    auto *node = tree_.first();
    return node ? node->at : ~0UL;
  }

  // Notifications run under caller's lock: bounded wakeups only, never protocol
  // work, allocation, sleep, or re-entry into this queue. Cancellation therefore
  // synchronizes with completion of a notification, including stack-owned nodes.
  unsigned expire(unsigned long now, unsigned budget = 64)
  {
    unsigned count = 0;
    while (count < budget)
    {
      auto *node = tree_.first();
      if (!node || node->at > now)
      {
        break;
      }
      cancel(*node);
      node->notify(node->owner);
      ++count;
    }
    return count;
  }

private:

  xv::support::RbTree<Deadline, Earlier> tree_;
};
} // namespace xv::time
