module;
#include "kernel/support/flow.h"

module xv.process;

import :resource_platform;

namespace xv::process
{

namespace flow = xv::flow;

struct DescriptorTable::DuplicateFlow
{
  struct Context
  {
    DescriptorTable &owner;
    int descriptor;
    xv::fs::File *borrowed = nullptr, *reference = nullptr;
    int result = -1;
  };

  static void lookup(Context &c) noexcept
  {
    c.borrowed = c.owner.get(c.descriptor);
  }

  static bool found(Context &c) noexcept
  {
    return c.borrowed != nullptr;
  }

  static void retain(Context &c) noexcept
  {
    c.reference = detail::retain_file(c.borrowed);
  }

  static void install(Context &c) noexcept
  {
    c.result = c.owner.install(c.reference);
  }

  static bool installed(Context &c) noexcept
  {
    return c.result >= 0;
  }

  static void rollback(Context &c) noexcept
  {
    detail::release_file(c.reference);
  }
  struct Lookup;
  struct Retain;
  struct Install;
  struct Rollback;
  using Graph = flow::graph<
      Lookup, flow::types<flow::node<Lookup, flow::action<&lookup>,
                                     flow::branch<flow::condition<&found>,
                                                  flow::go<Retain>, flow::stop>>,
                          flow::node<Retain, flow::action<&retain>, flow::go<Install>>,
                          flow::node<Install, flow::action<&install>,
                                     flow::branch<flow::condition<&installed>,
                                                  flow::stop, flow::go<Rollback>>>,
                          flow::node<Rollback, flow::action<&rollback>, flow::stop>>>;
};

xv::fs::File *DescriptorTable::get(int descriptor) const
{
  xv::sync::ScopedLock guard(lock_);
  return descriptor >= 0 && descriptor < Capacity ? entries_[descriptor] : nullptr;
}

int DescriptorTable::install(xv::fs::File *owned)
{
  if (!owned)
  {
    return -1;
  }
  xv::sync::ScopedLock guard(lock_);
  for (int i = 0; i < Capacity; ++i)
  {
    if (!entries_[i])
    {
      entries_[i] = owned;
      return i;
    }
  }
  return -1;
}

xv::fs::File *DescriptorTable::take(int descriptor)
{
  xv::sync::ScopedLock guard(lock_);
  auto *owned =
      descriptor >= 0 && descriptor < Capacity ? entries_[descriptor] : nullptr;
  if (owned)
  {
    entries_[descriptor] = nullptr;
  }
  return owned;
}

bool DescriptorTable::close(int descriptor)
{
  auto *owned = take(descriptor);
  if (!owned)
  {
    return false;
  }
  detail::release_file(owned);
  return true;
}

int DescriptorTable::duplicate(int descriptor)
{
  DuplicateFlow::Context context{*this, descriptor};
  (void)DuplicateFlow::Graph::run<0>(context);
  return context.result;
}

void DescriptorTable::close_all()
{
  for (int i = 0; i < Capacity; ++i)
  {
    close(i);
  }
}

void DescriptorTable::inherit_from(const DescriptorTable &parent)
{
  if (this == &parent)
  {
    detail::resource_fail("descriptor table: self inheritance");
  }
  for (int i = 0; i < Capacity; ++i)
  {
    if (entries_[i])
    {
      detail::resource_fail("descriptor table: child is not empty");
    }
  }
  parent.snapshot(entries_, Capacity);
}

unsigned DescriptorTable::snapshot(xv::fs::File **references, unsigned capacity) const
{
  xv::sync::ScopedLock guard(lock_);
  unsigned count = 0;
  for (unsigned i = 0; i < capacity; ++i)
  {
    references[i] =
        i < Capacity && entries_[i] ? detail::retain_file(entries_[i]) : nullptr;
    if (references[i])
    {
      ++count;
    }
  }
  return count;
}

unsigned DescriptorTable::inspect(xv::fs::FileInfo *entries, unsigned capacity) const
{
  xv::sync::ScopedLock guard(lock_);
  unsigned count = 0;
  for (unsigned i = 0; i < capacity; ++i)
  {
    entries[i] = {};
    if (i < Capacity && entries_[i] && detail::file_info(entries_[i], entries[i]))
    {
      ++count;
    }
  }
  return count;
}

xv::fs::Inode *WorkingDirectory::duplicate() const
{
  return directory_ ? detail::retain_inode(directory_) : nullptr;
}

void WorkingDirectory::replace(xv::fs::Inode *owned)
{
  auto *old = directory_;
  directory_ = owned;
  if (old)
  {
    detail::release_inode(old);
  }
}

void WorkingDirectory::clear()
{
  replace(nullptr);
}

void WorkingDirectory::inherit_from(const WorkingDirectory &parent)
{
  if (directory_ || this == &parent)
  {
    detail::resource_fail("working directory: child is not empty");
  }
  directory_ = parent.duplicate();
}
} // namespace xv::process
