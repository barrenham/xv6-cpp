export module xv.drivers:network;
export namespace xv::drivers
{
class EthernetReceiver
{
public:
  virtual void receive_frame(void *page, unsigned length) = 0;
protected:
  ~EthernetReceiver() = default;
};
class EthernetDevice
{
public:
  virtual bool online() const = 0;
  virtual bool carrier() const { return online(); }
  virtual void mac_address(unsigned char *destination) const = 0;
  virtual bool transmit(void *page, unsigned length) = 0;
protected:
  ~EthernetDevice() = default;
};
unsigned network_device_count();
EthernetDevice &network_device(unsigned index = 0);
void initialize_network(EthernetReceiver &receiver, unsigned index = 0);
}
