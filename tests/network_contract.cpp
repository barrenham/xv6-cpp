#include "user/runtime/icmp/echo.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "kernel/net/filter/control.h"
import xv.net;
extern "C" void run_firewall_contracts();
extern "C" void run_network_interface_contracts();
extern "C" void run_network_control_contracts();
extern "C" void run_bridge_contracts();
extern "C" void run_dhcp_contracts();
extern "C" void run_tcp_contracts();
extern "C" void run_packet_reassembly_contracts();
extern "C" void *net_test_allocate();
extern "C" void net_test_deliver(void *, unsigned);
extern "C" void net_test_capture(void *, unsigned *);
extern "C" void net_test_device(bool, bool);
extern "C" int net_test_pages();
extern "C" void net_test_clear_interrupt();
extern "C" void net_test_clock(unsigned long);
extern "C" void net_test_fail_after(int);
namespace
{
void check(bool okay, const char *message)
{ if (!okay) { std::fprintf(stderr, "network FAIL: %s\n", message); std::exit(1); } }
void tick(unsigned long now) { net_test_clock(now); xv::net::timer_tick(); }
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

int receive(xv::net::Socket *socket, char *data, unsigned capacity)
{
  return socket->receive_from({address(data), false}, capacity, nullptr);
}
}
int main()
{
  run_packet_reassembly_contracts();
  xv::net::initialize();
  run_firewall_contracts();
  auto *datagram = xv::net::create_socket(2, 17);
  check(datagram && datagram->bind({0, 2000}) == 0, "UDP socket bind");
  xv::net::Endpoint peer{0x0a000202, 9000};
  check(datagram->send_to({address("hello"), false}, 5, &peer) == 5, "UDP socket send");
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
  check(net_test_pages() == 1, "short packets released");
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
    check(net_test_pages() == 1, "invalid/unbound datagram released");
  }
  for (int i = 0; i < 40; ++i) inject(good, good_length);
  check(net_test_pages() == 17, "bounded UDP socket queue");
  auto *second = xv::net::create_socket(2, 17);
  check(second && second->bind({0, 2001}) == 0, "second UDP socket");
  unsigned char other[4096]; std::memcpy(other, good, good_length); put16(other+36, 2001);
  inject(other, good_length);
  char data[32]{}; xv::net::Endpoint source{};
  check(second->receive_from({address(data), false}, sizeof(data), &source) == 5 &&
        source.address == 0x0a000202 && source.port == 9000 && !std::memcmp(data, "hello", 5), "socket isolation and metadata");
  second->close();
  check(receive(datagram, data, 3) == 3 && !std::memcmp(data, "hel", 3), "truncate");
  check(datagram->receive_from({~0UL, false}, 5, nullptr) < 0, "bad copyout fails");
  check(net_test_pages() == 15, "bad copyout consumes and releases datagram");
  datagram->close(); check(net_test_pages() == 0, "socket close releases queue");
  xv::net::Socket *capacity[64]{};
  for (unsigned i = 0; i < 64; ++i)
  {
    capacity[i] = xv::net::create_socket(2, 17);
    check(capacity[i] && capacity[i]->bind({0, static_cast<unsigned short>(i+1)}) == 0, "socket table capacity");
  }
  check(!xv::net::create_socket(2, 17) && net_test_pages() == 64, "socket table exhaustion rolls back allocation");
  for (auto *socket : capacity) socket->close();
  check(net_test_pages() == 0, "socket capacity restored");
  datagram = xv::net::create_socket(2, 17);
  check(datagram && datagram->bind({0, 2000}) == 0, "rebind UDP socket");
  check(receive(datagram, data, sizeof(data)) < 0, "interrupted wait");
  net_test_clear_interrupt();
  net_test_device(true, false);
  check(datagram->send_to({address("hello"), false}, 5, &peer) == -11 &&
        net_test_pages() == 1, "TX full returns ownership");
  net_test_device(false, true);
  check(datagram->send_to({address("hello"), false}, 5, &peer) < 0 && net_test_pages() == 1, "absent NIC transmit fails without leaking");
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
  firewall_request filter{}; filter.version=FW_VERSION; filter.size=sizeof(filter);
  std::strcpy(filter.chain,"INPUT"); filter.rule.protocol=17;
  filter.rule.dport_low=filter.rule.dport_high=2000; filter.rule.action=FW_DROP;
  check(!xv::net::firewall_control(FW_APPEND,filter),"fragment port rule");
  unsigned char foreign[47]; std::memcpy(foreign,good,47);
  std::memcpy(foreign+30,"\xc0\x00\x02\x01",4); ip_checksum(foreign); inject(foreign,47);
  check(xv::net::firewall_control(FW_RULE,filter)==1 && !filter.packets,"INPUT excludes nonlocal unicast");
  inject(last,39); inject(first,42);
  check(datagram->receive_from({address(data),true},sizeof(data),nullptr)==-11,"reassembled fragment port DROP");
  check(xv::net::firewall_control(FW_RULE,filter)==1 && filter.packets==1 && filter.bytes==33,"fragments counted once per datagram");
  check(!xv::net::firewall_control(FW_FLUSH,filter) && net_test_pages()==1,"fragment DROP releases pages");
  inject(last, 39); check(net_test_pages() == 2, "fragment retained");
  inject(first, 42); check(receive(datagram, data, sizeof(data)) == 5 && !std::memcmp(data, "hello", 5), "reverse fragment reassembly");
  check(net_test_pages() == 1, "reassembled page released");
  put16(first+18, 100); ip_checksum(first); inject(first, 42); inject(first, 42);
  check(net_test_pages() == 1, "overlap rejected");
  put16(first+18, 101); ip_checksum(first); inject(first, 42); tick(16000);
  check(net_test_pages() == 1, "fragment expiry");
  unsigned char icmp[4096]; std::memcpy(icmp, good, 34); std::memset(icmp+34, 0, 11);
  icmp[23]=1; icmp[34]=8; std::memcpy(icmp+42, "abc", 3);
  put16(icmp+36, checksum(icmp+34, 11)); put16(icmp+16, 31); ip_checksum(icmp);
  inject(icmp, 45); unsigned captured_length; net_test_capture(icmp, &captured_length);
  check(captured_length == 45 && icmp[34] == 0 && checksum(icmp+34, 11) == 0, "ICMP echo");
  datagram->close();
  net_test_clear_interrupt();
  // Echo datagram sockets use independent identifiers and preserve queue ownership.
  auto *ping = xv::net::create_socket(2, 1);
  auto *ping2 = xv::net::create_socket(2, 1);
  check(ping && ping2, "ICMP sockets");
  check(!ping->connect({0x7f000001,0},false) && !ping2->connect({0x7f000001,0},false), "ICMP portless connect");
  check(ping->local().port != ping2->local().port, "unique ICMP identifiers");
  unsigned char request[65], reply[65];
  xv::echo::request(request,sizeof(request),7,12345);
  check(ping->send_to({address(request),false},7,nullptr)==-90, "short echo rejected");
  request[0]=3;
  check(ping->send_to({address(request),false},sizeof(request),nullptr)==-22, "only echo requests allowed");
  request[0]=8;
  for (unsigned i=0;i<50;++i) {
    check(ping->send_to({address(request),false},sizeof(request),nullptr)==sizeof(request), "ICMP loopback send");
    tick(16001+i*2); tick(16002+i*2);
    check(ping->receive_from({address(reply),true},sizeof(reply),nullptr)==sizeof(reply), "ICMP loopback receive");
    check(xv::echo::matches(reply,sizeof(reply),request,sizeof(request),ping->local().port), "ICMP checksum payload and identifier rewrite");
    check(ping2->receive_from({address(reply),true},sizeof(reply),nullptr)==-11, "ICMP socket isolation");
  }
  ping->close();ping2->close();
  check(net_test_pages()==0,"ICMP socket pages released");
  run_tcp_contracts();
  run_network_interface_contracts();
  run_network_control_contracts();
  run_bridge_contracts();
  run_dhcp_contracts();
  std::puts("network contract: all passed");
}
