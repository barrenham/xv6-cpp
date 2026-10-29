module;
#include "kernel/net/errno.h"
#include "kernel/fs/poll.h"
module xv.net;
import :tcp_lifecycle_flow;
import :tcp_io_flow;
import :tcp_sender_flow;
import :tcp_socket;

namespace xv::net
{
TcpSocket::TcpSocket(SocketRegistry &registry) : InetSocket(registry, 6)
{
}

bool TcpSocket::initialize()
{
  return transmit_.initialize() && receive_.initialize();
}

void TcpSocket::fail(int error)
{
  error_ = error;
  state_ = TcpState::Closed;
  send_unacknowledged_ = send_next_;
  syn_pending_ = false;
  transmit_.reset(send_next_);
  notify();
  if (listener_)
  {
    listener_->notify();
  }
}

void TcpSocket::enter_time_wait()
{
  state_ = TcpState::TimeWait;
  idle_deadline_ = detail::milliseconds() + 30000;
  notify();
}

bool TcpSocket::matches(Endpoint local, Endpoint peer) const
{
  return state_ != TcpState::Closed && state_ != TcpState::Listen &&
         local_.port == local.port && local_.address == local.address &&
         peer_.port == peer.port && peer_.address == peer.address;
}

bool TcpSocket::listening(Endpoint local) const
{
  return state_ == TcpState::Listen && local_.port == local.port &&
         (!local_.address || local_.address == local.address);
}

int TcpSocket::connect(Endpoint peer, bool nonblocking)
{
  ConnectFlow::Context ctx{*this, peer, nonblocking};
  (void)ConnectFlow::Graph::run<0>(ctx);
  return ctx.result;
}

int TcpSocket::listen(int backlog)
{
  xv::sync::ScopedLock guard(registry_.lock());
  ListenFlow::Context ctx{*this, backlog};
  (void)ListenFlow::Graph::run<0>(ctx);
  return ctx.result;
}

void TcpSocket::passive(TcpSocket &listener, Endpoint local, Endpoint peer,
                        const unsigned char *syn, unsigned length)
{
  PassiveFlow::Context ctx{*this, listener, local, peer, syn, length};
  (void)PassiveFlow::Graph::run<0>(ctx);
}

Socket *TcpSocket::accept(bool nonblocking, int &error)
{
  registry_.lock().acquire();
  AcceptFlow::Context ctx{*this, nonblocking, error};
  (void)AcceptFlow::Graph::run<0>(ctx);
  return ctx.child;
}

int TcpSocket::read(xv::io::Buffer destination, int count)
{
  ReadFlow::Context ctx{*this, destination, count};
  (void)ReadFlow::Graph::run<0>(ctx);
  return ctx.result;
}

int TcpSocket::write(xv::io::Buffer source, int count)
{
  WriteFlow::Context ctx{*this, source, count};
  (void)WriteFlow::Graph::run<0>(ctx);
  return ctx.result;
}

int TcpSocket::send_to(xv::io::Buffer data, unsigned length, const Endpoint *peer)
{
  return peer ? -EISCONN : write(data, length);
}

int TcpSocket::receive_from(xv::io::Buffer data, unsigned length, Endpoint *peer)
{
  int result = read(data, length);
  if (result >= 0 && peer)
  {
    *peer = this->peer();
  }
  return result;
}

unsigned TcpSocket::poll(xv::sync::PollTable *table)
{
  xv::sync::ScopedLock guard(registry_.lock());
  PollFlow::Context ctx{*this, table};
  (void)PollFlow::Graph::run<0>(ctx);
  return ctx.events;
}

int TcpSocket::shutdown(int how)
{
  ShutdownFlow::Context ctx{*this, how};
  (void)ShutdownFlow::Graph::run<0>(ctx);
  return ctx.result;
}

void TcpSocket::close()
{
  xv::sync::ScopedLock guard(registry_.lock());
  CloseFlow::Context ctx{*this};
  (void)CloseFlow::Graph::run<0>(ctx);
}

void TcpSocket::destroy()
{
  transmit_.release();
  receive_.release();
  this->~TcpSocket();
  detail::free_page(this);
}

void TcpSocket::tick(unsigned long now)
{
  TickFlow::Context ctx{*this, now};
  (void)TickFlow::Graph::run<0>(ctx);
}
} // namespace xv::net
