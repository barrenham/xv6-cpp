module;
#include "kernel/net/filter/decoder.h"
#include "kernel/net/filter/engine.h"
module xv.net;
import :filter_accept_flow;
import :filter;

namespace xv::net
{
void Firewall::initialize()
{
  lock_.init("IPv4 filter");
}

int Firewall::execute(unsigned operation, firewall_request &request)
{
  xv::sync::ScopedLock guard(lock_);
  return engine_.execute(operation, request);
}

bool Firewall::accept(FilterHook hook, const unsigned char *data, unsigned size,
                      unsigned interface)
{
  AcceptFlow::Context ctx{*this, hook, data, size, interface};
  (void)AcceptFlow::Graph::run<0>(ctx);
  return ctx.result;
}
} // namespace xv::net
