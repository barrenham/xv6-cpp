module;
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "kernel/net/errno.h"
module xv.net;
import :tcp;
extern "C" int net_test_pages();
extern "C" void net_test_fail_after(int);

namespace
{
using namespace xv::net;
constexpr Endpoint Local{0xc0000201, 2000};
constexpr Endpoint Peer{0xc0000202, 9000};

void check_protocol(bool condition, const char *message)
{
  if (!condition)
  {
    std::fprintf(stderr, "TCP protocol flow FAIL: %s\n", message);
    std::exit(1);
  }
}

unsigned read16(const unsigned char *p)
{
  return unsigned(p[0]) * 256 + p[1];
}

unsigned read32(const unsigned char *p)
{
  return read16(p) * 65536 + read16(p + 2);
}

void write16(unsigned char *p, unsigned n)
{
  p[0] = n >> 8;
  p[1] = n;
}

void write32(unsigned char *p, unsigned n)
{
  write16(p, n >> 16);
  write16(p + 2, n);
}

unsigned sum(const unsigned char *p, unsigned n, unsigned value = 0)
{
  for (unsigned i = 0; i < n; ++i)
  {
    value += unsigned(p[i]) << ((i & 1) ? 0 : 8);
  }
  while (value >> 16)
  {
    value = (value & 0xffff) + (value >> 16);
  }
  return value;
}

void checksum(Packet &packet, unsigned offset, unsigned length)
{
  auto *segment = packet.data() + offset;
  write16(segment + 16, 0);
  write16(segment + 16, ~sum(segment, length, sum(packet.data() + 12, 8, 6 + length)));
}

class CaptureInterface final : public NetworkInterface
{
public:

  unsigned calls = 0, length = 0;
  int result = 0;
  unsigned char bytes[4096]{};

  explicit CaptureInterface(unsigned mtu = 1500) : NetworkInterface(3, "flow", mtu)
  {
  }

  int transmit(Packet &packet, unsigned next_hop) override
  {
    check_protocol(next_hop == Peer.address, "selected next hop");
    ++calls;
    length = packet.size();
    std::memcpy(bytes, packet.data(), length);
    return result;
  }
};

struct Stack
{
  CaptureInterface interface;
  InterfaceRegistry interfaces;
  RoutingTable routes{interfaces};
  Ipv4Layer ip{interfaces, routes};
  TcpProtocol tcp{ip};

  explicit Stack(unsigned mtu = 1500) : interface(mtu)
  {
    ip.initialize();
    check_protocol(interface.add_address({Local.address, 24}) &&
                       interface.set_up(true) && interfaces.add(interface) &&
                       routes.add(0xc0000200, 24, interface) && interfaces.seal(),
                   "isolated stack setup");
    routes.seal();
  }
};

class Sink final : public TcpSegmentSink
{
public:

  unsigned calls = 0;
  bool handled = true;
  const unsigned char *expected_data = nullptr;
  unsigned expected_length = 0;

  bool deliver_segment(const TcpSegment &segment) override
  {
    ++calls;
    check_protocol(
        segment.source.address == Peer.address && segment.source.port == Peer.port &&
            segment.destination.address == Local.address &&
            segment.destination.port == Local.port && segment.data == expected_data &&
            segment.length == expected_length,
        "sink borrows the validated segment with decoded endpoints");
    return handled;
  }
};

Packet incoming(unsigned flags = 2, unsigned sequence = 100, unsigned ack = 500,
                unsigned payload = 0, unsigned offset = 20, unsigned header = 20)
{
  auto packet = Packet::allocate(offset + header + payload);
  check_protocol(bool(packet), "incoming allocation");
  std::memset(packet.data(), 0, packet.size());
  write32(packet.data() + 12, Peer.address);
  write32(packet.data() + 16, Local.address);
  auto *segment = packet.data() + offset;
  write16(segment, Peer.port);
  write16(segment + 2, Local.port);
  write32(segment + 4, sequence);
  write32(segment + 8, ack);
  segment[12] = (header / 4) << 4;
  segment[13] = flags;
  for (unsigned i = 0; i < payload; ++i)
  {
    segment[header + i] = 'a' + i;
  }
  checksum(packet, offset, header + payload);
  return packet;
}

void deliver(Stack &stack, Packet &packet, unsigned offset = 20)
{
  stack.tcp.receive(packet, {Peer.address, Local.address, 6, offset,
                             packet.size() - offset, &stack.interface});
}

void check_wire(const CaptureInterface &interface, unsigned header, unsigned payload)
{
  const auto *bytes = interface.bytes;
  check_protocol(
      interface.length == 20 + header + payload && bytes[9] == 6 &&
          read16(bytes + 2) == interface.length && sum(bytes, 20) == 0xffff &&
          read32(bytes + 12) == Local.address && read32(bytes + 16) == Peer.address &&
          read16(bytes + 20) == Local.port && read16(bytes + 22) == Peer.port &&
          (bytes[32] >> 4) * 4 == header &&
          sum(bytes + 20, header + payload, sum(bytes + 12, 8, 6 + header + payload)) ==
              0xffff,
      "encoded IPv4/TCP lengths, endpoints and checksums");
}

void transmit_paths()
{
  Stack stack;
  int baseline = net_test_pages();
  check_protocol(stack.tcp.transmit(Local, Peer, 123, 456, 2, 4096, "odd", 3) == 0,
                 "SYN with odd payload");
  check_wire(stack.interface, 24, 3);
  auto *segment = stack.interface.bytes + 20;
  check_protocol(
      segment[13] == 2 && read32(segment + 4) == 123 && read32(segment + 8) == 456 &&
          read16(segment + 14) == 4096 && segment[20] == 2 && segment[21] == 4 &&
          read16(segment + 22) == 1460 && !std::memcmp(segment + 24, "odd", 3),
      "fixed header, MSS and payload encoding");
  check_protocol(stack.tcp.transmit(Local, Peer, 1, 2, 16, 9, nullptr, 0) == 0,
                 "ACK without options or payload");
  check_wire(stack.interface, 20, 0);
  check_protocol(stack.tcp.transmit(Local, {0xcb007101, 80}, 0, 0, 2, 0, nullptr, 0) ==
                         -ENETUNREACH &&
                     stack.interface.calls == 2,
                 "no route stops before allocation and send");
  net_test_fail_after(0);
  check_protocol(stack.tcp.transmit(Local, Peer, 0, 0, 2, 0, nullptr, 0) == -ENOBUFS,
                 "allocation failure propagates");
  net_test_fail_after(-1);
  char payload[1500]{};
  check_protocol(stack.tcp.transmit(Local, Peer, 0, 0, 16, 0, payload, 1500) ==
                         -EMSGSIZE &&
                     stack.interface.calls == 2,
                 "IPv4 MTU rejection propagates without device send");
  check_protocol(stack.tcp.transmit(Local, Peer, 0, 0, 16, 0, nullptr, 4096) ==
                     -ENOBUFS,
                 "oversize allocation stops before payload access");
  stack.interface.result = -EAGAIN;
  check_protocol(stack.tcp.transmit(Local, Peer, 0, 0, 16, 0, nullptr, 0) == -EAGAIN,
                 "device error propagates unchanged");
  check_protocol(net_test_pages() == baseline,
                 "all transmit exits release packet ownership");
  Stack small(68);
  check_protocol(!small.tcp.transmit(Local, Peer, 0, 0, 2, 0, nullptr, 0) &&
                     read16(small.interface.bytes + 42) == 28,
                 "MSS follows actual route MTU at minimum IPv4 MTU");
}

void receive_paths()
{
  Stack stack;
  Sink sink;
  stack.tcp.attach(sink);
  for (unsigned variant = 0; variant < 7; ++variant)
  {
    auto packet = incoming();
    auto *segment = packet.data() + 20;
    switch (variant)
    {
    case 0:
      check_protocol(packet.resize(39), "truncated header setup");
      break;
    case 1:
      segment[12] = 0x40;
      checksum(packet, 20, 20);
      break;
    case 2:
      segment[12] = 0x60;
      checksum(packet, 20, 20);
      break;
    case 3:
      segment[12] |= 1;
      checksum(packet, 20, 20);
      break;
    case 4:
      segment[16] ^= 1;
      break;
    case 5:
      write16(segment, 0);
      checksum(packet, 20, 20);
      break;
    case 6:
      write16(segment + 2, 0);
      checksum(packet, 20, 20);
      break;
    }
    deliver(stack, packet);
    check_protocol(sink.calls == 0 && stack.interface.calls == 0,
                   "invalid segments neither reach sink nor trigger resets");
  }
  // A short datagram must not even read the Packet's data pointer.
  Packet empty;
  stack.tcp.receive(empty, {Peer.address, Local.address, 6, 0, 0, &stack.interface});
  auto packet = incoming(4, 100, 500, 3, 24, 24);
  sink.expected_data = packet.data() + 24;
  sink.expected_length = packet.size() - 24;
  deliver(stack, packet, 24);
  check_protocol(sink.calls == 1 && stack.interface.calls == 0 && bool(packet),
                 "handled RST is delivered before RST gate, borrowed packet retained");
  sink.handled = false;
  deliver(stack, packet, 24);
  check_protocol(sink.calls == 2 && stack.interface.calls == 0,
                 "unmatched RST is not answered");
  packet.data()[24 + 13] = 16;
  checksum(packet, 24, packet.size() - 24);
  deliver(stack, packet, 24);
  check_wire(stack.interface, 20, 0);
  check_protocol(stack.interface.calls == 1 && stack.interface.bytes[33] == 4 &&
                     read32(stack.interface.bytes + 24) == 500 &&
                     read32(stack.interface.bytes + 28) == 0,
                 "unmatched ACK reset uses peer acknowledgement as sequence");

  Stack unattached;
  auto syn_fin = incoming(3, 0xfffffffeU, 500, 3);
  deliver(unattached, syn_fin);
  check_wire(unattached.interface, 20, 0);
  check_protocol(unattached.interface.calls == 1 &&
                     unattached.interface.bytes[33] == 20 &&
                     read32(unattached.interface.bytes + 24) == 0 &&
                     read32(unattached.interface.bytes + 28) == 3,
                 "no sink reset acknowledges payload plus SYN/FIN with sequence wrap");
  net_test_fail_after(0);
  deliver(unattached, syn_fin);
  net_test_fail_after(-1);
  check_protocol(unattached.interface.calls == 1 && bool(syn_fin),
                 "failed reset allocation keeps received packet ownership");
}
} // namespace

extern "C" void run_tcp_protocol_flow_contracts()
{
  int baseline = net_test_pages();
  transmit_paths();
  receive_paths();
  check_protocol(net_test_pages() == baseline,
                 "protocol flow fixtures release every page");
  std::puts(
      "TCP protocol flow: validation, sink handling, RST rules and transmit exits passed");
}
