module;
#include "kernel/fs/io_status.h"
module xv.net;
import :platform;
import xv.drivers;
import xv.sync;
import :services;
import :registry;
import :udp;
namespace xv::net
{
namespace
{
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
xv::sync::SpinLock &legacy_lock() { return socket_registry().lock(); }
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
class LegacyUdp final : public DatagramSink
{
public:
  bool deliver(Packet &packet, Endpoint source, Endpoint destination, unsigned offset, unsigned length) override
  {
    xv::sync::ScopedLock guard(legacy_lock());
    for (auto &port : ports) if (port.number == destination.port)
    {
      if (port.count < 16)
      {
        port.messages[(port.head+port.count)%16] = {packet.release(), source.address,
            source.port, static_cast<unsigned short>(offset), static_cast<unsigned short>(length)};
        ++port.count; detail::wake(&port);
      }
      return true;
    }
    return false;
  }
};
constinit LegacyUdp legacy;

}
void initialize_legacy_udp() { udp_protocol().attach(legacy); }
bool legacy_port_used(unsigned short port)
{
  for (auto &p : ports) if (p.number == port) return true;
  return false;
}
int bind(unsigned short number)
{
  if (!number || !xv::drivers::network_device().online()) return -1;
  xv::sync::ScopedLock guard(legacy_lock());
  Port *free_port = nullptr;
  Handle *free_handle = nullptr;
  if (socket_registry().port_used(17, number)) return -1;
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
  xv::sync::ScopedLock guard(legacy_lock());
  auto *h = find(number, detail::owner());
  if (!h) return -1;
  detach(*h);
  return 0;
}
bool inherit(int parent, int child)
{
  xv::sync::ScopedLock guard(legacy_lock());
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
  xv::sync::ScopedLock guard(legacy_lock());
  for (auto &h : handles) if (h.owner == owner) detach(h);
}
int send(unsigned short source, unsigned destination, unsigned short port,
         unsigned long data, unsigned length)
{
  if (!source || !destination || !port || length > 1472 ||
      !xv::drivers::network_device().online()) return -1;
  auto packet = Packet::allocate(length);
  if (!packet) return -1;
  if (detail::copy_in(packet.data(), data, length) < 0) return -1;
  int result = udp_protocol().send({0, source},
                                  {destination, port}, packet.data(), length);
  return result < 0 ? (result == -EAGAIN ? result : -1) : 0;
}
int receive(unsigned short number, unsigned long source, unsigned long source_port,
            unsigned long data, unsigned capacity)
{
  legacy_lock().acquire();
  for (;;)
  {
    auto *h = find(number, detail::owner());
    if (!h || detail::interrupted()) { legacy_lock().release(); return -1; }
    auto &p = *h->port;
    if (p.count)
    {
      auto message = p.messages[p.head];
      p.messages[p.head] = {};
      p.head = (p.head+1) % 16;
      --p.count;
      legacy_lock().release();
      unsigned copied = message.length < capacity ? message.length : capacity;
      bool bad = detail::copy_out(source, &message.source, sizeof(message.source)) < 0 ||
                 detail::copy_out(source_port, &message.port, sizeof(message.port)) < 0 ||
                 detail::copy_out(data, static_cast<char *>(message.page)+message.offset, copied) < 0;
      detail::free_page(message.page);
      return bad ? -1 : static_cast<int>(copied);
    }
    detail::prepare_sleep(&p);
    legacy_lock().release();
    detail::sleep();
    legacy_lock().acquire();
  }
}
}
