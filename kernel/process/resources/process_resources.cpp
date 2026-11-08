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
    xv::fs::File *reference = nullptr;
    int result = -1;
  };

  static void lookup(Context &c) noexcept
  {
    c.reference = c.owner.retain(c.descriptor);
  }

  static bool found(Context &c) noexcept
  {
    return c.reference != nullptr;
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
  struct Install;
  struct Rollback;
  using Graph = flow::graph<
      Lookup, flow::types<flow::node<Lookup, flow::action<&lookup>,
                                     flow::branch<flow::condition<&found>,
                                                  flow::go<Install>, flow::stop>>,
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

xv::fs::File *
DescriptorTable::retain(int descriptor) const {
  xv::sync::ScopedLock guard(lock_);
  auto *file =
      descriptor >= 0 && descriptor < Capacity ? entries_[descriptor] : nullptr;
  return file ? detail::retain_file(file) : nullptr;
}

int
DescriptorTable::install(xv::fs::File *owned, unsigned flags, int minimum,
                         unsigned long *generation) {
  if (!owned || (flags & ~1U) || minimum < 0 || minimum >= Capacity)
  {
    return -1;
  }
  xv::sync::ScopedLock guard(lock_);
  for (int i = minimum; i < Capacity; ++i)
  {
    if (!entries_[i])
    {
      entries_[i] = owned;
      if (!++next_generation_)
      {
        ++next_generation_;
      }
      generations_[i] = next_generation_;
      if (generation)
      {
        *generation = next_generation_;
      }
      if (flags)
      {
        cloexec_ |= 1U << i;
      }
      else
      {
        cloexec_ &= ~(1U << i);
      }
      return i;
    }
  }
  return -1;
}

xv::fs::File *
DescriptorTable::take(int descriptor, unsigned long generation) {
  xv::sync::ScopedLock guard(lock_);
  auto *owned =
      descriptor >= 0 && descriptor < Capacity ? entries_[descriptor] : nullptr;
  if (owned && generation && generations_[descriptor] != generation)
  {
    return nullptr;
  }
  if (owned)
  {
    entries_[descriptor] = nullptr;
    cloexec_ &= ~(1U << descriptor);
  }
  return owned;
}

bool
DescriptorTable::close(int descriptor, unsigned long generation) {
  auto *owned = take(descriptor, generation);
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
  parent.snapshot(entries_, Capacity, &cloexec_);
  for (int i = 0; i < Capacity; ++i)
    if (entries_[i])
    {
      generations_[i] = ++next_generation_;
    }
}

unsigned
DescriptorTable::snapshot(xv::fs::File **references, unsigned capacity,
                          unsigned *flags) const {
  xv::sync::ScopedLock guard(lock_);
  unsigned count = 0;
  if (flags)
  {
    *flags = cloexec_;
  }
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
  xv::sync::ScopedLock guard(lock_);
  return directory_ ? detail::retain_inode(directory_) : nullptr;
}

void WorkingDirectory::replace(xv::fs::Inode *owned)
{
  lock_.acquire();
  auto *old = directory_;
  directory_ = owned;
  lock_.release();
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
  if (this == &parent)
  {
    detail::resource_fail("working directory: self inheritance");
  }
  auto *reference = parent.duplicate();
  lock_.acquire();
  if (directory_)
  {
    detail::resource_fail("working directory: child is not empty");
  }
  directory_ = reference;
  lock_.release();
}
} // namespace xv::process

namespace xv::process
{
int
DescriptorTable::flags(int descriptor) const {
  xv::sync::ScopedLock guard(lock_);
  return descriptor >= 0 && descriptor < Capacity && entries_[descriptor]
             ? int((cloexec_ >> descriptor) & 1U)
             : -1;
}

int
DescriptorTable::flags(int descriptor, unsigned value) {
  xv::sync::ScopedLock guard(lock_);
  if (descriptor < 0 || descriptor >= Capacity || !entries_[descriptor] ||
      (value & ~1U))
  {
    return -1;
  }
  if (value)
  {
    cloexec_ |= 1U << descriptor;
  }
  else
  {
    cloexec_ &= ~(1U << descriptor);
  }
  return 0;
}

void
DescriptorTable::close_on_exec() {
  for (int i = 0; i < Capacity; ++i)
  {
    if (flags(i) == 1)
    {
      close(i);
    }
  }
}
} // namespace xv::process
