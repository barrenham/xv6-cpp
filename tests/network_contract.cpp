#include <cstdio>
#include <cstdlib>
#include <cstring>
import xv.net;
extern "C" void *net_test_allocate();
extern "C" void net_test_deliver(void *, unsigned);
extern "C" void net_test_capture(void *, unsigned *);
extern "C" void net_test_device(bool, bool);
extern "C" int net_test_pages();
extern "C" void net_test_owner(int);
namespace
{
void check(bool okay, const char *message)
{ if (!okay) { std::fprintf(stderr, "network FAIL: %s\n", message); std::exit(1); } }
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
  xv::net::release_owner(1);
  check(net_test_pages() == 0, "all packet pages released");
  std::puts("network contract: all passed");
}
