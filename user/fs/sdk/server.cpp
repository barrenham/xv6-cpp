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
      int pairs[xv::userfs::TransportLanes][2]{{-1, -1}, {-1, -1}, {-1, -1}};
      unsigned lanes = 0;
      const unsigned mapped_bytes = capacity * xv::userfs::TransportLanes;
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
        if (ftruncate(memory, mapped_bytes) == 0)
        {
          mapping = mmap(nullptr, mapped_bytes, PROT_READ | PROT_WRITE, MAP_SHARED,
                         memory, 0);
        }
        if (mapping != MAP_FAILED)
        {
          while (lanes < xv::userfs::TransportLanes &&
                 socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, pairs[lanes]) ==
                     0)
          {
            ++lanes;
          }
          bool bound = false;
          if (lanes >= 2)
          {
            xv::userfs::TransportBindingV2 binding;
            binding.memory = memory;
            binding.payload = capacity;
            binding.lanes = lanes;
            binding.cache_flags = backend.cache_flags();
            for (unsigned i = 0; i < lanes; ++i)
            {
              binding.sockets[i] = pairs[i][0];
            }
            bound = ioctl(descriptor, xv::userfs::BindTransport,
                          reinterpret_cast<unsigned long>(&binding)) == 0;
          }
          unsigned active = bound ? lanes : 0;
          if (!bound && lanes)
          {
            xv::userfs::TransportBinding binding;
            binding.socket = pairs[0][0];
            binding.memory = memory;
            binding.payload = capacity;
            bound = ioctl(descriptor, xv::userfs::BindTransport,
                          reinterpret_cast<unsigned long>(&binding)) == 0;
            active = bound ? 1 : 0;
          }
          if (bound)
          {
            backend.transport_ = pairs[0][1];
            backend.transport_lanes_ = active;
            for (unsigned i = 1; i < active; ++i)
            {
              backend.extra_transports_[i - 1] = pairs[i][1];
            }
            for (unsigned i = 0; i < active; ++i)
            {
              pairs[i][1] = -1;
            }
            backend.bulk_ = mapping;
            backend.bulk_bytes_ = capacity;
            backend.bulk_mapping_bytes_ = mapped_bytes;
            backend.registered_payload_ = capacity;
            mapping = MAP_FAILED;
          }
        }
        for (unsigned i = 0; i < lanes; ++i)
        {
          close(pairs[i][0]);
          if (pairs[i][1] >= 0)
          {
            close(pairs[i][1]);
          }
        }
        if (mapping != MAP_FAILED)
        {
          munmap(mapping, mapped_bytes);
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
    int channels[xv::userfs::TransportLanes]{-1, -1, -1};
    unsigned lanes = 1;
    void *mapping = nullptr;
    unsigned bytes = 0, payload = DataSize;
    bool owned = false;

    ~TransportCleanup() {
      for (unsigned i = 0; i < lanes; ++i)
      {
        if (owned && channels[i] >= 0)
        {
          close(channels[i]);
        }
      }
      if (mapping)
      {
        munmap(mapping, bytes);
      }
    }
  } transport;

  bool fast = backend.transport_ >= 0;
  transport.owned = fast;
  transport.lanes = fast ? backend.transport_lanes_ : 1;
  transport.channels[0] = fast ? backend.transport_ : descriptor;
  for (unsigned i = 1; i < transport.lanes; ++i)
  {
    transport.channels[i] = backend.extra_transports_[i - 1];
  }
  transport.mapping = backend.bulk_;
  transport.bytes = backend.bulk_mapping_bytes_;
  transport.payload = fast ? backend.bulk_bytes_ : DataSize;
  backend.transport_ = -1;
  backend.bulk_ = nullptr;
  unsigned cursor = 0;
  auto &message = *static_cast<xv::userfs::Message *>(storage);
  memset(&message, 0, sizeof(message));
  for (;;)
  {
    int delay = -1;
    if (backend.maintenance_pending())
    {
      if (!backend.maintain())
      {
        return -1;
      }
      if (backend.maintenance_pending())
      {
        auto requested = backend.maintenance_delay_ms();
        delay = requested > 1000 ? 1000 : int(requested);
      }
    }
    pollfd events[xv::userfs::TransportLanes]{};
    for (unsigned i = 0; i < transport.lanes; ++i)
    {
      events[i] = {transport.channels[i], POLLIN, 0};
    }
    int ready = poll(events, transport.lanes, delay);
    if (ready < 0)
    {
      return -1;
    }
    if (!ready)
    {
      continue;
    }
    unsigned lane = cursor;
    for (unsigned scanned = 0; scanned < transport.lanes; ++scanned)
    {
      lane = (cursor + scanned) % transport.lanes;
      if (events[lane].revents & (POLLIN | POLLHUP | POLLERR))
      {
        break;
      }
    }
    cursor = (lane + 1) % transport.lanes;
    int channel = transport.channels[lane];
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
    void *data = transport.mapping
                     ? static_cast<char *>(transport.mapping) + lane * transport.payload
                     : nullptr;
    dispatch(message, backend, data, transport.payload);
    if (::write(channel, &message, sizeof(message)) != sizeof(message))
    {
      return -1;
    }
  }
}
} // namespace userfs
