#pragma once
#include "kernel/fs/ext3/volume.h"
#include "server.h"

namespace userfs
{
// All directory, allocation, metadata, orphan and journal policy runs here in
// userspace. The kernel sees object IDs and framed requests, not ext3 structures.
class Ext3Backend final : public Backend
{
public:

  explicit Ext3Backend(xv::ext3::Io &io) : volume_(io), io_(io) {}
  ~Ext3Backend();
  bool initialize();
  xv::ext3::Error error() const { return volume_.error(); }
  Attributes root() override;
  int lookup(unsigned, const char *, Attributes &) override;
  int getattr(unsigned, Attributes &) override;
  int readdir(unsigned, unsigned long, Entry &) override;
  int read(unsigned, unsigned long, void *, unsigned) override;
  int write(unsigned, unsigned long, const void *, unsigned) override;
  int create(unsigned, const char *, const Attributes &, Attributes &) override;
  int resize(unsigned, unsigned) override;
  int setattr(unsigned, const Attributes &) override;
  int link(unsigned, const char *, unsigned) override;
  int unlink(unsigned, const char *, Attributes &) override;
  int rename(unsigned, const char *, unsigned, const char *) override;
  int sync(unsigned) override;
  void forget(unsigned) override;

private:

  static unsigned number(unsigned node) { return node == 1 ? 2 : node; }
  static unsigned identity(unsigned inode) { return inode == 2 ? 1 : inode; }
  static bool attributes(const xv::ext3::Inode &, Attributes &);
  bool inode(unsigned, xv::ext3::Inode &);
  bool borrow(unsigned);
  bool retained(unsigned) const;
  bool commit();
  bool collect(unsigned);
  xv::ext3::Volume volume_;
  xv::ext3::Io &io_;
  unsigned live_[128]{};
  bool ready_ = false, failed_ = false;
};
} // namespace userfs
