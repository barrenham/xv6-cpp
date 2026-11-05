module;
#include "kernel/net/socket/socket.h"
#include "kernel/io/posix_errno.h"
#include "kernel/fs/poll.h"
#include "kernel/process/signals/signal.h"
#include "kernel/support/placement_new.hpp"
module xv.ipc;
import xv.fs;
import xv.process;
import xv.memory;
import xv.runtime;
import xv.arch;
import xv.print;

namespace xv::ipc
{
namespace
{
constexpr unsigned QueueSlots = 16;

struct Packet
{
  Packet *next = nullptr;
  unsigned size = 0, offset = 0;
  Rights rights;
  unsigned char data[MaxPacket];
};

static_assert(sizeof(Packet) <= 4096);

void
destroy(Packet *packet) {
  for (unsigned i = 0; i < packet->rights.count; ++i)
  {
    xv::fs::files().release_message(
        static_cast<xv::fs::File *>(packet->rights.files[i]));
  }
  packet->~Packet();
  xv::memory::allocator().deallocate(packet);
}

struct Queue
{
  Packet *packets[QueueSlots]{};
  unsigned head = 0, size = 0;

  Packet *
  front() const {
    return size ? packets[head] : nullptr;
  }

  void
  push(Packet *packet) {
    packets[(head + size++) % QueueSlots] = packet;
  }

  Packet *
  pop() {
    auto *packet = front();
    if (packet)
    {
      packets[head] = nullptr;
      head = (head + 1) % QueueSlots;
      --size;
    }
    return packet;
  }
};

struct Pair;

class Endpoint final : public xv::io::IoObject, public LocalSocket
{
public:

  Pair *pair = nullptr;
  unsigned side = 0;
  xv::sync::SleepLock reader, writer;
  int read(xv::io::Buffer buffer, int count) override;
  int write(xv::io::Buffer buffer, int count) override;
  unsigned poll(xv::sync::PollTable *) override;
  int stat(xv::io::Metadata &) override;
  void close() override;

  xv::io::Capability *
  capability(xv::io::CapabilityId id) override {
    return id == LocalSocket::Id ? static_cast<LocalSocket *>(this) : nullptr;
  }

  int send(const iovec *, unsigned, bool, unsigned, const Rights &,
           unsigned long) override;
  int receive(const iovec *, unsigned, bool, unsigned, Rights &, unsigned &,
              unsigned long) override;
  int shutdown(int how) override;
  void cancel() override;
  bool connected() const override;
  int type() const override;
  void attach_file(void *) override;
};

struct Pair
{
  Pair *next = nullptr;
  xv::sync::SpinLock lock;
  Endpoint endpoints[2];
  Queue queues[2];
  xv::fs::File *files[2]{};
  bool reading[2]{true, true}, writing[2]{true, true}, closed[2]{};
  bool marked = false;
  int type = SOCK_STREAM;
};

static_assert(sizeof(Pair) <= 4096);
constinit xv::sync::SpinLock registry;
Pair *pairs = nullptr;
unsigned registry_initialized = 0, collecting = 0, collect_again = 0;
unsigned long topology_epoch = 0, collection_generation = 0;
unsigned worker_started = 0;

void
initialize_registry() {
  unsigned expected = 0;
  if (__atomic_compare_exchange_n(&registry_initialized, &expected, 1U, false,
                                  __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
  {
    registry.init("local sockets");
    __atomic_store_n(&registry_initialized, 2U, __ATOMIC_RELEASE);
  }
  while (__atomic_load_n(&registry_initialized, __ATOMIC_ACQUIRE) != 2)
  {
  }
}

bool
interrupted() {
  return xv::process::current()->is_killed();
}

unsigned long
milliseconds() {
  return xv::arch::r_time() / 10000;
}

int
wait(Pair &pair, void *channel, unsigned long deadline, bool interruptible) {
  if (interruptible && interrupted())
  {
    return -EINTR;
  }
  if (deadline && milliseconds() >= deadline)
  {
    return -ETIMEDOUT;
  }
  bool valid;
  {
    auto waiter = xv::process::make_poll_wait();
    waiter.watch(channel);
    if (deadline)
    {
      waiter.set_deadline(deadline);
    }
    valid = !waiter.failed();
    pair.lock.release();
    if (valid)
    {
      waiter.sleep(interruptible);
    }
  }
  pair.lock.acquire();
  return !valid ? -ENOMEM : interruptible && interrupted() ? -EINTR : 0;
}

// Copy across a bounded iovec snapshot. Channel locks never protect a user's
// iovec array, and no user pointer is retained in a queued packet.
int
copy_vectors(const iovec *vectors, unsigned count, unsigned long offset, void *bytes,
             unsigned length, bool user, bool output) {
  auto *data = static_cast<unsigned char *>(bytes);
  unsigned done = 0;
  for (unsigned i = 0; i < count && done < length; ++i)
  {
    if (offset >= vectors[i].iov_len)
    {
      offset -= vectors[i].iov_len;
      continue;
    }
    unsigned long available = vectors[i].iov_len - offset;
    unsigned take = available < length - done ? unsigned(available) : length - done;
    unsigned long address =
        reinterpret_cast<unsigned long>(vectors[i].iov_base) + offset;
    if (address < offset)
    {
      return -EFAULT;
    }
    int result = output ? xv::process::copy_out(user, address, data + done, take)
                        : xv::process::copy_in(data + done, user, address, take);
    if (result < 0)
    {
      return -EFAULT;
    }
    done += take;
    offset = 0;
  }
  return done == length ? 0 : -EINVAL;
}

int
vector_size(const iovec *vectors, unsigned count) {
  if (count > MaxVectors)
  {
    return -EMSGSIZE;
  }
  unsigned long size = 0;
  for (unsigned i = 0; i < count; ++i)
  {
    if (vectors[i].iov_len > 0x7fffffffUL - size)
    {
      return -EMSGSIZE;
    }
    size += vectors[i].iov_len;
  }
  return size;
}

void
Endpoint::attach_file(void *file) {
  xv::sync::ScopedLock global(registry);
  xv::sync::ScopedLock guard(pair->lock);
  pair->files[side] = static_cast<xv::fs::File *>(file);
}

int
Endpoint::type() const {
  return pair->type;
}

bool
Endpoint::connected() const {
  xv::sync::ScopedLock guard(pair->lock);
  return !pair->closed[1 - side];
}

int
Endpoint::stat(xv::io::Metadata &value) {
  value = {};
  value.type = static_cast<short>(xv::io::MetadataKind::Socket);
  value.nlink = 1;
  return 0;
}

int
Endpoint::send(const iovec *vectors, unsigned count, bool user, unsigned flags,
               const Rights &rights, unsigned long deadline) {
  if (flags & ~(MSG_DONTWAIT | MSG_NOSIGNAL | MSG_EOR | (user ? 0 : KernelWait)))
  {
    return -EOPNOTSUPP;
  }
  int total = vector_size(vectors, count);
  bool stream = pair->type == SOCK_STREAM;
  if (total < 0 || rights.count > MaxRights)
  {
    return total < 0 ? total : -EINVAL;
  }
  if (!stream && unsigned(total) > MaxPacket)
  {
    return -EMSGSIZE;
  }
  if (stream && !total)
  {
    return rights.count ? -EINVAL : 0;
  }
  bool interruptible = user || !(flags & KernelWait);
  if (interruptible)
  {
    if (!writer.acquire_interruptible())
    {
      return -EINTR;
    }
  }
  else
  {
    writer.acquire();
  }
  int done = 0, error = 0;
  do
  {
    unsigned take = total - done;
    if (take > MaxPacket)
    {
      take = MaxPacket;
    }
    auto *memory = xv::memory::allocator().allocate();
    if (!memory)
    {
      error = -ENOBUFS;
      break;
    }
    auto *packet = new (memory) Packet;
    packet->size = take;
    error = copy_vectors(vectors, count, done, packet->data, take, user, false);
    if (error < 0)
    {
      destroy(packet);
      break;
    }
    if (!done)
    {
      for (unsigned i = 0; i < rights.count; ++i)
      {
        packet->rights.files[packet->rights.count++] = xv::fs::files().retain_message(
            static_cast<xv::fs::File *>(rights.files[i]));
      }
    }
    auto &owner = *pair;
    auto &queue = owner.queues[1 - side];
    owner.lock.acquire();
    for (;;)
    {
      if (!owner.writing[side] || !owner.reading[1 - side])
      {
        error = -EPIPE;
        break;
      }
      if (queue.size != QueueSlots)
      {
        break;
      }
      if (flags & MSG_DONTWAIT)
      {
        error = -EAGAIN;
        break;
      }
      error = wait(owner, &queue, deadline, interruptible);
      if (error < 0)
      {
        break;
      }
    }
    if (error >= 0)
    {
      queue.push(packet);
      xv::process::wakeup(&queue);
      done += take;
    }
    owner.lock.release();
    if (error < 0)
    {
      destroy(packet);
      break;
    }
  } while (done < total);
  writer.release();
  // SIGPIPE is posted after dropping every channel lock.
  if (!done && error == -EPIPE && pair->type != SOCK_DGRAM && !(flags & MSG_NOSIGNAL))
  {
    xv::process::send_signal(xv::process::current()->id(), SIGPIPE);
  }
  return done ? done : error;
}

int
Endpoint::receive(const iovec *vectors, unsigned count, bool user, unsigned flags,
                  Rights &rights, unsigned &output_flags, unsigned long deadline) {
  if (flags & ~(MSG_DONTWAIT | MSG_PEEK | MSG_TRUNC | MSG_WAITALL | MSG_CMSG_CLOEXEC |
                (user ? 0 : KernelWait)))
  {
    return -EOPNOTSUPP;
  }
  int capacity = vector_size(vectors, count);
  if (capacity < 0)
  {
    return capacity;
  }
  bool stream = pair->type == SOCK_STREAM;
  if (stream && !capacity)
  {
    return 0;
  }
  bool interruptible = user || !(flags & KernelWait);
  if (interruptible)
  {
    if (!reader.acquire_interruptible())
    {
      return -EINTR;
    }
  }
  else
  {
    reader.acquire();
  }
  auto &owner = *pair;
  auto &queue = owner.queues[side];
  int done = 0, error = 0;
  unsigned original_size = 0;
  Packet *garbage = nullptr;
  Rights consumed_rights;
  output_flags = 0;
  owner.lock.acquire();
  for (;;)
  {
    auto *packet = queue.front();
    if (!owner.reading[side])
    {
      break;
    }
    if (!packet)
    {
      if (!owner.writing[1 - side])
      {
        break;
      }
      if (done && !(flags & MSG_WAITALL))
      {
        break;
      }
      if (flags & MSG_DONTWAIT)
      {
        error = -EAGAIN;
        break;
      }
      error = wait(owner, &queue, deadline, interruptible);
      if (error < 0)
      {
        break;
      }
      continue;
    }
    unsigned available = packet->size - packet->offset;
    unsigned take = capacity - done;
    if (take > available)
    {
      take = available;
    }
    error = copy_vectors(vectors, count, done, packet->data + packet->offset, take,
                         user, true);
    if (error < 0)
    {
      break;
    }
    // Ancillary handles acquire ordinary references before leaving the queue.
    for (unsigned i = 0; i < packet->rights.count; ++i)
    {
      if (rights.count == MaxRights)
      {
        output_flags |= MSG_CTRUNC;
        break;
      }
      rights.files[rights.count++] =
          xv::fs::files().retain(static_cast<xv::fs::File *>(packet->rights.files[i]));
    }
    done += take;
    original_size = available;
    if (!stream && take < available)
    {
      output_flags |= MSG_TRUNC;
    }
    if (pair->type == SOCK_SEQPACKET)
    {
      output_flags |= MSG_EOR;
    }
    if (flags & MSG_PEEK)
    {
      break;
    }
    if (stream && take < available)
    {
      packet->offset += take;
      // Rights belong to the first received byte, never to subsequent reads.
      consumed_rights = packet->rights;
      packet->rights.count = 0;
      if (consumed_rights.count)
      {
        __atomic_fetch_add(&topology_epoch, 1UL, __ATOMIC_RELEASE);
      }
      break;
    }
    auto *consumed = queue.pop();
    if (consumed->rights.count)
    {
      __atomic_fetch_add(&topology_epoch, 1UL, __ATOMIC_RELEASE);
    }
    consumed->next = garbage;
    garbage = consumed;
    xv::process::wakeup(&queue);
    if (!stream || done == capacity || rights.count)
    {
      break;
    }
    if (!queue.size && !(flags & MSG_WAITALL))
    {
      break;
    }
  }
  owner.lock.release();
  reader.release();
  for (unsigned i = 0; i < consumed_rights.count; ++i)
  {
    xv::fs::files().release_message(
        static_cast<xv::fs::File *>(consumed_rights.files[i]));
  }
  while (garbage)
  {
    auto *next = garbage->next;
    destroy(garbage);
    garbage = next;
  }
  if (done || (!stream && original_size))
  {
    return !stream && (flags & MSG_TRUNC) ? int(original_size) : done;
  }
  return error;
}

int
Endpoint::read(xv::io::Buffer buffer, int count) {
  if (count < 0)
  {
    return -EINVAL;
  }
  iovec vector{reinterpret_cast<void *>(buffer.address), unsigned(count)};
  Rights discarded;
  unsigned flags = 0;
  int result = receive(&vector, 1, true, buffer.nonblocking ? MSG_DONTWAIT : 0,
                       discarded, flags, 0);
  release_rights(discarded);
  return result;
}

int
Endpoint::write(xv::io::Buffer buffer, int count) {
  if (count < 0)
  {
    return -EINVAL;
  }
  iovec vector{reinterpret_cast<void *>(buffer.address), unsigned(count)};
  return send(&vector, 1, true, buffer.nonblocking ? MSG_DONTWAIT : 0, {}, 0);
}

unsigned
Endpoint::poll(xv::sync::PollTable *table) {
  xv::sync::ScopedLock guard(pair->lock);
  if (table)
  {
    table->watch(&pair->queues[side]);
    table->watch(&pair->queues[1 - side]);
  }
  unsigned events = 0;
  if (pair->queues[side].size || !pair->writing[1 - side] || !pair->reading[side])
  {
    events |= POLLIN;
  }
  if (!pair->writing[side] || !pair->reading[1 - side])
  {
    events |= POLLERR;
  }
  else if (pair->queues[1 - side].size != QueueSlots)
  {
    events |= POLLOUT;
  }
  if (pair->closed[1 - side])
  {
    events |= POLLHUP;
  }
  return events;
}

void
Endpoint::cancel() {
  xv::sync::ScopedLock guard(pair->lock);
  pair->reading[side] = pair->writing[side] = false;
  xv::process::wakeup(&pair->queues[0]);
  xv::process::wakeup(&pair->queues[1]);
}

int
Endpoint::shutdown(int how) {
  if (how < SHUT_RD || how > SHUT_RDWR)
  {
    return -EINVAL;
  }
  pair->lock.acquire();
  if (how != SHUT_WR)
  {
    pair->reading[side] = false;
  }
  if (how != SHUT_RD)
  {
    pair->writing[side] = false;
  }
  xv::process::wakeup(&pair->queues[0]);
  xv::process::wakeup(&pair->queues[1]);
  pair->lock.release();
  if (how != SHUT_WR)
  {
    reader.acquire();
    for (;;)
    {
      pair->lock.acquire();
      auto *packet = pair->queues[side].pop();
      pair->lock.release();
      if (!packet)
      {
        break;
      }
      destroy(packet);
    }
    reader.release();
  }
  return 0;
}

void
Endpoint::close() {
  shutdown(SHUT_RDWR);
  auto *owner = pair;
  registry.acquire();
  owner->lock.acquire();
  owner->closed[side] = true;
  bool last = owner->closed[0] && owner->closed[1];
  if (last)
  {
    auto **link = &pairs;
    while (*link && *link != owner)
    {
      link = &(*link)->next;
    }
    if (*link)
    {
      *link = owner->next;
    }
  }
  owner->lock.release();
  registry.release();
  if (last)
  {
    owner->~Pair();
    xv::memory::allocator().deallocate(owner);
  }
  collect_socket_cycles();
}
} // namespace

void
release_rights(Rights &rights) {
  for (unsigned i = 0; i < rights.count; ++i)
  {
    xv::fs::files().release(static_cast<xv::fs::File *>(rights.files[i]));
  }
  rights.count = 0;
}

bool
socket_pair(int type, xv::io::IoObject *out[2]) {
  if (type != SOCK_STREAM && type != SOCK_DGRAM && type != SOCK_SEQPACKET)
  {
    return false;
  }
  initialize_registry();
  xv::fs::files().set_collect_hook(collect_socket_cycles);
  auto *page = xv::memory::allocator().allocate();
  if (!page)
  {
    return false;
  }
  auto *pair = new (page) Pair;
  pair->type = type;
  pair->lock.init("local socket state");
  for (unsigned i = 0; i < 2; ++i)
  {
    pair->endpoints[i].pair = pair;
    pair->endpoints[i].side = i;
    pair->endpoints[i].reader.init_single_waiter("local receive");
    pair->endpoints[i].writer.init_single_waiter("local send");
    out[i] = &pair->endpoints[i];
  }
  registry.acquire();
  pair->next = pairs;
  pairs = pair;
  registry.release();
  return true;
}

void
request_socket_reclamation() {
  __atomic_add_fetch(&collection_generation, 1UL, __ATOMIC_RELEASE);
  xv::process::wakeup(&collection_generation);
}

namespace
{
void
collection_worker() {
  xv::arch::intr_on();
  unsigned passes = 0;
  for (;;)
  {
    auto wait = xv::process::make_poll_wait();
    wait.watch(&collection_generation);
    if (wait.failed())
    {
      xv::print::panic("socket collector subscription");
    }
    auto generation = __atomic_load_n(&collection_generation, __ATOMIC_ACQUIRE);
    collect_deferred_socket_cycles();
    if (++passes % 8 == 0)
    {
      xv::process::scheduler().yield();
    }
    if (generation == __atomic_load_n(&collection_generation, __ATOMIC_ACQUIRE))
    {
      wait.sleep(false);
    }
  }
}
} // namespace

void
start_socket_collector() {
  unsigned expected = 0;
  if (__atomic_compare_exchange_n(&worker_started, &expected, 1U, false,
                                  __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
  {
    xv::process::create_kernel_task("ipc-gc", collection_worker);
  }
}

void
collect_deferred_socket_cycles() {
  // An inode close can itself retire another socket through a received right.
  // Drain to a fixed point outside FS locks, including on non-returning exit.
  while (reclaim_named_sockets() || __atomic_load_n(&collect_again, __ATOMIC_ACQUIRE))
  {
    if (!xv::fs::can_reclaim_file_objects() ||
        __atomic_load_n(&collecting, __ATOMIC_ACQUIRE))
    {
      return;
    }
    if (__atomic_load_n(&collect_again, __ATOMIC_ACQUIRE))
    {
      collect_socket_cycles();
    }
  }
}

void
collect_socket_cycles() {
  // Sweeping SCM_RIGHTS can close pathname sockets and ordinary file inodes.
  // Defer those callbacks until FS namespace/metadata/RPC locks are released.
  if (!xv::fs::can_reclaim_file_objects())
  {
    __atomic_store_n(&collect_again, 1U, __ATOMIC_RELEASE);
    request_socket_reclamation();
    return;
  }
  if (__atomic_exchange_n(&collecting, 1U, __ATOMIC_ACQ_REL))
  {
    __atomic_store_n(&collect_again, 1U, __ATOMIC_RELEASE);
    request_socket_reclamation();
    return;
  }
  do
  {
    __atomic_store_n(&collect_again, 0U, __ATOMIC_RELEASE);
    Packet *garbage = nullptr;
    registry.acquire();
    for (auto *pair = pairs; pair; pair = pair->next)
    {
      pair->marked = false;
      for (unsigned i = 0; i < 2; ++i)
      {
        if (!pair->closed[i] &&
            (!pair->files[i] || xv::fs::files().external_references(pair->files[i])))
        {
          pair->marked = true;
        }
      }
    }
    bool changed;
    unsigned long epoch;
    do
    {
      epoch = __atomic_load_n(&topology_epoch, __ATOMIC_ACQUIRE);
      changed = false;
      // A receiver can move a right from a queue into a descriptor while the
      // collector traverses another pair. Recheck roots and repeat any pass
      // spanning that ownership transition before reclaiming packets.
      for (auto *candidate = pairs; candidate; candidate = candidate->next)
      {
        if (candidate->marked)
        {
          continue;
        }
        for (unsigned i = 0; i < 2; ++i)
        {
          if (!candidate->closed[i] &&
              (!candidate->files[i] ||
               xv::fs::files().external_references(candidate->files[i])))
          {
            candidate->marked = true;
            changed = true;
          }
        }
      }
      for (auto *pair = pairs; pair; pair = pair->next)
      {
        if (!pair->marked)
        {
          continue;
        }
        xv::sync::ScopedLock guard(pair->lock);
        for (auto &queue : pair->queues)
        {
          for (unsigned i = 0; i < queue.size; ++i)
          {
            auto *packet = queue.packets[(queue.head + i) % QueueSlots];
            for (unsigned j = 0; j < packet->rights.count; ++j)
            {
              auto *file = static_cast<xv::fs::File *>(packet->rights.files[j]);
              for (auto *candidate = pairs; candidate; candidate = candidate->next)
              {
                if (!candidate->marked &&
                    ((!candidate->closed[0] && candidate->files[0] == file) ||
                     (!candidate->closed[1] && candidate->files[1] == file)))
                {
                  candidate->marked = true;
                  changed = true;
                }
              }
            }
          }
        }
      }
    } while (changed || epoch != __atomic_load_n(&topology_epoch, __ATOMIC_ACQUIRE));
    for (auto *pair = pairs; pair; pair = pair->next)
    {
      if (pair->marked)
      {
        continue;
      }
      xv::sync::ScopedLock guard(pair->lock);
      for (auto &queue : pair->queues)
      {
        while (queue.size)
        {
          auto *packet = queue.pop();
          packet->next = garbage;
          garbage = packet;
        }
      }
    }
    registry.release();
    while (garbage)
    {
      auto *next = garbage->next;
      // Packet references are counted separately from descriptor roots.
      for (unsigned i = 0; i < garbage->rights.count; ++i)
      {
        xv::fs::files().release_message(
            static_cast<xv::fs::File *>(garbage->rights.files[i]));
      }
      garbage->rights.count = 0;
      destroy(garbage);
      garbage = next;
    }
  } while (__atomic_load_n(&collect_again, __ATOMIC_ACQUIRE));
  __atomic_store_n(&collecting, 0U, __ATOMIC_RELEASE);
  // A deferred worker may be asleep behind this synchronous collector.
  request_socket_reclamation();
  if (__atomic_load_n(&collect_again, __ATOMIC_ACQUIRE))
  {
    collect_socket_cycles();
  }
}
} // namespace xv::ipc
