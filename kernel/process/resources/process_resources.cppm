module;

#include "kernel/support/param.h"

export module xv.process:resources;

export import xv.fs;

export namespace xv::process
{
// Each process has one kernel execution thread. Its descriptor table and cwd
// are accessed only by that thread, or before a fork child is published.
class DescriptorTable final
{
public:

  static constexpr int Capacity = NOFILE;
  constexpr DescriptorTable() = default;
  DescriptorTable(const DescriptorTable &) = delete;
  DescriptorTable &operator=(const DescriptorTable &) = delete;
  xv::fs::File *get(int descriptor) const;
  // Takes the reference only on success; failure leaves it with the caller.
  int install(xv::fs::File *owned);
  // Removes and transfers the table's reference; invalid descriptors return null.
  xv::fs::File *take(int descriptor);
  bool close(int descriptor);
  int duplicate(int descriptor);
  void close_all();
  void inherit_from(const DescriptorTable &parent);

private:

  xv::fs::File *entries_[Capacity]{};
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
  // Takes one reference and drops the old one. Caller surrounds possible inode release
  // with a filesystem transaction; this operation may sleep.
  void replace(xv::fs::Inode *owned);
  void clear();
  void inherit_from(const WorkingDirectory &parent);

private:

  xv::fs::Inode *directory_ = nullptr;
};
} // namespace xv::process
