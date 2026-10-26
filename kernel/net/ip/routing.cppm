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
           unsigned gateway = 0, unsigned metric = 0, unsigned source = 0);
  void seal() { sealed_ = true; }
  bool lookup(unsigned destination, RouteSelection &result, unsigned source = 0,
              const NetworkInterface *via = nullptr) const;
private:
  struct Route
  {
    unsigned network = 0, prefix = 0, gateway = 0, metric = 0, source = 0;
    NetworkInterface *interface = nullptr;
  } routes_[32]{};
  unsigned count_ = 0;
  bool sealed_ = false;
  const InterfaceRegistry &interfaces_;
};
}
