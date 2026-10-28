module;
#include "kernel/support/placement_new.hpp"
module xv.net;
import :registry;
import :tcp_socket;
import :udp_socket;

namespace xv::net
{
void SocketRegistry::initialize()
{
  lock_.init("internet sockets");
}

bool SocketRegistry::add(InetSocket &socket)
{
  for (auto &slot : sockets_)
  {
    if (slot)
    {
      continue;
    }
    slot = &socket;
    return true;
  }
  return false;
}

void SocketRegistry::destroy(InetSocket &socket)
{
  for (auto &slot : sockets_)
  {
    if (slot != &socket)
    {
      continue;
    }
    slot = nullptr;
    socket.destroy();
    return;
  }
}

Socket *SocketRegistry::create(int type, int protocol)
{
  if ((type != 1 && type != 2) || (protocol && protocol != (type == 1 ? 6 : 17)))
  {
    return nullptr;
  }
  xv::sync::ScopedLock guard(lock_);
  if (type == 1)
  {
    return create_tcp();
  }
  static_assert(sizeof(UdpSocket) <= 4096);
  auto *page = detail::allocate_page();
  if (!page)
  {
    return nullptr;
  }
  auto *socket = new (page) UdpSocket(*this);
  if (add(*socket))
  {
    return socket;
  }
  socket->destroy();
  return nullptr;
}

TcpSocket *SocketRegistry::create_tcp()
{
  static_assert(sizeof(TcpSocket) <= 4096);
  auto *page = detail::allocate_page();
  if (!page)
  {
    return nullptr;
  }
  auto *socket = new (page) TcpSocket(*this);
  if (socket->initialize() && add(*socket))
  {
    return socket;
  }
  socket->destroy();
  return nullptr;
}

bool SocketRegistry::port_used(unsigned protocol, unsigned short port,
                               const InetSocket *except, unsigned local_address) const
{
  if (protocol == 17 && port == 68)
  {
    return true;
  }
  for (auto *socket : sockets_)
  {
    if (socket && socket != except && socket->protocol() == protocol &&
        socket->bound().port == port &&
        (!local_address || !socket->bound().address ||
         socket->bound().address == local_address))
    {
      return true;
    }
  }
  return false;
}

unsigned short SocketRegistry::ephemeral(unsigned protocol)
{
  for (unsigned tries = 0; tries < 16384; ++tries)
  {
    unsigned short candidate = next_ephemeral_++;
    if (next_ephemeral_ < 49152)
    {
      next_ephemeral_ = 49152;
    }
    if (!port_used(protocol, candidate))
    {
      return candidate;
    }
  }
  return 0;
}

unsigned SocketRegistry::initial_sequence()
{
  sequence_counter_ += 64001U;
  return sequence_counter_ + detail::milliseconds() * 250;
}

bool SocketRegistry::deliver(Packet &packet, Endpoint source, Endpoint destination,
                             unsigned offset, unsigned length)
{
  xv::sync::ScopedLock guard(lock_);
  for (auto *socket : sockets_)
  {
    if (socket && socket->protocol() == 17 &&
        static_cast<UdpSocket *>(socket)->deliver(packet, source, destination, offset,
                                                  length))
    {
      return true;
    }
  }
  return false;
}

bool SocketRegistry::deliver_segment(const TcpSegment &segment)
{
  xv::sync::ScopedLock guard(lock_);
  TcpSocket *listener = nullptr;
  for (auto *socket : sockets_)
  {
    if (!socket || socket->protocol() != 6)
    {
      continue;
    }
    auto *tcp = static_cast<TcpSocket *>(socket);
    if (tcp->matches(segment.destination, segment.source))
    {
      tcp->incoming(segment.source, segment.destination, segment.data, segment.length);
      return true;
    }
    if (tcp->listening(segment.destination))
    {
      listener = tcp;
    }
  }
  if (!listener || (segment.data[13] & 0x17) != 2)
  {
    return false;
  }
  if (child_count(*listener) >= listener->backlog_)
  {
    return true;
  }
  if (auto *child = create_tcp())
  {
    child->passive(*listener, segment.destination, segment.source, segment.data,
                   segment.length);
  }
  return true;
}

unsigned SocketRegistry::child_count(const TcpSocket &listener) const
{
  unsigned count = 0;
  for (auto *socket : sockets_)
  {
    if (socket && socket->protocol() == 6 &&
        static_cast<TcpSocket *>(socket)->listener_ == &listener)
    {
      ++count;
    }
  }
  return count;
}

TcpSocket *SocketRegistry::ready_child(const TcpSocket &listener) const
{
  for (auto *socket : sockets_)
  {
    if (!socket || socket->protocol() != 6)
    {
      continue;
    }
    auto *child = static_cast<TcpSocket *>(socket);
    if (child->listener_ == &listener && (child->state_ == TcpState::Established ||
                                          child->state_ == TcpState::CloseWait))
    {
      return child;
    }
  }
  return nullptr;
}

bool SocketRegistry::has_ready_child(const TcpSocket &listener) const
{
  return ready_child(listener) != nullptr;
}

TcpSocket *SocketRegistry::accept_child(TcpSocket &listener)
{
  auto *child = ready_child(listener);
  if (!child)
  {
    return nullptr;
  }
  child->owned_ = true;
  child->listener_ = nullptr;
  return child;
}

void SocketRegistry::close_children(TcpSocket &listener)
{
  for (auto *socket : sockets_)
  {
    if (!socket || socket->protocol() != 6)
    {
      continue;
    }
    auto *child = static_cast<TcpSocket *>(socket);
    if (child->listener_ != &listener)
    {
      continue;
    }
    child->emit(20);
    destroy(*child);
  }
}

void SocketRegistry::tick(unsigned long now)
{
  xv::sync::ScopedLock guard(lock_);
  for (auto *socket : sockets_)
  {
    if (socket)
    {
      socket->tick(now);
    }
  }
}

Socket *create_socket(int type, int protocol)
{
  return socket_registry().create(type, protocol);
}
} // namespace xv::net
