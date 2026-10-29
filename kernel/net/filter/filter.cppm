module;
#include "kernel/net/filter/engine.h"
export module xv.net:filter;
export import :filter_hook;
import xv.sync;

export namespace xv::net
{
class Firewall final : public PacketFilter
{
public:

  constexpr Firewall() = default;
  void initialize();
  int execute(unsigned operation, firewall_request &request);
  bool accept(FilterHook hook, const unsigned char *data, unsigned size,
              unsigned interface) override;

private:

  struct AcceptFlow;

  filter::Engine engine_;
  xv::sync::SpinLock lock_;
};
} // namespace xv::net
