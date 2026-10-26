export module xv.net:lease;
export import :routing;
export import :dhcp_message;
export namespace xv::net
{
class LeaseConfiguration final
{
public:
  constexpr LeaseConfiguration(InterfaceRegistry &interfaces, RoutingTable &routes, NetworkInterface &loopback)
    : interfaces_(interfaces), routes_(routes), loopback_(loopback) {}
  int install(NetworkInterface &interface, const DhcpLease &lease, bool &changed);
  bool remove(NetworkInterface &interface);
private:
  InterfaceRegistry &interfaces_;
  RoutingTable &routes_;
  NetworkInterface &loopback_;
};
}
