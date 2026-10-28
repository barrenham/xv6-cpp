module;
#include "kernel/net/errno.h"
#include "kernel/fs/poll.h"
module xv.net;
import :udp_socket;

namespace xv::net
{
int UdpSocket::connect(Endpoint peer, bool)
{
  if (!peer.address || !peer.port)
  {
    return -1;
  }
  xv::sync::ScopedLock guard(registry_.lock());
  int bound = auto_bind(peer.address);
  if (bound < 0)
  {
    return bound;
  }
  if (!local_.address)
  {
    local_.address = registry_.ip().source_for(peer.address);
  }
  peer_ = peer;
  return 0;
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
  if (length > 4042)
  {
    return -EMSGSIZE;
  }
  auto staged = Packet::allocate(length);
  if (!staged)
  {
    return -ENOBUFS;
  }
  if (detail::copy_in(staged.data(), data.address, length) < 0)
  {
    return -1;
  }
  xv::sync::ScopedLock guard(registry_.lock());
  Endpoint peer = destination ? *destination : peer_;
  if (!peer.address || !peer.port)
  {
    return -ENOTCONN;
  }
  int bound = auto_bind(peer.address);
  if (bound < 0)
  {
    return bound;
  }
  int result = registry_.udp().send(local_, peer, staged.data(), length);
  return result < 0 ? result : static_cast<int>(length);
}

bool UdpSocket::deliver(Packet &packet, Endpoint source, Endpoint destination,
                        unsigned offset, unsigned length)
{
  if (!local_.port || local_.port != destination.port ||
      (local_.address && local_.address != destination.address) ||
      (peer_.port && (peer_.port != source.port || peer_.address != source.address)))
  {
    return false;
  }
  if (count_ < 16)
  {
    messages_[(head_ + count_) % 16] = {
        packet.release(), source, static_cast<unsigned short>(offset),
        static_cast<unsigned short>(length), packet.context()};
    ++count_;
    notify();
  }
  return true;
}

int UdpSocket::receive_from(xv::io::Buffer data, unsigned length, Endpoint *peer)
{
  registry_.lock().acquire();
  for (;;)
  {
    if (count_)
    {
      auto message = messages_[head_];
      messages_[head_] = {};
      head_ = (head_ + 1) % 16;
      --count_;
      registry_.lock().release();
      Packet packet(message.page, message.offset + message.length, message.context);
      auto copied = message.length < length ? message.length : length;
      if (detail::copy_out(data.address, packet.data() + message.offset, copied) < 0)
      {
        return -1;
      }
      if (peer)
      {
        *peer = message.source;
      }
      return copied;
    }
    if (detail::interrupted())
    {
      registry_.lock().release();
      return -1;
    }
    if (data.nonblocking)
    {
      registry_.lock().release();
      return -EAGAIN;
    }
    wait();
  }
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
