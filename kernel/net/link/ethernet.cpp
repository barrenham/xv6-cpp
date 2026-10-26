module;
#include "kernel/net/errno.h"
module xv.net;
import :ethernet;
import :wire;
namespace xv::net
{
void EthernetLayer::initialize(xv::drivers::EthernetDevice &device)
{ device_ = &device; device.mac_address(mac_.bytes); }
int EthernetLayer::send_ip(Packet &packet, const unsigned char *destination)
{
  if (!packet.prepend(14)) return -EMSGSIZE;
  int result = send(packet, destination, 0x800);
  if (result < 0) packet.remove_prefix(14);
  return result;
}
void EthernetLayer::attach(unsigned type, EthernetProtocol &protocol)
{
  for (auto &b : bindings_) if (!b.protocol) { b = {type, &protocol}; return; }
}
int EthernetLayer::send(Packet &packet, const unsigned char *destination, unsigned type)
{
  if (!packet || packet.size() < 14 || packet.size() > mtu_+14) return -EMSGSIZE;
  auto *p = packet.data();
  wire::copy(p, destination, 6); wire::copy(p+6, mac_.bytes, 6); wire::write16(p+12, type);
  return transmit_frame(packet);
}
int EthernetLayer::transmit_frame(Packet &packet)
{
  if (!packet || packet.size() < 14 || packet.size() > mtu_+14) return -EMSGSIZE;
  if (!online() || !carrier()) return -ENETUNREACH;
  if (!device_->transmit(packet.data(), packet.size())) return -EAGAIN;
  packet.release();
  return 0;
}
void EthernetLayer::receive_frame(void *page, unsigned length)
{
  Packet packet(page, length);
  constexpr unsigned char broadcast[6] = {255,255,255,255,255,255};
  if (length < 14 || length > mtu_+14) return;
  if (dispatch_frame(packet)) return;
  if (!wire::equal(page, mac_.bytes, 6) && !wire::equal(page, broadcast, 6)) return;
  for (auto &b : bindings_)
    if (b.protocol && b.type == wire::read16(packet.data()+12)) { b.protocol->receive(packet); return; }
}
}
