module;
#include "kernel/net/control.h"
export module xv.net:bridge;
export import :ethernet_interface;
export import :routing;
import xv.drivers;
import xv.sync;

export namespace xv::net
{
class BridgeInterface final : public EthernetInterface,
                              private xv::drivers::EthernetDevice,
                              private FrameReceiver
{
public:

  constexpr BridgeInterface(unsigned id, const char *name)
      : EthernetInterface(id, name),
        bridge_mac_{{2, 0x58, 0x56, 0, 0, static_cast<unsigned char>(id)}}
  {
  }

  void initialize(IpReceiver &ip);

  bool is_bridge() const override
  {
    return true;
  }

  bool carrier() const override;
  void tick(unsigned long now) override;

private:

  struct ForwardFlow;

  friend class BridgeManager;

  unsigned bridge_id() const override
  {
    return id();
  }

  bool online() const override
  {
    ConfigurationGuard guard;
    return active_ && up();
  }

  void mac_address(unsigned char *out) const override;
  using EthernetInterface::transmit;
  bool transmit(void *page, unsigned length) override;
  void ingress(Layer2Port &port, unsigned epoch, Packet &frame) override;
  bool forward(Layer2Port *ingress, unsigned epoch, Packet &frame);

  struct Member
  {
    NetworkInterface *interface = nullptr;
    Layer2Port *port = nullptr;
    unsigned epoch = 0;
  } members_[4]{};

  struct Entry
  {
    MacAddress mac{};
    Member member{};
    unsigned long expires = 0;
  } fdb_[64]{};

  MacAddress bridge_mac_;
  bool active_ = false;
};

class BridgeManager final
{
public:

  constexpr BridgeManager(InterfaceRegistry &interfaces, RoutingTable &routes,
                          IpReceiver &ip)
      : interfaces_(interfaces), routes_(routes), ip_(ip)
  {
  }

  void initialize();
  int execute(unsigned operation, netctl_request &request);

private:

  struct MembershipFlow;
  struct ExecuteFlow;

  BridgeInterface *find(unsigned id);
  bool unused(NetworkInterface &interface) const;
  int membership(BridgeInterface &bridge, NetworkInterface &interface, bool add);
  InterfaceRegistry &interfaces_;
  RoutingTable &routes_;
  IpReceiver &ip_;
  xv::sync::SpinLock admin_lock_;
  BridgeInterface bridges_[2]{{16, "br0"}, {17, "br1"}};
};
} // namespace xv::net
