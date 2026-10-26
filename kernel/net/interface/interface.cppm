export module xv.net:interface;
export import :address;
export import :packet;
export namespace xv::net
{
class NetworkInterface;
class InterfaceRegistry;
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
      : id_(id), name_(name), mtu_(mtu) {}
  NetworkInterface(const NetworkInterface &) = delete;
  NetworkInterface &operator=(const NetworkInterface &) = delete;
  unsigned id() const { return id_; }
  const char *name() const { return name_; }
  unsigned mtu() const { return mtu_; }
  bool up() const { return up_; }
  virtual bool is_tunnel() const { return false; }
  virtual bool carrier() const { return true; }
  virtual const MacAddress *mac() const { return nullptr; }
  virtual bool owns(unsigned address) const;
  bool broadcast(unsigned address) const;
  virtual bool accepts_destination(unsigned address, const InterfaceRegistry &registry) const;
  unsigned source_for(unsigned destination) const;
  unsigned address_count() const { return address_count_; }
  const Ipv4Address &address(unsigned index) const { return addresses_[index]; }
  bool add_address(Ipv4Address address);
  bool set_up(bool up);
  void attach(IpReceiver &receiver) { receiver_ = &receiver; }
  void deliver(Packet &packet);
  virtual int transmit(Packet &packet, unsigned next_hop) = 0;
  virtual void tick(unsigned long) {}
protected:
  ~NetworkInterface() = default;
private:
  friend class InterfaceRegistry;
  unsigned id_;
  const char *name_;
  unsigned mtu_;
  bool up_ = false, sealed_ = false;
  Ipv4Address addresses_[4]{};
  unsigned address_count_ = 0;
  IpReceiver *receiver_ = nullptr;
};
class InterfaceRegistry final
{
public:
  bool add(NetworkInterface &interface);
  bool seal();
  unsigned count() const { return count_; }
  NetworkInterface *at(unsigned index) const { return index < count_ ? interfaces_[index] : nullptr; }
  NetworkInterface *find(unsigned id) const;
  bool owns(unsigned address) const;
  void tick(unsigned long now);
private:
  NetworkInterface *interfaces_[8]{};
  unsigned count_ = 0;
  bool sealed_ = false;
};
}
