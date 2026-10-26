module;
#include "kernel/net/control.h"
export module xv.net:control;
import :routing;
import :bridge;
import :dhcp;
export namespace xv::net
{
class NetworkManager final
{
public:
  constexpr NetworkManager(InterfaceRegistry &interfaces, RoutingTable &routes, NetworkInterface &loopback, BridgeManager *bridges = nullptr, DhcpManager *dhcp = nullptr)
    : interfaces_(interfaces), routes_(routes), loopback_(loopback), bridges_(bridges), dhcp_(dhcp) {}
  int execute(unsigned operation, netctl_request &request);
private:
  int inspect(unsigned operation, netctl_request &request);
  int mutate(unsigned operation, const netctl_request &request, NetworkInterface &interface);
  void remove_route(unsigned index);
  InterfaceRegistry &interfaces_;
  RoutingTable &routes_;
  NetworkInterface &loopback_;
  BridgeManager *bridges_;
  DhcpManager *dhcp_;
};
}
