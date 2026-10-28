module;
#include "kernel/net/errno.h"
module xv.net;
import :registry;

namespace xv::net
{
int InetSocket::bind(Endpoint local)
{
  xv::sync::ScopedLock guard(registry_.lock());
  if (local_.port || (local.address && !registry_.ip().local(local.address)))
  {
    return -1;
  }
  if (!local.port)
  {
    local.port = registry_.ephemeral(protocol_);
  }
  if (!local.port || registry_.port_used(protocol_, local.port, this, local.address))
  {
    return -EADDRINUSE;
  }
  local_ = local;
  return 0;
}

int InetSocket::auto_bind(unsigned destination)
{
  unsigned source = registry_.ip().source_for(destination, local_.address);
  if (!source)
  {
    return -ENETUNREACH;
  }
  if (!local_.port)
  {
    local_.port = registry_.ephemeral(protocol_);
  }
  if (!local_.port)
  {
    return -EADDRINUSE;
  }
  if (protocol_ == 6)
  {
    local_.address = source;
  }
  return 0;
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
