export module xv.net:address_configuration;
export import :routing;

export namespace xv::net
{
class AddressConfiguration final
{
public:

  constexpr AddressConfiguration(InterfaceRegistry &interfaces, RoutingTable &routes,
                                 NetworkInterface &loopback)
      : interfaces_(interfaces), routes_(routes), loopback_(loopback)
  {
  }

  int add_static(NetworkInterface &interface, Ipv4Address address);
  int remove_static(NetworkInterface &interface, Ipv4Address address);
  int install_lease(NetworkInterface &interface, Ipv4Address address, unsigned gateway,
                    unsigned metric, bool &changed);
  bool remove_lease(NetworkInterface &interface);

private:

  InterfaceRegistry &interfaces_;
  RoutingTable &routes_;
  NetworkInterface &loopback_;
};
} // namespace xv::net
