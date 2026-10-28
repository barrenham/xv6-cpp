export module xv.net:routing;
export import :interface;

export namespace xv::net
{
struct RouteSelection
{
  NetworkInterface *interface = nullptr;
  unsigned next_hop = 0, source = 0;
};

class RoutingTable final
{
public:

  struct Route
  {
    unsigned network = 0, prefix = 0, gateway = 0, metric = 0, source = 0;
    NetworkInterface *interface = nullptr;
    unsigned address_owner = 0, local_address = 0, lease_owner = 0;
  };

  constexpr explicit RoutingTable(const InterfaceRegistry &interfaces)
      : interfaces_(interfaces)
  {
  }

  bool add(unsigned network, unsigned prefix, NetworkInterface &interface,
           unsigned gateway = 0, unsigned metric = 0, unsigned source = 0,
           unsigned owner = 0, unsigned local_address = 0);

  void seal()
  {
    ConfigurationGuard guard;
    sealed_ = true;
  }

  bool lookup(unsigned destination, RouteSelection &result, unsigned source = 0,
              const NetworkInterface *via = nullptr) const;
  bool at(unsigned index, Route &result) const;

  unsigned available() const
  {
    ConfigurationGuard guard;
    return Capacity - count_;
  }

  bool references(const NetworkInterface &interface) const;
  int add_static(unsigned network, unsigned prefix, NetworkInterface &interface,
                 unsigned gateway = 0, unsigned metric = 0, unsigned source = 0);
  int remove_static(unsigned network, unsigned prefix, NetworkInterface &interface,
                    unsigned gateway = 0, unsigned metric = 0, unsigned source = 0);
  bool add_address_routes(NetworkInterface &interface, NetworkInterface &loopback,
                          Ipv4Address address);
  void remove_address_routes(const NetworkInterface &interface, unsigned address);
  bool lease_routes_match(NetworkInterface &interface, NetworkInterface &loopback,
                          Ipv4Address address, unsigned gateway, unsigned metric) const;
  bool replace_lease_routes(NetworkInterface &interface, NetworkInterface &loopback,
                            Ipv4Address address, unsigned gateway, unsigned metric);
  bool remove_lease_routes(const NetworkInterface &interface);

private:

  static constexpr unsigned Capacity = 32;

  struct RouteSet
  {
    Route entries[3];
    unsigned count;
  };

  static RouteSet address_routes(NetworkInterface &interface,
                                 NetworkInterface &loopback, Ipv4Address address,
                                 unsigned lease_owner = 0, unsigned gateway = 0,
                                 unsigned metric = 0);
  bool valid(const Route &route) const;
  static bool same_path(const Route &left, const Route &right);
  int find(const Route &route) const;
  void erase(unsigned index);
  unsigned lease_route_count(unsigned owner) const;
  Route routes_[Capacity]{};
  unsigned count_ = 0;
  bool sealed_ = false;
  const InterfaceRegistry &interfaces_;
};
} // namespace xv::net
