export module xv.net:ethernet;
import :packet;
import :address;
import xv.drivers;
namespace xv::net
{
class EthernetProtocol
{
public:
  virtual void receive(Packet &packet) = 0;
protected:
  ~EthernetProtocol() = default;
};
class EthernetLayer final : public xv::drivers::EthernetReceiver
{
public:
  constexpr explicit EthernetLayer(unsigned mtu) : mtu_(mtu) {}
  void initialize(xv::drivers::EthernetDevice &device);
  const MacAddress &mac() const { return mac_; }
  bool carrier() const { return device_ && device_->carrier(); }
  bool online() const { return device_ && device_->online(); }
  int send_ip(Packet &packet, const unsigned char *destination);
  void attach(unsigned type, EthernetProtocol &protocol);
  int send(Packet &packet, const unsigned char *destination, unsigned type);
  void receive_frame(void *page, unsigned length) override;
private:
  struct Binding { unsigned type = 0; EthernetProtocol *protocol = nullptr; } bindings_[4]{};
  xv::drivers::EthernetDevice *device_ = nullptr;
  MacAddress mac_{};
  unsigned mtu_;
};
}
