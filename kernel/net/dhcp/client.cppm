module;
#include "kernel/net/control.h"
export module xv.net:dhcp;
export import :lease;
export import :ipv4;

export namespace xv::net
{
class DhcpManager final : public Ipv4Bootstrap
{
public:

  constexpr DhcpManager(InterfaceRegistry &interfaces, Ipv4Layer &ip,
                        LeaseConfiguration &configuration)
      : interfaces_(interfaces), ip_(ip), configuration_(configuration)
  {
  }

  void initialize();
  int execute(unsigned operation, netctl_request &request);
  bool receive_bootstrap(NetworkInterface &interface, Packet &packet,
                         const IpDatagram &datagram) override;
  void tick(unsigned long now);

private:

  struct Client
  {
    NetworkInterface *interface = nullptr;
    unsigned state = DHCP_OFF, xid = 0, serial = 0, retries = 0, error = 0;
    MacAddress mac{};
    DhcpLease lease{};
    unsigned long started = 0, next = 0, renewal = 0, rebinding = 0, expires = 0;
  } clients_[8]{};

  Client *find(NetworkInterface &interface);
  void select(Client &client, unsigned long now, unsigned delay = 0);
  int transmit(const Client &snapshot, unsigned type = 0);
  InterfaceRegistry &interfaces_;
  Ipv4Layer &ip_;
  LeaseConfiguration &configuration_;
  unsigned counter_ = 0x58260000;
};
} // namespace xv::net
