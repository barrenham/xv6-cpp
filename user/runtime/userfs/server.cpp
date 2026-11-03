#include "kernel/fs/stat.h"
#include "kernel/fs/poll.h"
#include "user/runtime/user.h"
#include "user/runtime/userfs/server.h"

namespace userfs
{
int register_backend(const char *type, Backend &backend, unsigned payload)
{
  if (!type || !*type || strlen(type) >= 32)
  {
    return -1;
  }
  xv::userfs::Registration registration;
  strcpy(registration.type, type);
  registration.root = backend.root();
  registration.flags = backend.flags();
  registration.operations = backend.operations();
  registration.max_payload = payload;
  return fsregister(&registration);
}

namespace
{
void dispatch(xv::userfs::Message &m, Backend &backend)
{
  using xv::userfs::Operation;
  m.result = -1;
  if (m.version != xv::userfs::Version || !m.node || m.count > DataSize ||
      m.name[NameSize - 1] || m.target[NameSize - 1])
  {
    return;
  }
  backend.context(m.context);
  switch (m.operation)
  {
  case Operation::SnapshotBegin:
  {
    unsigned size = 0;
    m.result = backend.snapshot(m.node, size);
    m.attributes.size = size;
    break;
  }
  case Operation::SnapshotRead:
  {
    unsigned limit = m.count;
    m.result = backend.snapshot_read(m.offset, m.data, limit);
    if (m.result > int(limit)) m.result = -1;
    m.count = m.result < 0 ? 0 : unsigned(m.result);
    break;
  }
  case Operation::SnapshotEnd:
    backend.snapshot_end();
    m.result = 0;
    break;
  case Operation::Lookup:
    m.result = backend.lookup(m.node, m.name, m.attributes);
    break;
  case Operation::Getattr:
    m.result = backend.getattr(m.node, m.attributes);
    break;
  case Operation::Create:
  {
    auto requested = m.attributes;
    m.result = backend.create(m.node, m.name, requested, m.attributes);
    break;
  }
  case Operation::Read:
  {
    unsigned limit = m.count;
    m.result = backend.read(m.node, m.offset, m.data, limit);
    if (m.result > int(limit))
    {
      m.result = -1;
    }
    m.count = m.result >= 0 ? unsigned(m.result) : 0;
    break;
  }
  case Operation::Write:
    m.result = backend.write(m.node, m.offset, m.data, m.count);
    if (m.result > int(m.count))
    {
      m.result = -1;
    }
    break;
  case Operation::Resize:
    m.result = backend.resize(m.node, m.attributes.size);
    break;
  case Operation::Setattr:
    m.result = backend.setattr(m.node, m.attributes);
    break;
  case Operation::Link:
    m.result = backend.link(m.node, m.name, m.other);
    break;
  case Operation::Unlink:
    m.result = backend.unlink(m.node, m.name, m.attributes);
    break;
  case Operation::Rename:
    m.result = backend.rename(m.node, m.name, m.other, m.target);
    break;
  case Operation::Sync:
    m.result = backend.sync(m.node);
    break;
  case Operation::Forget:
    backend.forget(m.node);
    m.result = 0;
    break;
  case Operation::Readdir:
  {
    Entry entry;
    m.result = backend.readdir(m.node, m.offset, entry);
    if (m.result == 1)
    {
      m.attributes = entry.attributes;
      m.offset = entry.next;
      memmove(m.name, entry.name, NameSize);
    }
    break;
  }
  }
}
} // namespace

int serve(int descriptor, Backend &backend)
{
  auto *storage = malloc(sizeof(xv::userfs::Message));
  if (!storage) return -1;
  struct Storage
  {
    void *pointer;
    ~Storage() { free(pointer); }
  } allocation{storage};
  auto &message = *static_cast<xv::userfs::Message *>(storage);
  memset(&message, 0, sizeof(message));
  for (;;)
  {
    if (backend.maintenance_pending())
    {
      if (!backend.maintain()) return -1;
      if (backend.maintenance_pending())
      {
        pollfd pending{descriptor, POLLIN, 0};
        unsigned delay = backend.maintenance_delay_ms();
        if (delay > 1000) delay = 1000;
        int ready = poll(&pending, 1, delay);
        if (ready < 0) return -1;
        if (!ready) continue;
      }
    }
    int count = ::read(descriptor, &message, sizeof(message));
    if (!count)
    {
      while (backend.maintenance_pending())
      {
        if (!backend.maintain()) return -1;
        unsigned delay = backend.maintenance_delay_ms();
        if (delay > 1000) delay = 1000;
        if (backend.maintenance_pending() && delay && poll(nullptr, 0, delay) < 0)
          return -1;
      }
      return 0;
    }
    if (count != sizeof(message))
    {
      return -1;
    }
    dispatch(message, backend);
    if (::write(descriptor, &message, sizeof(message)) != sizeof(message))
    {
      return -1;
    }
  }
}
} // namespace userfs
