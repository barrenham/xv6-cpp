export module xv.net:interface;
export import :address;
export import :packet;
export import :configuration;
export import :port;

export namespace xv::net
{
class NetworkInterface;
class InterfaceRegistry;

struct NeighborInfo
{
  unsigned address = 0;
  unsigned char mac[6]{};
  unsigned long expires_ms = 0;
};

struct InterfaceStatistics
{
  unsigned long long rx_packets, rx_bytes, rx_dropped, tx_packets, tx_bytes, tx_errors;
};

class IpReceiver
{
public:

  virtual void receive(NetworkInterface &interface, Packet &packet) = 0;

protected:

  ~IpReceiver() = default;
};

class NetworkInterface
{
public:

  constexpr NetworkInterface(unsigned id, const char *name, unsigned mtu)
      : id_(id), name_(name), mtu_(mtu)
  {
  }

  NetworkInterface(const NetworkInterface &) = delete;
  NetworkInterface &operator=(const NetworkInterface &) = delete;

  unsigned id() const
  {
    return id_;
  }

  const char *name() const
  {
    return name_;
  }

  unsigned mtu() const
  {
    return mtu_;
  }

  bool dhcp_active() const
  {
    ConfigurationGuard guard;
    return dhcp_active_;
  }

  bool up() const
  {
    ConfigurationGuard guard;
    return up_;
  }

  unsigned generation() const
  {
    ConfigurationGuard guard;
    return generation_;
  }

  InterfaceStatistics statistics() const;
  void receive_drop();
  int send(Packet &packet, unsigned next_hop);

  virtual bool neighbor(unsigned, NeighborInfo &)
  {
    return false;
  }

  virtual void invalidate()
  {
  }

  virtual bool is_bridge() const
  {
    return false;
  }

  virtual Layer2Port *layer2()
  {
    return nullptr;
  }

  virtual unsigned master() const
  {
    return 0;
  }

  virtual bool is_tunnel() const
  {
    return false;
  }

  virtual bool carrier() const
  {
    return true;
  }

  virtual const MacAddress *mac() const
  {
    return nullptr;
  }

  virtual bool owns(unsigned address) const;
  bool broadcast(unsigned address) const;
  virtual bool accepts_destination(unsigned address,
                                   const InterfaceRegistry &registry) const;
  unsigned source_for(unsigned destination) const;

  unsigned address_count() const
  {
    ConfigurationGuard guard;
    return address_count_;
  }

  Ipv4Address address(unsigned index) const
  {
    ConfigurationGuard guard;
    return index < address_count_ ? addresses_[index] : Ipv4Address{};
  }

  bool add_address(Ipv4Address address);
  bool set_up(bool up);
  void configure_link(bool up);
  void set_dhcp_active(bool active);
  void topology_changed();

  void attach(IpReceiver &receiver)
  {
    receiver_ = &receiver;
  }

  void deliver(Packet &packet);
  int send_bootstrap(Packet &packet);

  virtual int transmit_bootstrap(Packet &)
  {
    return -95;
  }

  virtual int transmit(Packet &packet, unsigned next_hop) = 0;

  virtual void tick(unsigned long)
  {
  }

protected:

  ~NetworkInterface() = default;

private:

  struct DeliverFlow;

  friend class InterfaceRegistry;
  friend class AddressConfiguration;
  int account_send(unsigned length, int result);
  unsigned id_;
  const char *name_;
  unsigned mtu_;
  bool up_ = false, sealed_ = false, dhcp_active_ = false;
  static constexpr unsigned AddressCapacity = 4;
  Ipv4Address addresses_[AddressCapacity]{};
  unsigned address_count_ = 0, generation_ = 0;
  InterfaceStatistics statistics_{};
  IpReceiver *receiver_ = nullptr;
};

class InterfaceRegistry final
{
public:

  bool add(NetworkInterface &interface);
  bool seal();
  int publish(NetworkInterface &interface);
  bool withdraw(NetworkInterface &interface);

  unsigned count() const
  {
    ConfigurationGuard guard;
    return count_;
  }

  NetworkInterface *at(unsigned index) const
  {
    ConfigurationGuard guard;
    return index < count_ ? interfaces_[index] : nullptr;
  }

  NetworkInterface *find(unsigned id) const;
  bool owns(unsigned address) const;
  void tick(unsigned long now);

private:

  struct UpdateFlow;
  struct SealFlow;

  struct RegistrationFlow;

  static constexpr unsigned Capacity = 8;
  int registration_error(const NetworkInterface &interface) const;
  NetworkInterface *interfaces_[Capacity]{};
  unsigned count_ = 0;
  bool sealed_ = false;
};
} // namespace xv::net
