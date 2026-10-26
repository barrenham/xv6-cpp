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
  virtual bool transmit(void *page, unsigned length) = 0;
protected:
  ~EthernetDevice() = default;
};
EthernetDevice &network_device();
void initialize_network(EthernetReceiver &receiver);
}
