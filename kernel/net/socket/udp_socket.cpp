module;
#include "kernel/net/errno.h"
#include "kernel/fs/poll.h"
module xv.net;
import :udp_socket_edges;
import :udp_socket_flows;
import :udp_socket;

namespace xv::net
{
int UdpSocket::connect(Endpoint peer, bool)
{
  ConnectFlow::Context ctx{*this, peer};
  (void)ConnectFlow::Graph::run<0>(ctx);
  return ctx.result;
}

int UdpSocket::read(xv::io::Buffer destination, int count)
{
  return count < 0 ? -1 : receive_from(destination, count, nullptr);
}

int UdpSocket::write(xv::io::Buffer source, int count)
{
  return count < 0 ? -1 : send_to(source, count, nullptr);
}

int UdpSocket::send_to(xv::io::Buffer data, unsigned length,
                       const Endpoint *destination)
{
  SendFlow::Context ctx{*this, data, length, destination};
  (void)SendFlow::Graph::run<0>(ctx);
  return ctx.result;
}

bool UdpSocket::deliver(Packet &packet, Endpoint source, Endpoint destination,
                        unsigned offset, unsigned length)
{
  DeliveryFlow::Context ctx{*this, packet, source, destination, offset, length};
  (void)DeliveryFlow::Graph::run<0>(ctx);
  return ctx.result;
}

int UdpSocket::receive_from(xv::io::Buffer data, unsigned length, Endpoint *peer)
{
  registry_.lock().acquire();
  ReceiveFlow::Context ctx{*this, data, length, peer};
  (void)ReceiveFlow::Graph::run<0>(ctx);
  return ctx.result;
}

unsigned UdpSocket::poll(xv::sync::PollTable *table)
{
  xv::sync::ScopedLock guard(registry_.lock());
  if (table)
  {
    table->watch(this);
  }
  return POLLOUT | (count_ ? POLLIN : 0);
}

void UdpSocket::close()
{
  xv::sync::ScopedLock guard(registry_.lock());
  registry_.destroy(*this);
}

void UdpSocket::destroy()
{
  for (auto &message : messages_)
  {
    if (message.page)
    {
      detail::free_page(message.page);
    }
  }
  this->~UdpSocket();
  detail::free_page(this);
}
} // namespace xv::net
