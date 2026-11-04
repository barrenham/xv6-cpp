#include "kmodule.h"

#ifndef XVK_MODULE_NAME
#define XVK_MODULE_NAME "virtio_block"
#endif

namespace
{
using u16 = unsigned short;
using u32 = unsigned;
using u64 = unsigned long;
constexpr unsigned QueueSize = 8;

struct Descriptor
{
  u64 address;
  u32 length;
  u16 flags, next;
};

struct Available
{
  u16 flags, index, ring[QueueSize], event;
};

struct Completion
{
  u32 id, length;
};

struct Used
{
  u16 flags, index;
  Completion ring[QueueSize];
  u16 event;
};

struct Request
{
  u32 type, reserved;
  u64 sector;
};

struct State
{
  const xv_kmod_host *host;
  volatile u32 *registers;
  void *lock;
  Descriptor *descriptors;
  Available *available;
  volatile Used *used;
  Request request;
  volatile unsigned char status;
  bool busy, pending, flush_supported;
  u16 used_index;
};

static_assert(sizeof(State) <= 4096);
static_assert(sizeof(Descriptor) == 16 && sizeof(Request) == 16);

void
barrier() {
  asm volatile("fence iorw, iorw" ::: "memory");
}

u32
read(State &s, unsigned offset) {
  return s.registers[offset / 4];
}

void
write(State &s, unsigned offset, u32 value) {
  s.registers[offset / 4] = value;
  barrier();
}

void
address(State &s, unsigned offset, void *pointer) {
  // The host's DMA pages and block-cache buffers are identity-mapped RAM.
  auto physical = reinterpret_cast<u64>(pointer);
  write(s, offset, physical);
  write(s, offset + 4, physical >> 32);
}

int
submit(State &s, unsigned type, u64 sector, void *buffer, unsigned bytes) {
  auto &h = *s.host;
  h.acquire(s.lock);
  while (s.busy)
  {
    h.wait(s.lock, &s);
  }
  s.busy = true;
  s.request = {type, 0, sector};
  s.status = 0xff;
  s.pending = true;
  s.descriptors[0] = {reinterpret_cast<u64>(&s.request), sizeof(Request), 1,
                      static_cast<u16>(type == 4 ? 2 : 1)};
  s.descriptors[1] = {reinterpret_cast<u64>(buffer), bytes,
                      static_cast<u16>(1 | (type == 0 ? 2 : 0)), 2};
  s.descriptors[2] = {reinterpret_cast<u64>(&s.status), 1, 2, 0};
  s.available->ring[s.available->index % QueueSize] = 0;
  barrier();
  s.available->index++;
  barrier();
  write(s, 0x50, 0);
  while (s.pending)
  {
    h.wait(s.lock, &s.pending);
  }
  auto result = s.status == 0 ? 0 : -1;
  s.busy = false;
  h.wake(&s);
  h.release(s.lock);
  return result;
}

int
transfer(void *context, unsigned writing, u64 sector, void *buffer, unsigned bytes) {
  return submit(*static_cast<State *>(context), writing ? 1 : 0, sector, buffer, bytes);
}

int
flush(void *context) {
  auto &s = *static_cast<State *>(context);
  return s.flush_supported ? submit(s, 4, 0, nullptr, 0) : 0;
}

void
interrupt(void *context) {
  auto &s = *static_cast<State *>(context);
  auto &h = *s.host;
  h.acquire(s.lock);
  write(s, 0x64, read(s, 0x60) & 3);
  barrier();
  while (s.used_index != s.used->index)
  {
    barrier();
    auto id = s.used->ring[s.used_index % QueueSize].id;
    if (id != 0 || !s.pending)
    {
      s.status = 1;
    }
    s.pending = false;
    ++s.used_index;
    h.wake(&s.pending);
  }
  h.release(s.lock);
}

int
bind(const xv_kmod_host *host, const xv_kmod_device *device, xv_kmod_block *out) {
  if (host->abi != XV_KMOD_ABI || host->size != sizeof(*host) ||
      device->bus != XV_KMOD_BUS_VIRTIO_MMIO || device->mmio_size < 4096)
  {
    return -1;
  }
  auto *s = static_cast<State *>(host->page(host->owner));
  if (!s)
  {
    return -1;
  }
  s->host = host;
  s->registers = reinterpret_cast<volatile u32 *>(device->mmio);
  if (read(*s, 0) != 0x74726976 || read(*s, 4) != 2 || read(*s, 8) != 2)
  {
    return -1;
  }
  write(*s, 0x70, 0);
  write(*s, 0x70, 1 | 2);
  write(*s, 0x14, 0);
  auto features = read(*s, 0x10);
  if (features & (1U << 5))
  {
    write(*s, 0x70, 0);
    return -1;
  } // writable disks in v1
  s->flush_supported = features & (1U << 9);
  write(*s, 0x14, 1);
  if (!(read(*s, 0x10) & 1))
  {
    write(*s, 0x70, 0);
    return -1;
  }
  write(*s, 0x24, 0);
  write(*s, 0x20, s->flush_supported ? 1U << 9 : 0);
  write(*s, 0x24, 1);
  write(*s, 0x20, 1); // VIRTIO_F_VERSION_1
  write(*s, 0x70, 1 | 2 | 8);
  write(*s, 0x30, 0);
  if (!(read(*s, 0x70) & 8) || read(*s, 0x34) < QueueSize || read(*s, 0x44))
  {
    write(*s, 0x70, 0);
    return -1;
  }
  s->lock = host->lock(host->owner);
  s->descriptors = static_cast<Descriptor *>(host->page(host->owner));
  s->available = static_cast<Available *>(host->page(host->owner));
  s->used = static_cast<Used *>(host->page(host->owner));
  if (!s->lock || !s->descriptors || !s->available || !s->used)
  {
    write(*s, 0x70, 0);
    return -1;
  }
  write(*s, 0x38, QueueSize);
  address(*s, 0x80, s->descriptors);
  address(*s, 0x90, s->available);
  address(*s, 0xa0, const_cast<Used *>(s->used));
  write(*s, 0x44, 1);
  unsigned generation, low, high;
  do
  {
    generation = read(*s, 0xfc);
    barrier();
    low = read(*s, 0x100);
    high = read(*s, 0x104);
    barrier();
  } while (generation != read(*s, 0xfc));
  u64 sectors = static_cast<u64>(high) << 32 | low;
  if (!sectors)
  {
    write(*s, 0x70, 0);
    return -1;
  }
  *out = {sizeof(*out), 0, s, sectors, transfer, flush, interrupt};
  write(*s, 0x70, 1 | 2 | 8 | 4);
  return 0;
}

const xv_kmod_driver driver{sizeof(driver),  XV_KMOD_ABI, XV_KMOD_BUS_VIRTIO_MMIO, 2,
                            XVK_MODULE_NAME, bind};
} // namespace

extern "C"
{
  [[gnu::visibility("default")]]
  const xv_kmod_driver *
  xv_driver_entry(unsigned abi) {
    return abi == XV_KMOD_ABI ? &driver : nullptr;
  }
}
