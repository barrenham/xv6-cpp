module;
#include "kernel/support/placement_new.hpp"
module xv.net;
import :registry;
import :tcp_socket;
import :udp_socket;
namespace xv::net
{
void SocketRegistry::initialize() { lock_.init("internet sockets"); }
bool SocketRegistry::add(InetSocket &socket)
{ for (auto &slot : sockets_) if (!slot) { slot = &socket; return true; } return false; }
void SocketRegistry::destroy(InetSocket &socket)
{
  for (auto &slot : sockets_) if (slot == &socket) { slot = nullptr; socket.destroy(); return; }
}
Socket *SocketRegistry::create(int type, int protocol)
{
  if ((type != 1 && type != 2) || (protocol && protocol != (type == 1 ? 6 : 17))) return nullptr;
  xv::sync::ScopedLock guard(lock_);
  if (type == 1) return create_child();
  static_assert(sizeof(UdpSocket) <= 4096);
  auto *page = detail::allocate_page(); if (!page) return nullptr;
  auto *socket = new (page) UdpSocket(*this);
  if (add(*socket)) return socket;
  socket->destroy(); return nullptr;
}
TcpSocket *SocketRegistry::create_child()
{
  static_assert(sizeof(TcpSocket) <= 4096);
  auto *page = detail::allocate_page(); if (!page) return nullptr;
  auto *socket = new (page) TcpSocket(*this);
  if (socket->initialize() && add(*socket)) return socket;
  socket->destroy(); return nullptr;
}
bool SocketRegistry::port_used(unsigned protocol, unsigned short port, const InetSocket *except, unsigned local_address) const
{
  if (protocol==17 && port==68) return true;
  for (auto *s : sockets_) if (s && s != except && s->protocol() == protocol && s->bound().port == port &&
      (!local_address || !s->bound().address || s->bound().address == local_address)) return true;
  return false;
}
unsigned short SocketRegistry::ephemeral(unsigned protocol)
{
  for (unsigned tries = 0; tries < 16384; ++tries)
  {
    unsigned short candidate = next_ephemeral_++;
    if (next_ephemeral_ < 49152) next_ephemeral_ = 49152;
    if (!port_used(protocol, candidate)) return candidate;
  }
  return 0;
}
bool SocketRegistry::deliver(Packet &packet, Endpoint source, Endpoint destination, unsigned offset, unsigned length)
{
  xv::sync::ScopedLock guard(lock_);
  for (auto *s : sockets_)
    if (s && s->protocol() == 17 && static_cast<UdpSocket *>(s)->deliver(packet, source, destination, offset, length)) return true;
  return false;
}
void SocketRegistry::tick(unsigned long now)
{
  xv::sync::ScopedLock guard(lock_);
  for (auto *s : sockets_) if (s) s->tick(now);
}
Socket *create_socket(int type, int protocol) { return socket_registry().create(type, protocol); }
}
