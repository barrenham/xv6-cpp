module;

#include "kernel/support/param.h"

export module xv.process:resources;

export import xv.fs;

export namespace xv::process
{
// Descriptor snapshots may observe another process; cwd stays owner-thread managed.
class DescriptorTable final
{
public:

  static constexpr int Capacity = NOFILE;
  constexpr DescriptorTable() = default;
  DescriptorTable(const DescriptorTable &) = delete;
  DescriptorTable &operator=(const DescriptorTable &) = delete;
  xv::fs::File *get(int descriptor) const;
  // Takes the reference only on success; failure leaves it with the caller.
  int install(xv::fs::File *owned, unsigned flags = 0, int minimum = 0);
  int flags(int descriptor) const;
  int flags(int descriptor, unsigned value);
  void close_on_exec();
  // Removes and transfers the table's reference; invalid descriptors return null.
  xv::fs::File *take(int descriptor);
  bool close(int descriptor);
  int duplicate(int descriptor);
  void close_all();
  void inherit_from(const DescriptorTable &parent);
  unsigned snapshot(xv::fs::File **references, unsigned capacity) const;
  unsigned inspect(xv::fs::FileInfo *entries, unsigned capacity) const;

private:

  struct DuplicateFlow;

  mutable xv::sync::SpinLock lock_;
  xv::fs::File *entries_[Capacity]{};
  static_assert(Capacity <= 32);
  unsigned cloexec_ = 0;
};

class WorkingDirectory final
{
public:

  constexpr WorkingDirectory() = default;
  WorkingDirectory(const WorkingDirectory &) = delete;
  WorkingDirectory &operator=(const WorkingDirectory &) = delete;

  xv::fs::Inode *get() const
  {
    return directory_;
  } // borrowed reference

  xv::fs::Inode *duplicate() const;
  // Takes one reference and drops the old one; this operation may sleep.
  void replace(xv::fs::Inode *owned);
  void clear();
  void inherit_from(const WorkingDirectory &parent);

private:

  xv::fs::Inode *directory_ = nullptr;
};
} // namespace xv::process
