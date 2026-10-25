module;
#include "kernel/support/placement_new.hpp"

module xv.process;
import :paged_store;
import :storage_platform;

namespace xv::process
{
void PagedProcessStore::initialize()
{
  lock_.init("process registry");
}

Process *PagedProcessStore::process_at(Node &node)
{
  return __builtin_launder(reinterpret_cast<Process *>(node.storage));
}

PagedProcessStore::Node *PagedProcessStore::node_at(Process &process)
{
  return reinterpret_cast<Node *>(reinterpret_cast<unsigned char *>(&process) -
                                  __builtin_offsetof(Node, storage));
}

Process *PagedProcessStore::allocate()
{
  static_assert(sizeof(Node) <= 4096);
  void *page = detail::allocate_process_page();
  if (!page)
  {
    return nullptr;
  }
  auto stack = detail::allocate_kernel_stack();
  if (!stack)
  {
    detail::free_process_page(page);
    return nullptr;
  }
  auto *node = new (page) Node{};
  auto *process = construct(node->storage);
  set_stack(*process, stack);
  {
    // The new process is not yet published, so no reader can be waiting for
    // its lock while this allocator takes the registry lock.
    xv::sync::ScopedLock guard(lock_);
    node->next = head_;
    head_ = node;
    ++count_;
  }
  return process;
}

void PagedProcessStore::release(Process *process)
{
  auto *node = node_at(*process);
  auto stack = process->kernel_stack();
  // Readers use registry -> process order. Never wait for the registry while
  // holding a published process lock: mark retired and drop it first.
  retire(*process);
  {
    xv::sync::ScopedLock guard(lock_);
    Node **link = &head_;
    while (*link && *link != node)
    {
      link = &(*link)->next;
    }
    if (!*link)
    {
      detail::storage_fail("process store: unknown descriptor");
    }
    *link = node->next;
    --count_;
    // A successful find may still own the lock after releasing the registry.
    lock(*process);
  }
  unlock(*process);
  destroy(*process);
  node->~Node();
  detail::free_kernel_stack(stack);
  detail::free_process_page(node);
}

Process *PagedProcessStore::find(ProcessPredicate &predicate)
{
  xv::sync::ScopedLock guard(lock_);
  for (auto *node = head_; node; node = node->next)
  {
    auto *process = process_at(*node);
    lock(*process);
    if (predicate.matches(*process))
    {
      return process;
    }
    unlock(*process);
  }
  return nullptr;
}

void PagedProcessStore::visit(ProcessVisitor &visitor)
{
  xv::sync::ScopedLock guard(lock_);
  for (auto *node = head_; node; node = node->next)
  {
    auto *process = process_at(*node);
    lock(*process);
    visitor.visit(*process);
    unlock(*process);
  }
}

unsigned long PagedProcessStore::count() const
{
  xv::sync::ScopedLock guard(lock_);
  return count_;
}
} // namespace xv::process
