export module xv.net:lease;
export import :address_configuration;
export import :dhcp_message;

export namespace xv::net
{
class LeaseConfiguration final
{
public:

  constexpr LeaseConfiguration(InterfaceRegistry &interfaces, RoutingTable &routes,
                               NetworkInterface &loopback)
      : addresses_(interfaces, routes, loopback)
  {
  }

  int install(NetworkInterface &interface, const DhcpLease &lease, bool &changed);
  bool remove(NetworkInterface &interface);

private:

  AddressConfiguration addresses_;
};
} // namespace xv::net
