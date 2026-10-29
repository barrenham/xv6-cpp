module;
#include "kernel/net/errno.h"
module xv.net;
import :registry_flows;
import :registry;

namespace xv::net
{
int InetSocket::bind(Endpoint local)
{
  xv::sync::ScopedLock guard(registry_.lock());
  BindFlow::Context ctx{*this, local};
  (void)BindFlow::Graph::run<0>(ctx);
  return ctx.result;
}

int InetSocket::auto_bind(unsigned destination)
{
  BindFlow::Context ctx{*this, {}, destination};
  (void)BindFlow::AutoGraph::run<0>(ctx);
  return ctx.result;
}

int InetSocket::listen(int)
{
  return -EOPNOTSUPP;
}

Socket *InetSocket::accept(bool, int &error)
{
  error = -EOPNOTSUPP;
  return nullptr;
}

int InetSocket::shutdown(int)
{
  return -EOPNOTSUPP;
}

Endpoint InetSocket::local()
{
  xv::sync::ScopedLock guard(registry_.lock());
  return local_;
}

Endpoint InetSocket::peer()
{
  xv::sync::ScopedLock guard(registry_.lock());
  return peer_;
}

int InetSocket::error()
{
  xv::sync::ScopedLock guard(registry_.lock());
  int value = error_;
  error_ = 0;
  return value;
}

void InetSocket::notify()
{
  detail::wake(this);
}

void InetSocket::wait()
{
  detail::prepare_sleep(this);
  registry_.lock().release();
  detail::sleep();
  registry_.lock().acquire();
}
} // namespace xv::net
