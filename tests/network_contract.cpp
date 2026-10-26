#include <cstdio>
#include <cstdlib>
#include <cstring>
import xv.net;
extern "C" void run_network_interface_contracts();
extern "C" void run_network_control_contracts();
extern "C" void run_bridge_contracts();
extern "C" void *net_test_allocate();
extern "C" void net_test_deliver(void *, unsigned);
extern "C" void net_test_capture(void *, unsigned *);
extern "C" void net_test_device(bool, bool);
extern "C" int net_test_pages();
extern "C" void net_test_owner(int);
extern "C" void net_test_clock(unsigned long);
extern "C" void net_test_fail_after(int);
namespace
{
void check(bool okay, const char *message)
{ if (!okay) { std::fprintf(stderr, "network FAIL: %s\n", message); std::exit(1); } }
void tick(unsigned long now) { net_test_clock(now); xv::net::timer_tick(); }
unsigned get32(const unsigned char *p) { return (unsigned(p[0])<<24) | (unsigned(p[1])<<16) | (unsigned(p[2])<<8) | p[3]; }
void put32(unsigned char *p, unsigned n) { p[0]=n>>24; p[1]=n>>16; p[2]=n>>8; p[3]=n; }
unsigned long address(const void *p) { return reinterpret_cast<unsigned long>(p); }
void put16(unsigned char *p, unsigned n) { p[0] = n >> 8; p[1] = n; }
unsigned get16(const unsigned char *p) { return (unsigned(p[0]) << 8) | p[1]; }
unsigned checksum(const unsigned char *p, unsigned length)
{
  unsigned value = 0;
  while (length >= 2) { value += get16(p); p += 2; length -= 2; }
  if (length) value += unsigned(*p) << 8;
  while (value >> 16) value = (value & 65535) + (value >> 16);
  return (~value) & 65535;
}
void ip_checksum(unsigned char *p)
{ p[24] = p[25] = 0; put16(p+24, checksum(p+14, 20)); }
unsigned char good[4096];
unsigned good_length;
void inject(const unsigned char *data, unsigned length)
{
  void *page = net_test_allocate();
  std::memcpy(page, data, length);
  net_test_deliver(page, length);
}
void tcp_reply(unsigned char *out, const unsigned char *sent, unsigned sequence,
               unsigned acknowledgement, unsigned flags, const char *data = nullptr, unsigned length = 0)
{
  std::memcpy(out, sent, 34); std::memcpy(out, sent+6, 6); std::memcpy(out+6, sent, 6);
  std::memcpy(out+26, sent+30, 4); std::memcpy(out+30, sent+26, 4);
  std::memset(out+34, 0, 20+length); put16(out+34, get16(sent+36)); put16(out+36, get16(sent+34));
  put32(out+38, sequence); put32(out+42, acknowledgement); out[46] = 0x50; out[47] = flags;
  put16(out+48, 4096); if (length) std::memcpy(out+54, data, length);
  unsigned char pseudo[12+1460+20]; std::memcpy(pseudo, out+26, 8); pseudo[8]=0; pseudo[9]=6;
  put16(pseudo+10, 20+length); std::memcpy(pseudo+12, out+34, 20+length);
  put16(out+50, checksum(pseudo, 32+length)); put16(out+16, 40+length); ip_checksum(out);
}

int receive(unsigned short port, char *data, unsigned capacity)
{
  unsigned source = 0;
  unsigned short sport = 0;
  return xv::net::receive(port, address(&source), address(&sport), address(data), capacity);
}
}
int main()
{
  xv::net::initialize();
  check(xv::net::bind(2000) == 0, "bind");
  check(xv::net::send(2000, 0x0a000202, 9000, address("hello"), 5) == 0, "send");
  net_test_capture(good, &good_length);
  check(good_length == 47 && checksum(good+14, 20) == 0, "wire IPv4 length/checksum");
  unsigned char pseudo[12+13];
  std::memcpy(pseudo, good+26, 8); pseudo[8] = 0; pseudo[9] = 17;
  put16(pseudo+10, 13); std::memcpy(pseudo+12, good+34, 13);
  check(checksum(pseudo, sizeof(pseudo)) == 0, "wire UDP pseudoheader/checksum");
  std::memcpy(good, good+6, 6);
  std::memcpy(good+6, "\x52\x55\x0a\x00\x02\x02", 6);
  std::memcpy(good+26, "\x0a\x00\x02\x02", 4);
  std::memcpy(good+30, "\x0a\x00\x02\x0f", 4);
  put16(good+34, 9000); put16(good+36, 2000); put16(good+40, 0);
  ip_checksum(good);
  for (unsigned length = 0; length < 47; ++length) inject(good, length);
  check(net_test_pages() == 0, "short packets released");
  for (unsigned variant = 0; variant < 10; ++variant)
  {
    unsigned char bad[4096]; std::memcpy(bad, good, good_length);
    switch (variant)
    {
    case 0: bad[0] ^= 1; break;
    case 1: bad[14] = 0x65; break;
    case 2: bad[14] = 0x44; break;
    case 3: put16(bad+16, 1500); break;
    case 4: put16(bad+20, 0x2000); ip_checksum(bad); break;
    case 5: bad[24] ^= 1; break;
    case 6: put16(bad+38, 7); break;
    case 7: put16(bad+40, 1234); break;
    case 8: put16(bad+36, 3333); break;
    case 9: bad[33] ^= 1; ip_checksum(bad); break;
    }
    inject(bad, good_length);
    check(net_test_pages() == 0, "invalid/unbound datagram released");
  }
  for (int i = 0; i < 40; ++i) inject(good, good_length);
  check(net_test_pages() == 16, "bounded per-port queue");
  check(xv::net::bind(2001) == 0, "second bind");
  unsigned char other[4096]; std::memcpy(other, good, good_length); put16(other+36, 2001);
  inject(other, good_length);
  char data[32]{};
  check(receive(2001, data, sizeof(data)) == 5 && !std::memcmp(data, "hello", 5), "port isolation");
  check(xv::net::inherit(1, 2), "inherit");
  xv::net::release_owner(1);
  check(net_test_pages() == 16, "inherited queue retained");
  net_test_owner(2);
  check(receive(2000, data, 3) == 3 && !std::memcmp(data, "hel", 3), "truncate");
  check(xv::net::receive(2000, ~0UL, address(data), address(data), 5) < 0,
        "bad copyout fails");
  check(net_test_pages() == 14, "bad copyout consumes and releases datagram");
  xv::net::release_owner(2);
  check(net_test_pages() == 0, "last owner releases queue");
  net_test_owner(1);
  for (unsigned port = 1; port <= 32; ++port) check(xv::net::bind(port) == 0, "port table capacity");
  check(xv::net::bind(33) < 0, "port table exhaustion");
  for (int child = 2; child <= 8; ++child) check(xv::net::inherit(1, child), "handle capacity");
  check(!xv::net::inherit(1, 9), "inherit exhaustion is atomic");
  for (int child = 1; child <= 9; ++child) xv::net::release_owner(child);
  check(xv::net::bind(2000) == 0, "capacity restored");
  check(receive(2000, data, sizeof(data)) < 0, "interrupted wait");
  net_test_owner(1);
  net_test_device(true, false);
  check(xv::net::send(2000, 0x0a000202, 9000, address("hello"), 5) == -11 &&
        net_test_pages() == 0, "TX full returns ownership");
  net_test_device(false, true);
  check(xv::net::bind(2001) < 0 && xv::net::send(2000, 1, 1, address(""), 0) < 0,
        "absent NIC");
  net_test_device(true, true);
  unsigned char arp[42]{};
  std::memset(arp, 255, 6); std::memcpy(arp+6, good+6, 6);
  put16(arp+12, 0x806); put16(arp+14, 1); put16(arp+16, 0x800);
  arp[18] = 6; arp[19] = 4; put16(arp+20, 1);
  std::memcpy(arp+22, good+6, 6); std::memcpy(arp+28, good+26, 4);
  std::memcpy(arp+38, good+30, 4);
  for (int i = 0; i < 2; ++i)
  {
    inject(arp, sizeof(arp));
    unsigned char reply[4096]; unsigned length;
    net_test_capture(reply, &length);
    check(length == 42 && get16(reply+20) == 2 && !std::memcmp(reply, arp+6, 6),
          "repeated ARP requests answered");
  }

  // IP reassembly, including reverse arrival, overlap rejection and expiry.
  unsigned char first[4096], last[4096];
  std::memcpy(first, good, 42); std::memcpy(last, good, 34);
  std::memcpy(last+34, good+42, 5);
  put16(first+16, 28); put16(first+18, 99); put16(first+20, 0x2000); ip_checksum(first);
  put16(last+16, 25); put16(last+18, 99); put16(last+20, 1); ip_checksum(last);
  inject(last, 39); check(net_test_pages() == 1, "fragment retained");
  inject(first, 42); check(receive(2000, data, sizeof(data)) == 5 && !std::memcmp(data, "hello", 5), "reverse fragment reassembly");
  check(net_test_pages() == 0, "reassembled page released");
  put16(first+18, 100); ip_checksum(first); inject(first, 42); inject(first, 42);
  check(net_test_pages() == 0, "overlap rejected");
  put16(first+18, 101); ip_checksum(first); inject(first, 42); tick(16000);
  check(net_test_pages() == 0, "fragment expiry");
  unsigned char icmp[4096]; std::memcpy(icmp, good, 34); std::memset(icmp+34, 0, 11);
  icmp[23]=1; icmp[34]=8; std::memcpy(icmp+42, "abc", 3);
  put16(icmp+36, checksum(icmp+34, 11)); put16(icmp+16, 31); ip_checksum(icmp);
  inject(icmp, 45); unsigned captured_length; net_test_capture(icmp, &captured_length);
  check(captured_length == 45 && icmp[34] == 0 && checksum(icmp+34, 11) == 0, "ICMP echo");
  xv::net::release_owner(1);
  net_test_owner(1);
  auto *stream = xv::net::create_socket(1, 6);
  check(stream && stream->connect({0x0a000202, 9000}, true) == -115, "nonblocking TCP SYN");
  unsigned char syn[4096], response[4096], data_segment[4096]; unsigned syn_length;
  net_test_capture(syn, &syn_length); unsigned initial = get32(syn+38);
  check(syn_length == 58 && syn[47] == 2, "SYN with MSS");
  tick(17000); net_test_capture(response, &captured_length);
  check(response[47] == 2 && get32(response+38) == initial, "SYN retransmission");
  tcp_reply(response, syn, 4000, initial+1, 18); inject(response, 54);
  check(stream->poll(nullptr)&4, "TCP established writable");
  check(stream->write({address("hello"), true}, 5) == 5, "TCP data queued");
  net_test_capture(data_segment, &captured_length);
  unsigned next = get32(data_segment+38)+5;
  check(stream->write({address("again"), true}, 5) == -11, "unacknowledged send backpressure");
  tcp_reply(response, data_segment, 4001, next+100, 16); inject(response, 54);
  check(stream->write({address("again"), true}, 5) == -11, "future ACK rejected");
  tick(19000); net_test_capture(response, &captured_length);
  check(captured_length == 59 && get32(response+38) == get32(data_segment+38) && !std::memcmp(response+54, "hello", 5), "data retransmission");
  tcp_reply(response, data_segment, 4001, next, 24, "hello", 5); inject(response, 59); inject(response, 59);
  check(stream->read({address(data), true}, sizeof(data)) == 5 && !std::memcmp(data, "hello", 5), "duplicate data delivered once");
  check(stream->read({address(data), true}, sizeof(data)) == -11, "duplicate not requeued");
  tcp_reply(response, data_segment, 4008, next, 24, "gap", 3); inject(response, 57);
  check(stream->read({address(data), true}, sizeof(data)) == -11, "out of order waits for retransmission");
  tcp_reply(response, data_segment, 4006, next, 24, "ab", 2); inject(response, 56);
  tcp_reply(response, data_segment, 4008, next, 24, "gap", 3); inject(response, 57);
  check(stream->read({address(data), true}, sizeof(data)) == 5 && !std::memcmp(data, "abgap", 5), "gap recovery");
  tcp_reply(response, data_segment, 4012, next, 20); inject(response, 54);
  check(!(stream->poll(nullptr)&8), "inexact RST challenged");
  tcp_reply(response, data_segment, 4011, next, 20); inject(response, 54);
  check((stream->poll(nullptr)&8) && stream->read({address(data), true}, 1) == -104, "valid RST closes stream");
  stream->close(); check(net_test_pages() == 0, "reset socket resources released");
  stream = xv::net::create_socket(1, 6);
  check(stream && stream->connect({0x0a000202, 9001}, true) == -115, "timeout connect");
  for (unsigned long now = 19500; now <= 250000; now += 500) tick(now);
  check(stream->error() == 110, "bounded retry timeout"); stream->close();
  check(net_test_pages() == 0, "all packet and socket pages released");

  net_test_fail_after(1);
  check(xv::net::create_socket(1, 6) == nullptr && net_test_pages() == 0, "TCP partial allocation rollback");
  net_test_fail_after(-1);
  run_network_interface_contracts();
  run_network_control_contracts();
  run_bridge_contracts();
  std::puts("network contract: all passed");
}
