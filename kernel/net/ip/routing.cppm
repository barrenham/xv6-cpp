export module xv.net:routing;
export import :interface;
export namespace xv::net
{
struct RouteSelection { NetworkInterface *interface = nullptr; unsigned next_hop = 0, source = 0; };
class RoutingTable final
{
public:
  constexpr explicit RoutingTable(const InterfaceRegistry &interfaces) : interfaces_(interfaces) {}
  bool add(unsigned network, unsigned prefix, NetworkInterface &interface,
           unsigned gateway = 0, unsigned metric = 0, unsigned source = 0,
           unsigned owner = 0, unsigned local_address = 0);
  void seal() { ConfigurationGuard guard; sealed_ = true; }
  bool lookup(unsigned destination, RouteSelection &result, unsigned source = 0,
              const NetworkInterface *via = nullptr) const;
private:
  friend class NetworkManager;
  friend class BridgeManager;
  friend class LeaseConfiguration;
  struct Route
  {
    unsigned network = 0, prefix = 0, gateway = 0, metric = 0, source = 0;
    NetworkInterface *interface = nullptr;
    unsigned owner = 0, local_address = 0, lease_owner = 0;
  } routes_[32]{};
  unsigned count_ = 0;
  bool sealed_ = false;
  const InterfaceRegistry &interfaces_;
};
}
