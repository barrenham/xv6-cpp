module;
#include "kernel/support/types.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/fcntl.h"
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
    unsigned capacity = backend.max_transfer();
    if (capacity > xv::userfs::BulkSize)
    {
      capacity = xv::userfs::BulkSize;
    }
    if (capacity > DataSize && payload == DataSize)
    {
      int pair[2]{-1, -1};
      char name[64];
      static unsigned sequence;
      strcpy(name, "/xvfs-");
      unsigned long identity =
          (static_cast<unsigned long>(getpid()) << 32) | ++sequence;
      unsigned at = 6;
      do
      {
        name[at++] = "0123456789abcdef"[identity & 15];
        identity >>= 4;
      } while (identity);
      name[at] = 0;
      int memory = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
      if (memory >= 0)
      {
        shm_unlink(name);
        void *mapping = MAP_FAILED;
        if (ftruncate(memory, capacity) == 0)
        {
          mapping =
              mmap(nullptr, capacity, PROT_READ | PROT_WRITE, MAP_SHARED, memory, 0);
        }
        if (mapping != MAP_FAILED &&
            socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, pair) == 0)
        {
          xv::userfs::TransportBinding binding;
          binding.socket = pair[0];
          binding.memory = memory;
          binding.payload = capacity;
          if (ioctl(descriptor, xv::userfs::BindTransport,
                    reinterpret_cast<unsigned long>(&binding)) == 0)
          {
            backend.transport_ = pair[1];
            backend.bulk_ = mapping;
            backend.bulk_bytes_ = capacity;
            backend.registered_payload_ = capacity;
            pair[1] = -1;
            mapping = MAP_FAILED;
          }
        }
        if (pair[0] >= 0)
        {
          close(pair[0]);
        }
        if (pair[1] >= 0)
        {
          close(pair[1]);
        }
        if (mapping != MAP_FAILED)
        {
          munmap(mapping, capacity);
        }
        close(memory);
      }
    }
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

  struct TransportCleanup
  {
    int descriptor;
    void *mapping;
    unsigned bytes;

    ~TransportCleanup() {
      if (descriptor >= 0)
      {
        close(descriptor);
      }
      if (mapping)
      {
        munmap(mapping, bytes);
      }
    }
  } transport{backend.transport_, backend.bulk_, backend.bulk_bytes_};

  backend.transport_ = -1;
  backend.bulk_ = nullptr;
  int channel = transport.descriptor >= 0 ? transport.descriptor : descriptor;
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
        pollfd pending{channel, POLLIN, 0};
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
    int count = ::read(channel, &message, sizeof(message));
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
    dispatch(message, backend, transport.mapping,
             transport.bytes ? transport.bytes : DataSize);
    if (::write(channel, &message, sizeof(message)) != sizeof(message))
    {
      return -1;
    }
  }
}
} // namespace userfs
