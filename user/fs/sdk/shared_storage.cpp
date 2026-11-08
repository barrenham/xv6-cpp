module;
#include "user/runtime/user.h"
#include "kernel/fs/fcntl.h"
#include "kernel/support/placement_new.hpp"
module xv.userfs.shared;

namespace userfs
{
namespace
{
constexpr unsigned Chunks = 16, Page = 4096, ChunkBytes = 1024 * 1024;

unsigned
aligned(unsigned value) {
  return (value + 15) & ~15U;
}
} // namespace

struct alignas(16) SharedStorage::Block
{
  unsigned bytes = 0;
  Block *next = nullptr;
};

struct SharedStorage::State
{
  struct Chunk
  {
    char *base = nullptr;
    unsigned committed = Page;
  } chunks[Chunks];

  unsigned held = 0;
  bool failed = false;
  Block *free = nullptr;
};

SharedStorage::SharedStorage() {
  static_assert(sizeof(State) + sizeof(Block) < Page);
  if (socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, queue_) < 0)
  {
    return;
  }
  // Each unlinked object is kept by its mapping and by one capability queued
  // in this socket pair. This avoids sixteen permanent FDs and leaked shm names.
  for (unsigned i = 0; i < Chunks; ++i)
  {
    char name[48];
    strcpy(name, "/xv-shared-");
    unsigned at = 11, pid = getpid();
    do
    {
      name[at++] = "0123456789abcdef"[pid & 15];
      pid >>= 4;
    } while (pid);
    name[at++] = '-';
    name[at++] = "0123456789abcdef"[i];
    name[at] = 0;
    int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd < 0)
    {
      return;
    }
    shm_unlink(name);
    bool okay = ftruncate(fd, Page) == 0;
    void *mapping =
        okay ? mmap(nullptr, ChunkBytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)
             : MAP_FAILED;
    if (mapping != MAP_FAILED)
    {
      mappings_[i] = mapping;
    }
    okay = mapping != MAP_FAILED && give(i, fd);
    close(fd);
    if (!okay)
    {
      return;
    }
  }
  state_ = new (mappings_[0]) State{};
  for (unsigned i = 0; i < Chunks; ++i)
  {
    state_->chunks[i].base = static_cast<char *>(mappings_[i]);
    unsigned reserved = i ? 0 : aligned(sizeof(State));
    auto *block = new (static_cast<char *>(mappings_[i]) + reserved) Block{};
    block->bytes = Page - reserved - sizeof(Block);
    insert(block);
  }
}

SharedStorage::~SharedStorage() {
  for (int fd : queue_)
  {
    if (fd >= 0)
    {
      close(fd);
    }
  }
  state_ = nullptr;
  for (void *mapping : mappings_)
  {
    if (mapping)
    {
      munmap(mapping, ChunkBytes);
    }
  }
}

bool
SharedStorage::give(unsigned index, int fd) {
  unsigned char byte = index;
  iovec vector{&byte, 1};
  alignas(unsigned long) char control[CMSG_SPACE(sizeof(int))]{};
  auto *header = reinterpret_cast<cmsghdr *>(control);
  header->cmsg_level = SOL_SOCKET;
  header->cmsg_type = SCM_RIGHTS;
  header->cmsg_len = CMSG_LEN(sizeof(int));
  memmove(CMSG_DATA(header), &fd, sizeof(fd));
  msghdr message{};
  message.msg_iov = &vector;
  message.msg_iovlen = 1;
  message.msg_control = control;
  message.msg_controllen = sizeof(control);
  return sendmsg(queue_[0], &message, MSG_NOSIGNAL | MSG_DONTWAIT) == 1;
}

int
SharedStorage::borrow(unsigned wanted) {
  for (unsigned n = 0; n < Chunks; ++n)
  {
    unsigned char index = 255;
    iovec vector{&index, 1};
    alignas(unsigned long) char control[CMSG_SPACE(sizeof(int))]{};
    msghdr message{};
    message.msg_iov = &vector;
    message.msg_iovlen = 1;
    message.msg_control = control;
    message.msg_controllen = sizeof(control);
    if (recvmsg(queue_[1], &message, MSG_CMSG_CLOEXEC | MSG_DONTWAIT) != 1 ||
        (message.msg_flags & MSG_CTRUNC))
    {
      return -1;
    }
    auto *header = reinterpret_cast<cmsghdr *>(control);
    if (header->cmsg_level != SOL_SOCKET || header->cmsg_type != SCM_RIGHTS ||
        header->cmsg_len != CMSG_LEN(sizeof(int)))
    {
      return -1;
    }
    int fd = -1;
    memmove(&fd, CMSG_DATA(header), sizeof(fd));
    if (index == wanted)
    {
      return fd;
    }
    bool okay = index < Chunks && give(index, fd);
    close(fd);
    if (!okay)
    {
      return -1;
    }
  }
  return -1;
}

void
SharedStorage::lock() {
  while (__atomic_exchange_n(&state_->held, 1U, __ATOMIC_ACQUIRE))
  {
    while (__atomic_load_n(&state_->held, __ATOMIC_RELAXED))
    {
      sched_yield();
    }
  }
}

void
SharedStorage::unlock() {
  __atomic_store_n(&state_->held, 0U, __ATOMIC_RELEASE);
}

void
SharedStorage::insert(Block *block) {
  auto **link = &state_->free;
  Block *previous = nullptr;
  auto address = reinterpret_cast<unsigned long>(block);
  unsigned chunk = 0;
  while (chunk < Chunks &&
         (address < reinterpret_cast<unsigned long>(state_->chunks[chunk].base) ||
          address >=
              reinterpret_cast<unsigned long>(state_->chunks[chunk].base) + ChunkBytes))
  {
    ++chunk;
  }
  if (chunk == Chunks)
  {
    state_->failed = true;
    return;
  }
  auto low = reinterpret_cast<unsigned long>(state_->chunks[chunk].base);
  while (*link && reinterpret_cast<unsigned long>(*link) < address)
  {
    previous = *link;
    link = &(*link)->next;
  }
  block->next = *link;
  *link = block;
  auto end = address + sizeof(Block) + block->bytes;
  if (block->next && end == reinterpret_cast<unsigned long>(block->next) &&
      end < low + ChunkBytes)
  {
    block->bytes += sizeof(Block) + block->next->bytes;
    block->next = block->next->next;
  }
  if (previous && reinterpret_cast<unsigned long>(previous) >= low &&
      reinterpret_cast<unsigned long>(previous) + sizeof(Block) + previous->bytes ==
          address)
  {
    previous->bytes += sizeof(Block) + block->bytes;
    previous->next = block->next;
  }
}

bool
SharedStorage::grow(unsigned requested) {
  for (unsigned i = 0; i < Chunks; ++i)
  {
    auto &chunk = state_->chunks[i];
    unsigned room = ChunkBytes - chunk.committed;
    unsigned minimum = (requested + sizeof(Block) + Page - 1) & ~(Page - 1);
    if (room < minimum)
    {
      continue;
    }
    unsigned bytes = minimum < 65536 ? 65536 : minimum;
    if (bytes > room)
    {
      bytes = room;
    }
    int fd = borrow(i);
    if (fd < 0)
    {
      state_->failed = true;
      return false;
    }
    bool okay = ftruncate(fd, chunk.committed + bytes) == 0;
    if (!okay && bytes != minimum)
    {
      bytes = minimum;
      okay = ftruncate(fd, chunk.committed + bytes) == 0;
    }
    bool restored = give(i, fd);
    close(fd);
    if (!restored)
    {
      state_->failed = true;
    }
    if (!okay || !restored)
    {
      return false;
    }
    auto *block = new (chunk.base + chunk.committed) Block{};
    block->bytes = bytes - sizeof(Block);
    chunk.committed += bytes;
    insert(block);
    return !state_->failed;
  }
  return false;
}

void *
SharedStorage::allocate(unsigned bytes) {
  if (!state_ || bytes > ChunkBytes - sizeof(Block) - 15)
  {
    return nullptr;
  }
  bytes = aligned(bytes ? bytes : 1);
  lock();
  for (;;)
  {
    if (state_->failed)
    {
      unlock();
      return nullptr;
    }
    auto **link = &state_->free;
    while (*link && (*link)->bytes < bytes)
    {
      link = &(*link)->next;
    }
    if (*link)
    {
      auto *block = *link;
      if (block->bytes >= bytes + sizeof(Block) + 16)
      {
        auto *tail =
            new (reinterpret_cast<char *>(block) + sizeof(Block) + bytes) Block{};
        tail->bytes = block->bytes - bytes - sizeof(Block);
        tail->next = block->next;
        *link = tail;
        block->bytes = bytes;
      }
      else
      {
        *link = block->next;
      }
      block->next = nullptr;
      unlock();
      return reinterpret_cast<char *>(block) + sizeof(Block);
    }
    if (!grow(bytes))
    {
      unlock();
      return nullptr;
    }
  }
}

void
SharedStorage::release(void *pointer) {
  if (!pointer || !state_)
  {
    return;
  }
  lock();
  if (!state_->failed)
  {
    insert(reinterpret_cast<Block *>(static_cast<char *>(pointer) - sizeof(Block)));
  }
  unlock();
}

void
SharedStorage::invalidate() {
  if (!state_)
  {
    return;
  }
  // No worker remains alive. Do not walk a free list whose owner may have
  // died between updates; mapping/descriptor teardown does not need this lock.
  state_->failed = true;
  __atomic_store_n(&state_->held, 0U, __ATOMIC_RELEASE);
}
} // namespace userfs
