module;
module xv.drivers;
import :module_runtime;
import xv.memory;
import xv.runtime;
import xv.print;

namespace xv::drivers::modules
{
void
ResourcePool::initialize() {
  lock_.init("resident driver resources");
}

void *
ResourcePool::allocate() {
  xv::sync::ScopedLock guard(lock_);
  if (count_ == 8192)
  {
    return nullptr;
  }
  if (!free_)
  {
    auto *node = static_cast<Node *>(xv::memory::allocator().allocate());
    if (!node)
    {
      return nullptr;
    }
    xv::runtime::fill(node, 0, 4096);
    node->next = nodes_;
    nodes_ = node;
    for (auto &slot : node->slots)
    {
      slot.next = free_;
      free_ = &slot;
    }
  }
  auto *page = xv::memory::allocator().allocate();
  if (!page)
  {
    return nullptr;
  }
  auto *slot = free_;
  free_ = slot->next;
  slot->address = page;
  slot->next = nullptr;
  ++count_;
  xv::runtime::fill(page, 0, 4096);
  return page;
}

bool
ResourcePool::adopt(void *page) {
  xv::sync::ScopedLock guard(lock_);
  if (!page || count_ == 8192)
  {
    return false;
  }
  if (!free_)
  {
    auto *node = static_cast<Node *>(xv::memory::allocator().allocate());
    if (!node)
    {
      return false;
    }
    xv::runtime::fill(node, 0, 4096);
    node->next = nodes_;
    nodes_ = node;
    for (auto &slot : node->slots)
    {
      slot.next = free_;
      free_ = &slot;
    }
  }
  auto *slot = free_;
  free_ = slot->next;
  slot->address = page;
  slot->next = nullptr;
  ++count_;
  return true;
}

void
ResourcePool::detach(void *page) {
  if (!page)
  {
    return;
  }
  bool found = false;
  {
    xv::sync::ScopedLock guard(lock_);
    for (auto *node = nodes_; node && !found; node = node->next)
    {
      for (auto &slot : node->slots)
      {
        if (slot.address == page)
        {
          slot.address = nullptr;
          slot.next = free_;
          free_ = &slot;
          --count_;
          found = true;
          break;
        }
      }
    }
  }
  if (!found)
  {
    xv::print::panic("driver: freeing an unowned page");
  }
}

void
ResourcePool::release(void *page, bool shared) {
  if (!page)
  {
    return;
  }
  detach(page);
  if (shared)
  {
    xv::memory::allocator().release_page(page);
  }
  else
  {
    xv::memory::allocator().deallocate(page);
  }
}
} // namespace xv::drivers::modules
