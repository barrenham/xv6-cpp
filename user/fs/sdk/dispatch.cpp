module;
#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "user/runtime/user.h"
#include "kernel/fs/userfs/protocol.h"
#include "user/fs/sdk/include/xvfs/control.h"
module xv.userfs;

namespace userfs
{
namespace
{
thread_local xv::userfs::RequestContext request_context;
}

const xv::userfs::RequestContext &
current_context() {
  return request_context;
}
void
dispatch(xv::userfs::Message &m, Backend &backend, void *bulk, unsigned capacity) {
  using xv::userfs::Operation;
  void *data = bulk ? bulk : m.data;
  bool bulk_operation = m.operation == Operation::Read ||
                        m.operation == Operation::Write ||
                        m.operation == Operation::SnapshotRead;
  unsigned limit = bulk_operation ? capacity : DataSize;
  m.result = -1;
  if (m.version != xv::userfs::Version || !m.node || m.count > limit ||
      m.name[NameSize - 1] || m.target[NameSize - 1])
  {
    return;
  }

  struct ContextScope
  {
    xv::userfs::RequestContext previous = request_context;

    ~ContextScope() {
      request_context = previous;
    }
  } scope;

  request_context = m.context;
  backend.context(request_context);
  switch (m.operation)
  {
  case Operation::Control: {
    unsigned input = m.count, capacity = m.attributes.size;
    if (!m.offset || m.offset > 0xffffffffUL || capacity > DataSize)
    {
      break;
    }
    if (!strcmp(m.name, XVFS_DRIVER_INTERFACE))
    {
      if (m.offset != 1 || m.other != XVFS_DRIVER_DESCRIBE || input ||
          capacity < sizeof(xvfs_driver_info))
      {
        break;
      }
      xvfs_driver_info info{};
      info.size = sizeof(info);
      info.version = 1;
      info.wire_version = xv::userfs::Version;
      info.flags = backend.flags();
      info.operations = backend.operations();
      info.max_payload = backend.payload_limit();
      info.interface_version = backend.extension_version();
      const char *name = backend.extension_interface();
      if (!name || strlen(name) >= sizeof(info.interface_name))
      {
        break;
      }
      strcpy(info.interface_name, name);
      memmove(m.data, &info, sizeof(info));
      m.result = sizeof(info);
    }
    else
    {
      m.result = backend.control(m.node, m.name, m.offset, m.other, m.attributes.mode,
                                 m.data, input, m.data, capacity);
    }
    if (m.result > int(capacity))
    {
      m.result = -1;
    }
    m.count = m.result >= 0 ? unsigned(m.result) : 0;
    if (m.result >= 0 && backend.getattr(m.node, m.attributes) < 0)
    {
      m.result = -1;
      m.count = 0;
    }
    break;
  }
  case Operation::SnapshotBegin: {
    unsigned size = 0;
    m.result = backend.snapshot(m.node, size);
    m.attributes.size = size;
    break;
  }
  case Operation::SnapshotRead: {
    unsigned limit = m.count;
    m.result = backend.snapshot_read(m.offset, data, limit);
    if (m.result > int(limit))
    {
      m.result = -1;
    }
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
  case Operation::Create: {
    auto requested = m.attributes;
    m.result = backend.create(m.node, m.name, requested, m.attributes);
    break;
  }
  case Operation::Read: {
    unsigned limit = m.count;
    m.result = backend.read(m.node, m.offset, data, limit);
    if (m.result > int(limit))
    {
      m.result = -1;
    }
    m.count = m.result >= 0 ? unsigned(m.result) : 0;
    break;
  }
  case Operation::Write:
    m.result = backend.write(m.node, m.offset, data, m.count);
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
  case Operation::Readdir: {
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

} // namespace userfs
