module;
#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/poll.h"
#include "user/runtime/user.h"
#include "kernel/fs/userfs/protocol.h"
#include "user/fs/sdk/include/xvfs/control.h"
module xv.userfs;

namespace userfs
{
int
register_backend(const char *type, Backend &backend, unsigned payload) {
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
  int descriptor = fsregister(&registration);
  if (descriptor >= 0)
  {
    backend.registered_payload_ = payload;
  }
  return descriptor;
}

int
serve(int descriptor, Backend &backend) {
  auto *storage = malloc(sizeof(xv::userfs::Message));
  if (!storage)
  {
    return -1;
  }

  struct Storage
  {
    void *pointer;

    ~Storage() {
      free(pointer);
    }
  } allocation{storage};

  auto &message = *static_cast<xv::userfs::Message *>(storage);
  memset(&message, 0, sizeof(message));
  for (;;)
  {
    if (backend.maintenance_pending())
    {
      if (!backend.maintain())
      {
        return -1;
      }
      if (backend.maintenance_pending())
      {
        pollfd pending{descriptor, POLLIN, 0};
        unsigned delay = backend.maintenance_delay_ms();
        if (delay > 1000)
        {
          delay = 1000;
        }
        int ready = poll(&pending, 1, delay);
        if (ready < 0)
        {
          return -1;
        }
        if (!ready)
        {
          continue;
        }
      }
    }
    int count = ::read(descriptor, &message, sizeof(message));
    if (!count)
    {
      while (backend.maintenance_pending())
      {
        if (!backend.maintain())
        {
          return -1;
        }
        unsigned delay = backend.maintenance_delay_ms();
        if (delay > 1000)
        {
          delay = 1000;
        }
        if (backend.maintenance_pending() && delay && poll(nullptr, 0, delay) < 0)
        {
          return -1;
        }
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
