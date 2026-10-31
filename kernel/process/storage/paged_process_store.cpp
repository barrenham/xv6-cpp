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
    int pid;
    bool published = false;
    void *page = nullptr;
    unsigned long stack = 0;
    Node *node = nullptr;
    Process *process = nullptr;
  };

  static void page(Context &c) noexcept
  {
    c.page = detail::allocate_process_object();
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
    detail::free_process_object(c.page);
  }

  static void construct(Context &c) noexcept
  {
    c.node = new (c.page) Node{};
    c.node->owner = &c.owner;
    c.process = ProcessStore::construct(c.node->storage, c.pid, c.node);
    set_stack(*c.process, c.stack);
  }

  static void publish(Context &c) noexcept
  {
    // The unpublished candidate is locked; no registry reader can yet wait on it.
    xv::sync::ScopedLock guard(c.owner.lock_);
    c.node->indexed_pid = c.pid;
    if (c.pid && !c.owner.pids_.insert(*c.node)) return;
    c.node->next = c.owner.head_;
    if (c.owner.head_) c.owner.head_->previous = c.node;
    c.owner.head_ = c.node;
    ++c.owner.count_;
    c.published = true;
  }
  static bool published(Context &c) noexcept { return c.published; }

  static void discard(Context &c) noexcept
  {
    ProcessStore::retire(*c.process);
    ProcessStore::destroy(*c.process);
    c.node->~Node();
    detail::free_kernel_stack(c.stack);
    detail::free_process_object(c.node);
    c.process = nullptr;
  }
  struct Discard;
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
                  flow::node<Publish, flow::action<&publish>,
                             flow::branch<flow::condition<&published>, flow::stop, flow::go<Discard>>>,
                  flow::node<Discard, flow::action<&discard>, flow::stop>>>;
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
    if (c.node->previous) c.node->previous->next = c.node->next;
    else if (c.owner.head_ == c.node) c.owner.head_ = c.node->next;
    else detail::storage_fail("process store: unknown descriptor");
    if (c.node->next) c.node->next->previous = c.node->previous;
    if (c.node->indexed_pid) c.owner.pids_.erase(*c.node);
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
    detail::free_process_object(c.node);
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
  static_assert(sizeof(Node) <= 2048);
  detail::initialize_process_objects(sizeof(Node), alignof(Node));
}

Process *PagedProcessStore::process_at(Node &node)
{
  return __builtin_launder(reinterpret_cast<Process *>(node.storage));
}

PagedProcessStore::Node *PagedProcessStore::node_at(Process &process)
{
  return static_cast<Node *>(storage_node(process));
}

Process *PagedProcessStore::allocate(int pid)
{
  static_assert(sizeof(Node) <= 2048);
  if (pid < 0) return nullptr;
  AllocateFlow::Context context{*this, pid};
  (void)AllocateFlow::Graph::run<0>(context);
  return context.process;
}

void PagedProcessStore::release(Process *process)
{
  auto *node = node_at(*process);
  if (!node || node->owner != this) detail::storage_fail("process store: foreign descriptor");
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

Process *PagedProcessStore::find_pid(int pid)
{
  if (pid <= 0) return nullptr;
  xv::sync::ScopedLock guard(lock_);
  Node key;
  key.indexed_pid = pid;
  auto *node = pids_.find(key);
  if (!node) return nullptr;
  auto *process = process_at(*node);
  lock(*process);
  if (has_pid(*process, pid)) return process;
  unlock(*process);
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
