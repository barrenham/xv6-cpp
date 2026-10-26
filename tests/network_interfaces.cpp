#include <cstdio>
#include <cstdlib>
#include <cstring>
import xv.net;
import xv.drivers;
extern "C" void net_test_free(void *);
extern "C" int net_test_pages();
namespace
{
void check(bool value, const char *message)
{ if (!value) { std::fprintf(stderr, "interface FAIL: %s\n", message); std::exit(1); } }
unsigned get16(const unsigned char *p) { return (unsigned(p[0])<<8)|p[1]; }
unsigned get32(const unsigned char *p) { return (get16(p)<<16)|get16(p+2); }
void put16(unsigned char *p, unsigned n) { p[0]=n>>8; p[1]=n; }
void put32(unsigned char *p, unsigned n) { put16(p,n>>16); put16(p+2,n); }
unsigned sum(const unsigned char *p, unsigned n)
{
  unsigned value=0; while (n>=2) { value+=get16(p); p+=2; n-=2; }
  if (n) value+=unsigned(*p)<<8;
  while (value>>16) value=(value&65535)+(value>>16);
  return value;
}
void checksum(unsigned char *p) { put16(p+10,0); put16(p+10,~sum(p,(p[0]&15)*4)); }
class Capture final : public xv::net::NetworkInterface
{
public:
  Capture(unsigned id, const char *name) : NetworkInterface(id,name,1500) {}
  unsigned char captured[4096]{};
  unsigned length=0, hop=0;
  bool connected=true;
  bool carrier() const override { return connected; }
  int transmit(xv::net::Packet &packet, unsigned next_hop) override
  {
    length=packet.size(); hop=next_hop;
    std::memcpy(captured,packet.data(),length); net_test_free(packet.release()); return 0;
  }
};
class Media final : public xv::drivers::EthernetDevice
{
public:
  bool online() const override { return true; }
  void mac_address(unsigned char *out) const override
  { std::memcpy(out,"\x52\x54\x00\x11\x22\x33",6); }
  bool transmit(void *page,unsigned) override { net_test_free(page); return true; }
};
class Receiver final : public xv::net::IpProtocol
{
public:
  unsigned count=0, ingress=0;
  void receive(xv::net::Packet &packet,const xv::net::IpDatagram &d) override
  {
    check(d.length==4 && d.offset==20 && !std::memcmp(packet.data()+d.offset,"ping",4),"pure IP payload boundary");
    ++count; ingress=d.interface->id();
  }
};
}
extern "C" void run_network_interface_contracts()
{
  using namespace xv::net;
  int baseline=net_test_pages();
  InterfaceRegistry interfaces;
  RoutingTable routes(interfaces);
  Ipv4Layer ip(interfaces,routes);
  LoopbackInterface loop;
  Capture first(2,"a"),second(3,"b");
  Media media;
  EthernetInterface unnumbered(4,"addressless");
  IpInIpInterface tunnel(5,"tun0"),unnumbered_tunnel(6,"tun1");
  IpInIpProtocol tunnels;
  Receiver receiver;
  ip.initialize(); loop.initialize(); unnumbered.initialize(media);
  check(loop.add_address({0x7f000001,8}) && loop.set_up(true),"loopback configuration");
  check(first.add_address({0xc0000201,24}) && first.set_up(true),"first L3 interface");
  check(second.add_address({0xc6336401,24}) && second.add_address({0xc6336409,32}) && second.set_up(true),"multiple IPv4 addresses");
  check(unnumbered.set_up(true) && unnumbered.mac() && !unnumbered.address_count(),"MAC without IPv4");
  check(!loop.mac() && !first.mac(),"interfaces without MAC");
  check(interfaces.add(loop) && interfaces.add(first) && interfaces.add(second) && interfaces.add(unnumbered),"heterogeneous registry");
  Capture duplicate_id(2,"other"),duplicate_name(8,"a");
  check(!interfaces.add(duplicate_id) && !interfaces.add(duplicate_name),"duplicate interface identity rejected");
  check(!second.add_address({0xc6336401,24}) && !second.add_address({1,33}),"invalid/duplicate address rejected");
  check(routes.add(0x7f000000,8,loop) && routes.add(0,0,first,0xc0000202,10) &&
        routes.add(0,0,second,0xc6336402,20) && routes.add(0xc0000200,24,first) &&
        routes.add(0xc6336400,24,second) && routes.add(0xc6336480,25,first,0xc0000202),"route setup");
  RouteSelection selection;
  check(routes.lookup(0xc6336402,selection) && selection.interface==&second && selection.source==0xc6336401,"connected route source selection");
  check(routes.lookup(0xc63364c8,selection) && selection.interface==&first && selection.next_hop==0xc0000202,"longest prefix wins");
  check(routes.lookup(0x08080808,selection) && selection.interface==&first,"default route metric");
  check(routes.lookup(0xc6336402,selection,0xc6336409) && selection.source==0xc6336409,"explicit alias source");
  check(!routes.lookup(0xc6336402,selection,0xcb007109),"unconfigured source rejected");
  first.connected=false;
  check(routes.lookup(0x08080808,selection) && selection.interface==&second,"carrier-down fallback"); first.connected=true;
  {
    auto packet=Packet::allocate(20);
    check(unnumbered.transmit(packet,0xc0000202)==-101 && bool(packet),"unnumbered link cannot send IPv4 and preserves ownership");
  }
  check(tunnel.add_address({0x0a080001,24}) && tunnel.set_up(true) &&
        tunnel.initialize(ip,first,0xc0000201,0xc0000202) && interfaces.add(tunnel),"IP-in-IP setup");
  check(routes.add(0x0a080000,24,tunnel) && tunnels.add(tunnel),"tunnel routing and receive registration");
  check(!tunnel.mac(),"tunnel does not fabricate MAC");
  check(unnumbered_tunnel.initialize(ip,first,0xc0000201,0xc0000203) &&
        unnumbered_tunnel.set_up(true) && interfaces.add(unnumbered_tunnel) &&
        routes.add(0xcb007100,24,unnumbered_tunnel,0,0,0xc0000201) && tunnels.add(unnumbered_tunnel),
        "unnumbered tunnel borrows an explicitly routed source");
  unnumbered_tunnel.attach(ip);
  loop.attach(ip); first.attach(ip); second.attach(ip); tunnel.attach(ip); unnumbered.attach(ip);
  ip.attach(99,receiver); ip.attach(4,tunnels);
  check(interfaces.seal(),"valid configuration seal"); routes.seal();
  check(!second.add_address({0xc633640a,24}) && !first.set_up(false) && !routes.add(0,0,first),"configuration immutable after publication");
  {
    auto packet=ip.allocate(4); std::memcpy(packet.data()+20,"ping",4);
    check(ip.send(packet,0x7f000001,99)==0 && !packet,"loopback owns queued pure IP packet");
    loop.tick(0); check(receiver.count==1 && receiver.ingress==loop.id(),"loopback ingress identity");
  }
  {
    auto packet=ip.allocate(4); std::memcpy(packet.data()+20,"ping",4);
    check(ip.send(packet,0x0a080002,99)==0 && !packet,"tunnel send");
    check(first.length==44 && first.captured[9]==4 && first.captured[29]==99 &&
          get32(first.captured+12)==0xc0000201 && get32(first.captured+32)==0x0a080001 &&
          first.hop==0xc0000202 && sum(first.captured,20)==65535,"outer encapsulation uses physical underlay");
    auto received=Packet::allocate(first.length); std::memcpy(received.data(),first.captured,first.length);
    put32(received.data()+12,0xc0000202); put32(received.data()+16,0xc0000201);
    put32(received.data()+32,0x0a080002); put32(received.data()+36,0x0a080001);
    checksum(received.data()); checksum(received.data()+20);
    second.deliver(received); check(receiver.count==1,"tunnel rejects wrong ingress despite matching outer endpoint");
    first.deliver(received); check(receiver.count==2 && receiver.ingress==tunnel.id(),"decapsulation carries tunnel interface identity");
  }
  {
    auto received=Packet::allocate(44); std::memcpy(received.data(),first.captured,44);
    put32(received.data()+12,0xc0000203); put32(received.data()+16,0xc0000201); checksum(received.data());
    first.deliver(received); check(receiver.count==2,"tunnel peer mismatch dropped");
  }
  {
    auto packet=ip.allocate(1461);
    check(ip.send(packet,0x0a080002,99)==-90 && bool(packet),"inner MTU checked before encapsulation");
    check(!packet.resize(4097),"public packet bounds preserved");
  }
  {
    auto packet=ip.allocate(4); std::memcpy(packet.data()+20,"ping",4);
    check(ip.send(packet,0xcb007102,99)==0 && first.length==44 && get32(first.captured+32)==0xc0000201 &&
          !unnumbered_tunnel.address_count() && !unnumbered_tunnel.mac(),"IP over interface without inner IP or MAC");
    auto reply=Packet::allocate(44); std::memcpy(reply.data(),first.captured,44);
    put32(reply.data()+12,0xc0000203); put32(reply.data()+16,0xc0000201);
    put32(reply.data()+32,0xcb007102); put32(reply.data()+36,0xc0000201);
    checksum(reply.data()); checksum(reply.data()+20);
    first.deliver(reply);
    check(receiver.count==3 && receiver.ingress==unnumbered_tunnel.id(),"unnumbered tunnel ingress");
  }
  IpInIpInterface nested(9,"nested");
  check(!nested.initialize(ip,tunnel,0x0a080001,0x0a080002),"recursive tunnel underlay rejected");
  check(net_test_pages()==baseline,"interface paths release every packet page");
  std::puts("network interfaces: routes, addressless links, loopback and tunnel passed");
}
