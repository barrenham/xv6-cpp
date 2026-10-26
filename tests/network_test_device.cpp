module;
#include <cstring>
#include <vector>
module xv.drivers;
namespace
{
xv::drivers::EthernetReceiver *receiver;
unsigned char captured[4096];
unsigned captured_length;
bool available = true, accept = true;
std::vector<std::vector<unsigned char>> frames;
extern "C" void net_test_free(void *page);
extern "C" void *net_test_allocate();
class FakeNetwork final : public xv::drivers::EthernetDevice
{
public:
  void mac_address(unsigned char *out) const override
  { std::memcpy(out, "\x52\x54\x00\x12\x34\x56", 6); }
  bool online() const override { return available; }
  bool transmit(void *page, unsigned length) override
  {
    if (!accept) return false;
    std::memcpy(captured, page, length);
    captured_length = length;
    auto *bytes = static_cast<unsigned char *>(page);
    frames.emplace_back(bytes,bytes+length);
    auto *sent = static_cast<unsigned char *>(page);
    bool arp = length == 42 && sent[12] == 8 && sent[13] == 6 && sent[21] == 1;
    unsigned char reply[42];
    if (arp)
    {
      std::memcpy(reply, sent, 42);
      std::memcpy(reply, sent+6, 6);
      std::memcpy(reply+6, "\x52\x55\x0a\x00\x02\x02", 6);
      reply[21] = 2;
      std::memcpy(reply+32, sent+22, 6); std::memcpy(reply+38, sent+28, 4);
      std::memcpy(reply+22, reply+6, 6); std::memcpy(reply+28, sent+38, 4);
    }
    net_test_free(page);
    if (arp)
    {
      auto *response = net_test_allocate(); std::memcpy(response, reply, 42);
      receiver->receive_frame(response, 42);
    }
    return true;
  }
} device;
}
namespace xv::drivers
{
unsigned network_device_count() { return 1; }
EthernetDevice &network_device(unsigned) { return device; }
void initialize_network(EthernetReceiver &r, unsigned) { receiver = &r; }
}
extern "C" void net_test_deliver(void *page, unsigned length) { receiver->receive_frame(page, length); }
extern "C" void net_test_capture(void *data, unsigned *length)
{ std::memcpy(data, captured, captured_length); *length = captured_length; }
extern "C" void net_test_device(bool online, bool transmit) { available = online; accept = transmit; }

extern "C" void net_test_clear_frames() { frames.clear(); }
extern "C" unsigned net_test_frame_count() { return frames.size(); }
extern "C" unsigned net_test_frame(unsigned index, void *out)
{ if (index >= frames.size()) return 0; const auto &frame = frames[index]; std::memcpy(out,frame.data(),frame.size()); return frame.size(); }
