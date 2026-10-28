module;
#include <cstdio>
#include <cstdlib>
#include <cstring>
module xv.net;
import :packet;
import :reassembly;
import :loopback;
import :wire;
extern "C" int net_test_pages();
extern "C" void net_test_fail_after(int);

namespace
{
using namespace xv::net;

void check_packet(bool condition, const char *message)
{
  if (condition)
  {
    return;
  }
  std::fprintf(stderr, "packet/reassembly FAIL: %s\n", message);
  std::exit(1);
}

Packet fragment(unsigned identification, unsigned offset, bool more,
                const char *payload, unsigned length, unsigned depth = 0)
{
  auto packet = Packet::allocate(20 + length);
  check_packet(bool(packet), "fragment allocation");
  auto *header = packet.data();
  wire::zero(header, 20);
  header[0] = 0x45;
  header[8] = 64;
  header[9] = 99;
  wire::write16(header + 2, packet.size());
  wire::write16(header + 4, identification);
  wire::write16(header + 6, (offset / 8) | (more ? 0x2000 : 0));
  wire::write32(header + 12, 0xc0000201);
  wire::write32(header + 16, 0xc0000202);
  wire::write16(header + 10, ~wire::sum(header, 20));
  wire::copy(header + 20, payload, length);
  for (unsigned i = 0; i < depth; ++i)
  {
    check_packet(packet.enter_tunnel(), "fragment depth setup");
  }
  return packet;
}

class ContextReceiver final : public IpReceiver
{
public:

  unsigned received = 0, depth = 0;

  void receive(NetworkInterface &, Packet &packet) override
  {
    ++received;
    depth = packet.context().tunnel_depth;
  }
};
} // namespace

extern "C" void run_packet_reassembly_contracts()
{
  using namespace xv::net;
  int baseline = net_test_pages();
  {
    auto packet = Packet::allocate(4);
    std::memcpy(packet.data(), "test", 4);
    check_packet(packet.enter_tunnel() && packet.enter_tunnel() &&
                     packet.enter_tunnel(),
                 "packet depth setup");
    auto copy = packet.clone();
    check_packet(copy && copy.data() != packet.data() &&
                     !std::memcmp(copy.data(), packet.data(), 4) &&
                     copy.context().tunnel_depth == 3,
                 "clone preserves bytes and context with independent ownership");
    copy.data()[0] = 'x';
    check_packet(packet.data()[0] == 't' && copy.enter_tunnel() && !copy.enter_tunnel(),
                 "clone preserves remaining tunnel budget");
    net_test_fail_after(0);
    check_packet(!packet.clone() && packet.context().tunnel_depth == 3 &&
                     packet.size() == 4,
                 "failed clone leaves original unchanged");
    net_test_fail_after(-1);
    Packet moved(static_cast<Packet &&>(packet));
    check_packet(moved && !packet && moved.context().tunnel_depth == 3 &&
                     packet.size() == 4 && packet.context().tunnel_depth == 3,
                 "move transfers ownership and preserves context");
    auto *page = moved.release();
    check_packet(!moved && moved.size() == 4 && moved.context().tunnel_depth == 3 &&
                     !moved.clone(),
                 "raw release preserves metadata while clearing ownership");
    Packet adopted(page, moved.size(), moved.context());
    check_packet(adopted.enter_tunnel() && !adopted.enter_tunnel(),
                 "page adoption restores queue context");
    check_packet(adopted.prepend(2) && adopted.size() == 6 && !adopted.data()[0] &&
                     !adopted.data()[1] && adopted.remove_prefix(2) &&
                     !std::memcmp(adopted.data(), "test", 4),
                 "prefix insertion and removal preserve payload");
    check_packet(!adopted.prepend(Packet::Capacity) && adopted.size() == 4 &&
                     !adopted.remove_prefix(5) && !adopted.resize(Packet::Capacity + 1),
                 "invalid bounds preserve packet size");
  }
  check_packet(net_test_pages() == baseline,
               "move clone and adoption release each page once");
  {
    LoopbackInterface loopback;
    loopback.initialize();
    loopback.set_up(true);
    ContextReceiver receiver;
    loopback.attach(receiver);
    auto packet = fragment(1, 0, true, "12345678", 8, 3);
    check_packet(!loopback.transmit(packet, 0) && !packet, "loopback takes ownership");
    loopback.tick(0);
    check_packet(receiver.received == 1 && receiver.depth == 3,
                 "loopback preserves packet processing context");
  }
  check_packet(net_test_pages() == baseline, "loopback context transfer releases page");
  unsigned ingress[5]{};
  Ipv4Reassembly reassembly;
  reassembly.initialize();
  {
    auto first = fragment(2, 0, true, "hello wo", 8, 3);
    auto last = fragment(2, 8, false, "rld", 3, 3);
    check_packet(!reassembly.accept(&ingress[1], last, 20, last.size(), 0),
                 "last fragment waits for first");
    auto complete = reassembly.accept(&ingress[1], first, 20, first.size(), 1);
    check_packet(complete && complete.size() == 31 &&
                     !std::memcmp(complete.data() + 20, "hello world", 11) &&
                     wire::read16(complete.data() + 2) == 31 &&
                     !wire::read16(complete.data() + 6) &&
                     wire::sum(complete.data(), 20) == 0xffff,
                 "reassembly restores payload length flags and checksum");
    check_packet(complete.context().tunnel_depth == 3 && complete.enter_tunnel() &&
                     !complete.enter_tunnel(),
                 "completed datagram retains tunnel processing budget");
  }
  {
    auto first = fragment(3, 0, true, "12345678", 8);
    auto last = fragment(3, 8, false, "tail", 4);
    check_packet(!reassembly.accept(&ingress[1], first, 20, first.size(), 0) &&
                     !reassembly.accept(&ingress[2], last, 20, last.size(), 0),
                 "ingress identity isolates fragment sets");
    auto complete_second = reassembly.accept(&ingress[2], first, 20, first.size(), 1);
    auto complete_first = reassembly.accept(&ingress[1], last, 20, last.size(), 1);
    check_packet(complete_second && complete_first,
                 "independent ingress assemblies complete");
  }
  {
    auto first_one = fragment(4, 0, true, "12345678", 8, 1);
    auto last_one = fragment(4, 8, false, "tail", 4, 1);
    auto first_two = fragment(4, 0, true, "12345678", 8, 2);
    auto last_two = fragment(4, 8, false, "tail", 4, 2);
    check_packet(!reassembly.accept(&ingress[1], first_one, 20, first_one.size(), 0) &&
                     !reassembly.accept(&ingress[1], last_two, 20, last_two.size(), 0),
                 "different processing contexts do not merge");
    auto complete_one =
        reassembly.accept(&ingress[1], last_one, 20, last_one.size(), 1);
    auto complete_two =
        reassembly.accept(&ingress[1], first_two, 20, first_two.size(), 1);
    check_packet(complete_one && complete_two &&
                     complete_one.context().tunnel_depth == 1 &&
                     complete_two.context().tunnel_depth == 2,
                 "context-isolated assemblies retain their own depth");
  }
  {
    auto first = fragment(5, 0, true, "12345678", 8);
    int pages = net_test_pages();
    check_packet(!reassembly.accept(&ingress[1], first, 20, first.size(), 0) &&
                     net_test_pages() == pages + 1,
                 "partial assembly owns a page");
    check_packet(!reassembly.accept(&ingress[1], first, 20, first.size(), 1) &&
                     net_test_pages() == pages,
                 "overlap discards the whole assembly");
    net_test_fail_after(0);
    check_packet(!reassembly.accept(&ingress[1], first, 20, first.size(), 1) &&
                     net_test_pages() == pages,
                 "allocation failure retains no state");
    net_test_fail_after(-1);
    for (unsigned index = 0; index < 5; ++index)
    {
      check_packet(!reassembly.accept(&ingress[index], first, 20, first.size(), 100),
                   "bounded assembly admission");
    }
    check_packet(net_test_pages() == pages + 4, "four assembly slots bound memory");
    reassembly.tick(15099);
    check_packet(net_test_pages() == pages + 4, "assemblies remain until deadline");
    reassembly.tick(15100);
    check_packet(net_test_pages() == pages, "deadline expiry releases every assembly");
    reassembly.accept(&ingress[1], first, 20, first.size(), 200);
    reassembly.clear();
    check_packet(net_test_pages() == pages,
                 "explicit teardown releases retained pages");
  }
  reassembly.clear();
  check_packet(net_test_pages() == baseline,
               "packet and reassembly contracts release every page");
  std::puts(
      "packet/reassembly: ownership, context, bounds, isolation, overlap, capacity and expiry passed");
}
