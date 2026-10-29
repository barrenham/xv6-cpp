module;
#include "kernel/support/placement_new.hpp"
#include "kernel/support/flow.h"

module xv.process;
import :paged_store;
import :storage_platform;

namespace xv::process
{

namespace flow = xv::flow;

struct PagedProcessStore::AllocateFlow
{
  struct Context
  {
    PagedProcessStore &owner;
    void *page = nullptr;
    unsigned long stack = 0;
    Node *node = nullptr;
    Process *process = nullptr;
  };

  static void page(Context &c) noexcept
  {
    c.page = detail::allocate_process_page();
  }

  static bool has_page(Context &c) noexcept
  {
    return c.page != nullptr;
  }

  static void stack(Context &c) noexcept
  {
    c.stack = detail::allocate_kernel_stack();
  }

  static bool has_stack(Context &c) noexcept
  {
    return c.stack != 0;
  }

  static void rollback(Context &c) noexcept
  {
    detail::free_process_page(c.page);
  }

  static void construct(Context &c) noexcept
  {
    c.node = new (c.page) Node{};
    c.process = ProcessStore::construct(c.node->storage);
    set_stack(*c.process, c.stack);
  }

  static void publish(Context &c) noexcept
  {
    // The unpublished candidate is locked; no registry reader can yet wait on it.
    xv::sync::ScopedLock guard(c.owner.lock_);
    c.node->next = c.owner.head_;
    c.owner.head_ = c.node;
    ++c.owner.count_;
  }
  struct Page;
  struct Stack;
  struct Rollback;
  struct Construct;
  struct Publish;
  using Graph = flow::graph<
      Page,
      flow::types<flow::node<Page, flow::action<&page>,
                             flow::branch<flow::condition<&has_page>, flow::go<Stack>,
                                          flow::stop>>,
                  flow::node<Stack, flow::action<&stack>,
                             flow::branch<flow::condition<&has_stack>,
                                          flow::go<Construct>, flow::go<Rollback>>>,
                  flow::node<Rollback, flow::action<&rollback>, flow::stop>,
                  flow::node<Construct, flow::action<&construct>, flow::go<Publish>>,
                  flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

struct PagedProcessStore::ReleaseFlow
{
  struct Context
  {
    PagedProcessStore &owner;
    Process &process;
    Node *node;
    unsigned long stack;
  };

  static void retire(Context &c) noexcept
  {
    ProcessStore::retire(c.process);
  }

  static void remove(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.lock_);
    Node **link = &c.owner.head_;
    while (*link && *link != c.node)
    {
      link = &(*link)->next;
    }
    if (!*link)
    {
      detail::storage_fail("process store: unknown descriptor");
    }
    *link = c.node->next;
    --c.owner.count_;
    // A reader may own the process lock after dropping the registry lock.
    lock(c.process);
  }

  static void destroy(Context &c) noexcept
  {
    unlock(c.process);
    ProcessStore::destroy(c.process);
    c.node->~Node();
  }

  static void free_stack(Context &c) noexcept
  {
    detail::free_kernel_stack(c.stack);
  }

  static void free_page(Context &c) noexcept
  {
    detail::free_process_page(c.node);
  }
  struct Retire;
  struct Remove;
  struct Destroy;
  struct Stack;
  struct Page;
  using Graph = flow::graph<
      Retire, flow::types<flow::node<Retire, flow::action<&retire>, flow::go<Remove>>,
                          flow::node<Remove, flow::action<&remove>, flow::go<Destroy>>,
                          flow::node<Destroy, flow::action<&destroy>, flow::go<Stack>>,
                          flow::node<Stack, flow::action<&free_stack>, flow::go<Page>>,
                          flow::node<Page, flow::action<&free_page>, flow::stop>>>;
};

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
  AllocateFlow::Context context{*this};
  (void)AllocateFlow::Graph::run<0>(context);
  return context.process;
}

void PagedProcessStore::release(Process *process)
{
  auto *node = node_at(*process);
  auto stack = process->kernel_stack();
  ReleaseFlow::Context context{*this, *process, node, stack};
  (void)ReleaseFlow::Graph::run<0>(context);
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
