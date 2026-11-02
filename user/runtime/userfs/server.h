#pragma once
#include "kernel/fs/userfs/protocol.h"

namespace userfs
{
using xv::userfs::Attributes;
using xv::userfs::DataSize;
using xv::userfs::NameSize;

struct Entry
{
  Attributes attributes;
  unsigned long next = 0;
  char name[NameSize]{};
};

// The backend owns filesystem policy and storage. The library owns wire dispatch.
class Backend
{
public:

  virtual void context(const xv::userfs::RequestContext &) {}
  virtual int snapshot(unsigned, unsigned &) { return -1; }
  virtual int snapshot_read(unsigned long, void *, unsigned) { return -1; }
  virtual void snapshot_end() {}

  // Run one bounded batch between requests; no filesystem policy lives in the
  // dispatcher. A backend with pending work must eventually clear this flag.
  virtual bool maintenance_pending() const { return false; }
  virtual bool maintain() { return true; }

  virtual Attributes root()
  {
    return {1, 0, 0, 0, 0755, 1, 2};
  }

  virtual unsigned flags()
  {
    return 0;
  }

  virtual unsigned long operations()
  {
    return xv::userfs::AllOperations;
  }

  virtual int lookup(unsigned, const char *, Attributes &)
  {
    return -1;
  }

  virtual int getattr(unsigned, Attributes &)
  {
    return -1;
  }

  virtual int readdir(unsigned, unsigned long, Entry &)
  {
    return -1;
  }

  virtual int read(unsigned, unsigned long, void *, unsigned)
  {
    return -1;
  }

  virtual int write(unsigned, unsigned long, const void *, unsigned)
  {
    return -1;
  }

  virtual int create(unsigned, const char *, const Attributes &, Attributes &)
  {
    return -1;
  }

  virtual int resize(unsigned, unsigned)
  {
    return -1;
  }

  virtual int setattr(unsigned, const Attributes &)
  {
    return -1;
  }

  virtual int link(unsigned, const char *, unsigned)
  {
    return -1;
  }

  virtual int unlink(unsigned, const char *, Attributes &)
  {
    return -1;
  }

  virtual int rename(unsigned, const char *, unsigned, const char *)
  {
    return -1;
  }

  virtual int sync(unsigned)
  {
    return 0;
  }

  virtual void forget(unsigned)
  {
  }

protected:

  ~Backend() = default;
};

int register_backend(const char *type, Backend &backend, unsigned payload = DataSize);
int serve(int descriptor, Backend &backend);
} // namespace userfs
