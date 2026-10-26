module;
#include "kernel/fs/io_status.h"
module xv.net;
import :platform;
import xv.drivers;
import xv.sync;
namespace xv::net
{
namespace
{
constexpr unsigned LocalIp = 0x0a00020f;
constexpr unsigned char LocalMac[6] = {0x52,0x54,0,0x12,0x34,0x56};
constexpr unsigned char GatewayMac[6] = {0x52,0x55,0x0a,0,2,2};
unsigned read16(const unsigned char *p) { return (unsigned(p[0]) << 8) | p[1]; }
unsigned read32(const unsigned char *p) { return (read16(p) << 16) | read16(p+2); }
void write16(unsigned char *p, unsigned n) { p[0] = n >> 8; p[1] = n; }
void write32(unsigned char *p, unsigned n) { write16(p, n >> 16); write16(p+2, n); }
void bytes(void *to, const void *from, unsigned n)
{
  auto *d = static_cast<unsigned char *>(to);
  auto *s = static_cast<const unsigned char *>(from);
  while (n--) *d++ = *s++;
}
bool equal(const unsigned char *a, const unsigned char *b, unsigned n)
{
  while (n--) if (*a++ != *b++) return false;
  return true;
}
unsigned sum(const unsigned char *data, unsigned length, unsigned value = 0)
{
  while (length >= 2) { value += read16(data); data += 2; length -= 2; }
  if (length) value += unsigned(*data) << 8;
  while (value >> 16) value = (value & 0xffff) + (value >> 16);
  return value;
}
void ethernet(unsigned char *p, const unsigned char *destination, unsigned type)
{
  bytes(p, destination, 6); bytes(p+6, LocalMac, 6); write16(p+12, type);
}
struct Message
{
  void *page = nullptr;
  unsigned source = 0;
  unsigned short port = 0, offset = 0, length = 0;
};
struct Port
{
  unsigned short number = 0, references = 0;
  unsigned head = 0, count = 0;
  Message messages[16]{};
};
struct Handle { int owner = 0; Port *port = nullptr; };
constinit xv::sync::SpinLock lock;
constinit Port ports[32];
constinit Handle handles[256];
Handle *find(unsigned short number, int owner)
{
  for (auto &h : handles)
    if (h.owner == owner && h.port->number == number) return &h;
  return nullptr;
}
void detach(Handle &h)
{
  auto &p = *h.port;
  h = {};
  if (--p.references) return;
  while (p.count)
  {
    detail::free_page(p.messages[p.head].page);
    p.messages[p.head] = {};
    p.head = (p.head + 1) % 16;
    --p.count;
  }
  p.number = 0;
  detail::wake(&p);
}
class Stack final : public xv::drivers::EthernetReceiver
{
public:
  void receive_frame(void *page, unsigned length) override;
private:
  void arp(const unsigned char *p, unsigned length);
  bool udp(void *page, unsigned length);
};
constinit Stack stack;
void Stack::arp(const unsigned char *p, unsigned length)
{
  if (length < 42 || read16(p+14) != 1 || read16(p+16) != 0x800 ||
      p[18] != 6 || p[19] != 4 || read16(p+20) != 1 ||
      read32(p+38) != LocalIp || !equal(p+6, p+22, 6)) return;
  auto *reply = static_cast<unsigned char *>(detail::allocate_page());
  if (!reply) return;
  ethernet(reply, p+22, 0x806);
  bytes(reply+14, p+14, 8); write16(reply+20, 2);
  bytes(reply+22, LocalMac, 6); write32(reply+28, LocalIp);
  bytes(reply+32, p+22, 6); bytes(reply+38, p+28, 4);
  if (!xv::drivers::network_device().transmit(reply, 42)) detail::free_page(reply);
}
bool Stack::udp(void *page, unsigned length)
{
  auto *p = static_cast<unsigned char *>(page);
  if (length < 42) return false;
  auto *ip = p+14;
  unsigned header = (ip[0] & 15) * 4;
  unsigned total = read16(ip+2);
  if ((ip[0] >> 4) != 4 || header < 20 || total < header+8 ||
      total > length-14 || !ip[8] || ip[9] != 17 ||
      (read16(ip+6) & 0xbfff) || read32(ip+16) != LocalIp ||
      sum(ip, header) != 0xffff) return false;
  auto *udp = ip+header;
  unsigned size = read16(udp+4);
  if (size < 8 || size != total-header || !read16(udp+2)) return false;
  if (read16(udp+6) && sum(udp, size, sum(ip+12, 8, 17+size)) != 0xffff)
    return false;
  xv::sync::ScopedLock guard(lock);
  for (auto &port : ports)
    if (port.number == read16(udp+2))
    {
      if (port.count == 16) return false;
      port.messages[(port.head+port.count) % 16] =
          {page, read32(ip+12), static_cast<unsigned short>(read16(udp)),
           static_cast<unsigned short>(14+header+8), static_cast<unsigned short>(size-8)};
      ++port.count;
      detail::wake(&port);
      return true;
    }
  return false;
}
void Stack::receive_frame(void *page, unsigned length)
{
  auto *p = static_cast<unsigned char *>(page);
  constexpr unsigned char broadcast[6] = {255,255,255,255,255,255};
  if (length >= 14 && (equal(p, LocalMac, 6) || equal(p, broadcast, 6)))
  {
    auto type = read16(p+12);
    if (type == 0x806) arp(p, length);
    else if (type == 0x800 && udp(page, length)) return;
  }
  detail::free_page(page);
}
}
void initialize()
{
  lock.init("udp ports");
  xv::drivers::initialize_network(stack);
}
int bind(unsigned short number)
{
  if (!number || !xv::drivers::network_device().online()) return -1;
  xv::sync::ScopedLock guard(lock);
  Port *free_port = nullptr;
  Handle *free_handle = nullptr;
  for (auto &p : ports)
  {
    if (p.number == number) return -1;
    if (!p.number) free_port = &p;
  }
  for (auto &h : handles) if (!h.owner) free_handle = &h;
  if (!free_port || !free_handle) return -1;
  free_port->number = number;
  free_port->references = 1;
  *free_handle = {detail::owner(), free_port};
  return 0;
}
int unbind(unsigned short number)
{
  xv::sync::ScopedLock guard(lock);
  auto *h = find(number, detail::owner());
  if (!h) return -1;
  detach(*h);
  return 0;
}
bool inherit(int parent, int child)
{
  xv::sync::ScopedLock guard(lock);
  unsigned needed = 0, available = 0;
  for (auto &h : handles)
  {
    if (h.owner == parent) ++needed;
    if (!h.owner) ++available;
  }
  if (needed > available) return false;
  for (auto &h : handles)
    if (h.owner == parent)
      for (auto &free : handles)
        if (!free.owner)
        {
          free = {child, h.port};
          ++h.port->references;
          break;
        }
  return true;
}
void release_owner(int owner)
{
  xv::sync::ScopedLock guard(lock);
  for (auto &h : handles) if (h.owner == owner) detach(h);
}
int send(unsigned short source, unsigned destination, unsigned short port,
         unsigned long data, unsigned length)
{
  if (!source || !destination || !port || length > 1472 ||
      !xv::drivers::network_device().online()) return -1;
  auto *p = static_cast<unsigned char *>(detail::allocate_page());
  if (!p) return -1;
  ethernet(p, GatewayMac, 0x800);
  auto *ip = p+14;
  for (unsigned i = 0; i < 28; ++i) ip[i] = 0;
  ip[0] = 0x45; ip[8] = 64; ip[9] = 17;
  write16(ip+2, 28+length); write16(ip+6, 0x4000);
  write32(ip+12, LocalIp); write32(ip+16, destination);
  write16(ip+10, ~sum(ip, 20));
  auto *udp = ip+20;
  write16(udp, source); write16(udp+2, port); write16(udp+4, 8+length);
  if (detail::copy_in(udp+8, data, length) < 0) { detail::free_page(p); return -1; }
  auto checksum = static_cast<unsigned short>(~sum(udp, 8+length, sum(ip+12, 8, 17+8+length)));
  write16(udp+6, checksum ? checksum : 0xffff);
  if (!xv::drivers::network_device().transmit(p, 42+length))
  { detail::free_page(p); return -EAGAIN; }
  return 0;
}
int receive(unsigned short number, unsigned long source, unsigned long source_port,
            unsigned long data, unsigned capacity)
{
  lock.acquire();
  for (;;)
  {
    auto *h = find(number, detail::owner());
    if (!h || detail::interrupted()) { lock.release(); return -1; }
    auto &p = *h->port;
    if (p.count)
    {
      auto message = p.messages[p.head];
      p.messages[p.head] = {};
      p.head = (p.head+1) % 16;
      --p.count;
      lock.release();
      unsigned copied = message.length < capacity ? message.length : capacity;
      bool bad = detail::copy_out(source, &message.source, sizeof(message.source)) < 0 ||
                 detail::copy_out(source_port, &message.port, sizeof(message.port)) < 0 ||
                 detail::copy_out(data, static_cast<char *>(message.page)+message.offset, copied) < 0;
      detail::free_page(message.page);
      return bad ? -1 : static_cast<int>(copied);
    }
    detail::prepare_sleep(&p);
    lock.release();
    detail::sleep();
    lock.acquire();
  }
}
}
