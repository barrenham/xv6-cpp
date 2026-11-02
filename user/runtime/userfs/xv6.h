#pragma once
#include "server.h"
#include "xv6_volume.h"

namespace userfs
{
class Xv6Backend final : public Backend
{
public:
  explicit Xv6Backend(xv6::Io &io) : volume_(io) {}
  ~Xv6Backend();
  bool initialize();
  xv6::Error error() const { return volume_.error(); }
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
  struct State { unsigned parent = 0; bool borrowed = false, orphan = false; };
  static Attributes attributes(const xv6::Inode &);
  bool borrow(const xv6::Inode &);
  bool retained(unsigned) const;
  bool collect();
  xv6::Volume volume_;
  State *state_ = nullptr;
  bool failed_ = false;
};
} // namespace userfs
