export module xv.net:tunnel;
export import :ipv4;

export namespace xv::net
{
class IpInIpInterface final : public NetworkInterface
{
public:

  constexpr IpInIpInterface(unsigned id, const char *name, unsigned mtu = 1480)
      : NetworkInterface(id, name, mtu)
  {
  }

  bool initialize(Ipv4Layer &ip, NetworkInterface &underlay, unsigned local,
                  unsigned remote);

  bool accepts_destination(unsigned value,
                           const InterfaceRegistry &registry) const override
  {
    return registry.owns(value);
  }

  bool is_tunnel() const override
  {
    return true;
  }

  bool carrier() const override
  {
    return underlay_ && underlay_->up() && underlay_->owns(outer_local_) &&
           underlay_->carrier();
  }

  int transmit(Packet &packet, unsigned next_hop) override;
  bool receive_outer(Packet &packet, const IpDatagram &datagram);

private:

  struct TransmitFlow;
  struct ReceiveFlow;

  Ipv4Layer *ip_ = nullptr;
  NetworkInterface *underlay_ = nullptr;
  unsigned outer_local_ = 0, outer_remote_ = 0;
};

class IpInIpProtocol final : public IpProtocol
{
public:

  bool add(IpInIpInterface &interface);
  void receive(Packet &packet, const IpDatagram &datagram) override;

private:

  struct DispatchFlow;

  IpInIpInterface *tunnels_[8]{};
};

class NetworkSetup
{
public:

  virtual bool configure(InterfaceRegistry &interfaces, RoutingTable &routes,
                         Ipv4Layer &ip, IpInIpProtocol &tunnels) = 0;

protected:

  ~NetworkSetup() = default;
};
} // namespace xv::net
