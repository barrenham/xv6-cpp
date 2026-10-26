module;
#include <cstring>
module xv.drivers;
namespace
{
xv::drivers::EthernetReceiver *receiver;
unsigned char captured[4096];
unsigned captured_length;
bool available = true, accept = true;
extern "C" void net_test_free(void *page);
class FakeNetwork final : public xv::drivers::EthernetDevice
{
public:
  bool online() const override { return available; }
  bool transmit(void *page, unsigned length) override
  {
    if (!accept) return false;
    std::memcpy(captured, page, length);
    captured_length = length;
    net_test_free(page);
    return true;
  }
} device;
}
namespace xv::drivers
{
EthernetDevice &network_device() { return device; }
void initialize_network(EthernetReceiver &r) { receiver = &r; }
}
extern "C" void net_test_deliver(void *page, unsigned length) { receiver->receive_frame(page, length); }
extern "C" void net_test_capture(void *data, unsigned *length)
{ std::memcpy(data, captured, captured_length); *length = captured_length; }
extern "C" void net_test_device(bool online, bool transmit) { available = online; accept = transmit; }
