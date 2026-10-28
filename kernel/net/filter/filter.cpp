module;
#include "kernel/net/filter/decoder.h"
#include "kernel/net/filter/engine.h"
module xv.net;
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
  filter::Engine::BuiltinChain chain;
  unsigned input_interface = 0, output_interface = 0;
  switch (hook)
  {
  case FilterHook::Input:
    chain = filter::Engine::BuiltinChain::Input;
    input_interface = interface;
    break;
  case FilterHook::Output:
    chain = filter::Engine::BuiltinChain::Output;
    output_interface = interface;
    break;
  default:
    return false;
  }
  filter::Metadata packet{};
  if (!filter::decode(data, size, input_interface, output_interface, packet))
  {
    return false;
  }
  xv::sync::ScopedLock guard(lock_);
  return engine_.accept(chain, packet);
}
} // namespace xv::net
