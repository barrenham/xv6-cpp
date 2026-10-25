module;

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"

module xv.process;

extern "C" xv::fs::File *filedup(xv::fs::File *);
extern "C" void fileclose(xv::fs::File *);
extern "C" xv::fs::Inode *idup(xv::fs::Inode *);
extern "C" void iput(xv::fs::Inode *);

namespace xv::process
{
namespace
{
[[noreturn]] void resource_failure(const char *message)
{
  panic(const_cast<char *>(message));
}
} // namespace

xv::fs::File *DescriptorTable::get(int descriptor) const
{
  return descriptor >= 0 && descriptor < Capacity ? entries_[descriptor] : nullptr;
}

int DescriptorTable::install(xv::fs::File *owned)
{
  if (!owned)
  {
    return -1;
  }
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
  auto *owned = get(descriptor);
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
  fileclose(owned);
  return true;
}

int DescriptorTable::duplicate(int descriptor)
{
  auto *borrowed = get(descriptor);
  if (!borrowed)
  {
    return -1;
  }
  auto *reference = filedup(borrowed);
  auto result = install(reference);
  if (result < 0)
  {
    fileclose(reference);
  }
  return result;
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
    resource_failure("descriptor table: self inheritance");
  }
  for (int i = 0; i < Capacity; ++i)
  {
    if (entries_[i])
    {
      resource_failure("descriptor table: child is not empty");
    }
  }
  for (int i = 0; i < Capacity; ++i)
  {
    if (auto *file = parent.get(i))
    {
      entries_[i] = filedup(file);
    }
  }
}

xv::fs::Inode *WorkingDirectory::duplicate() const
{
  return directory_ ? idup(directory_) : nullptr;
}

void WorkingDirectory::replace(xv::fs::Inode *owned)
{
  auto *old = directory_;
  directory_ = owned;
  if (old)
  {
    iput(old);
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
    resource_failure("working directory: child is not empty");
  }
  directory_ = parent.duplicate();
}
} // namespace xv::process
