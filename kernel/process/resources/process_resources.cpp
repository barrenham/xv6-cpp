module xv.process;

import :resource_platform;

namespace xv::process
{

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
  detail::release_file(owned);
  return true;
}

int DescriptorTable::duplicate(int descriptor)
{
  auto *borrowed = get(descriptor);
  if (!borrowed)
  {
    return -1;
  }
  auto *reference = detail::retain_file(borrowed);
  auto result = install(reference);
  if (result < 0)
  {
    detail::release_file(reference);
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
    detail::resource_fail("descriptor table: self inheritance");
  }
  for (int i = 0; i < Capacity; ++i)
  {
    if (entries_[i])
    {
      detail::resource_fail("descriptor table: child is not empty");
    }
  }
  for (int i = 0; i < Capacity; ++i)
  {
    if (auto *file = parent.get(i))
    {
      entries_[i] = detail::retain_file(file);
    }
  }
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
