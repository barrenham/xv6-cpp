#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "kernel/net/control.h"
import xv.net;
import xv.drivers;
extern "C" int net_test_pages();
extern "C" void *net_test_allocate();
extern "C" void net_test_free(void *);
extern "C" void net_test_clock(unsigned long);
extern "C" void net_test_fail_after(int);
namespace
{
void check(bool ok,const char *message)
{ if (!ok) { std::fprintf(stderr,"DHCP FAIL: %s\n",message); std::exit(1); } }
unsigned get16(const unsigned char *p) { return unsigned(p[0])*256+p[1]; }
unsigned get32(const unsigned char *p) { return (get16(p)<<16)|get16(p+2); }
void put16(unsigned char *p,unsigned v) { p[0]=v>>8; p[1]=v; }
void put32(unsigned char *p,unsigned v) { put16(p,v>>16); put16(p+2,v); }
unsigned checksum(const unsigned char *p,unsigned n)
{
  unsigned value=0; while (n>=2) { value+=get16(p); p+=2; n-=2; }
  if (n) value+=unsigned(*p)<<8;
  while (value>>16) value=(value&65535)+(value>>16);
  return (~value)&65535;
}
netctl_request request(unsigned id=2)
{ netctl_request r{}; r.version=NETCTL_VERSION; r.size=sizeof(r); r.ifindex=id; return r; }
class Media final : public xv::drivers::EthernetDevice
{
public:
  xv::drivers::EthernetReceiver *receiver=nullptr;
  unsigned char bytes[1600]{}; unsigned length=0,count=0;
  bool available=true; unsigned suffix=0x60;
  bool online() const override { return available; }
  void mac_address(unsigned char *out) const override { std::memcpy(out,"\x52\x54\x00\x12\x34\x60",6); out[5]=suffix; }
  bool transmit(void *page,unsigned n) override
  {
    check(n<=sizeof(bytes),"captured frame bound"); std::memcpy(bytes,page,n); length=n; ++count;
    unsigned char arp[42]; bool response=n==42&&bytes[12]==8&&bytes[13]==6&&get16(bytes+20)==1;
    if (response)
    {
      std::memcpy(arp,bytes,42); std::memcpy(arp,bytes+6,6); std::memcpy(arp+6,"\x02\0\0\0\0\2",6);
      put16(arp+20,2); std::memcpy(arp+32,bytes+22,6); std::memcpy(arp+38,bytes+28,4);
      std::memcpy(arp+22,arp+6,6); std::memcpy(arp+28,bytes+38,4);
    }
    net_test_free(page);
    if (response) { auto *reply=net_test_allocate(); check(reply,"ARP reply allocation"); std::memcpy(reply,arp,42); receiver->receive_frame(reply,42); }
    return true;
  }
};
class Probe final : public xv::net::IpProtocol
{ public: unsigned count=0; void receive(xv::net::Packet &,const xv::net::IpDatagram &) override { ++count; } };
struct Fixture
{
  xv::net::InterfaceRegistry interfaces;
  xv::net::RoutingTable routes{interfaces};
  xv::net::Ipv4Layer ip{interfaces,routes};
  xv::net::LoopbackInterface loop;
  xv::net::EthernetInterface port{2,"dhcp"};
  xv::net::LeaseConfiguration configuration{interfaces,routes,loop};
  xv::net::DhcpManager dhcp{interfaces,ip,configuration};
  xv::net::NetworkManager manager{interfaces,routes,loop,nullptr,&dhcp};
  Media media,other_media; Probe probe;
  xv::net::EthernetInterface other{3,"other"};
  Fixture(bool second=false)
  {
    net_test_clock(0); ip.initialize(); loop.initialize(); port.initialize(media); media.receiver=&port.receiver();
    loop.add_address({0x7f000001,8}); loop.set_up(true); port.set_up(true); port.attach(ip); loop.attach(ip);
    interfaces.add(loop); interfaces.add(port);
    if (second)
    {
      other_media.suffix=0x61; other.initialize(other_media); other_media.receiver=&other.receiver();
      other.set_up(true); other.attach(ip); interfaces.add(other);
    }
    interfaces.seal(); routes.seal(); dhcp.initialize(); ip.attach(17,probe);
  }
  ~Fixture() { auto r=request(); dhcp.execute(NETCTL_DHCP_STOP,r); if (interfaces.find(3)) { r=request(3); dhcp.execute(NETCTL_DHCP_STOP,r); } }
  void tick(unsigned long now) { net_test_clock(now); dhcp.tick(now); }
  netctl_request status() { auto r=request(); check(dhcp.execute(NETCTL_DHCP_STATUS,r)==1,"status query"); return r; }
  void start() { auto r=request(); check(dhcp.execute(NETCTL_DHCP_START,r)==0,"start client without IP"); }
  unsigned xid() const { check(media.length>=342&&media.bytes[23]==17,"captured DHCP request"); return get32(media.bytes+46); }
  xv::net::Packet reply(unsigned type,unsigned address,unsigned transaction,unsigned server=0x0a000202)
  {
    auto p=xv::net::Packet::allocate(342); check(bool(p),"reply page"); auto *b=p.data(); std::memset(b,0,342);
    std::memset(b,255,6); std::memcpy(b+6,"\x02\0\0\0\0\2",6); put16(b+12,0x800);
    b[14]=0x45; put16(b+16,328); b[22]=64; b[23]=17; put32(b+26,server); put32(b+30,0xffffffff);
    put16(b+24,checksum(b+14,20)); put16(b+34,67); put16(b+36,68); put16(b+38,308);
    auto *d=b+42; d[0]=2; d[1]=1; d[2]=6; put32(d+4,transaction); put32(d+16,address);
    media.mac_address(d+28); put32(d+236,0x63825363);
    unsigned n=240; d[n++]=53; d[n++]=1; d[n++]=type;
    constexpr unsigned codes[]={54,1,3,6,51,58,59}; unsigned values[]={server,0xffffff00,0x0a000202,0x0a000203,8,2,6};
    for (unsigned i=0;i<7;++i) { d[n++]=codes[i]; d[n++]=4; put32(d+n,values[i]); n+=4; }
    d[n]=255; return p;
  }
  void inject(xv::net::Packet &p) { auto n=p.size(); port.receiver().receive_frame(p.release(),n); }
};

}
extern "C" void run_dhcp_contracts()
{
  using namespace xv::net;
  int baseline=net_test_pages();
  {
    Fixture f;
    auto r=request(1); check(f.dhcp.execute(NETCTL_DHCP_START,r)==-95,"loopback unsupported");
    auto blocked=f.reply(2,0x0a000264,0); put16(blocked.data()+36,2000); f.inject(blocked);
    check(!f.probe.count&&!f.port.address_count(),"ordinary UDP cannot bypass destination policy without IP");
    f.start(); f.tick(0); auto xid=f.xid();
    check(get32(f.media.bytes+26)==0&&get32(f.media.bytes+30)==0xffffffff&&get16(f.media.bytes+34)==68,"zero-source broadcast discover");
    auto wrong=f.reply(2,0x0a000264,xid+1); f.inject(wrong); check(f.status().flags==DHCP_SELECTING,"wrong transaction ignored");
    auto malformed=f.reply(2,0x0a000264,xid); malformed.data()[42+241]=255; f.inject(malformed);
    check(f.status().flags==DHCP_SELECTING,"truncated options ignored");
    auto offer=f.reply(2,0x0a000264,xid); f.inject(offer); check(f.status().flags==DHCP_REQUESTING,"offer selected");
    f.tick(100); xid=f.xid();
    auto foreign=f.reply(5,0x0a000264,xid,0x0a000209); f.inject(foreign);
    check(!f.port.address_count(),"unselected server ignored");
    auto ack=f.reply(5,0x0a000264,xid); f.inject(ack);
    auto status=f.status(); check(status.flags==DHCP_BOUND&&status.address==0x0a000264&&status.source==0x0a000203&&status.expires_ms==8000,"ACK installs lease");
    RouteSelection selection; check(f.routes.lookup(0x08080808,selection)&&selection.next_hop==0x0a000202,"leased default route");
    r=request(); r.address=status.address; r.prefix=24;
    check(f.manager.execute(NETCTL_DEL_ADDRESS,r)==-16,"leased address protected");
    r=request(); r.address=0x0a0002c8; r.prefix=24; check(f.manager.execute(NETCTL_ADD_ADDRESS,r)==0,"manual alias coexistence");
    r=request(); r.address=0xcb007100; r.prefix=24; r.source=0x0a000264; r.gateway=0x0a000202;
    check(f.manager.execute(NETCTL_ADD_ROUTE,r)==0,"manual route may reference leased source");
    unsigned generation=f.port.generation();
    f.tick(2100); check(f.status().flags==DHCP_RENEWING&&get32(f.media.bytes+30)==0x0a000202&&get32(f.media.bytes+54)==0x0a000264,"T1 unicast with ciaddr");
    auto renewed=f.reply(5,0x0a000264,f.xid()); f.inject(renewed);
    check(f.status().flags==DHCP_BOUND&&f.port.generation()==generation,"renewal extends without changing configuration");
    f.tick(4100); check(f.status().flags==DHCP_RENEWING,"second T1");
    f.tick(8100); check(f.status().flags==DHCP_REBINDING&&get32(f.media.bytes+30)==0xffffffff&&get32(f.media.bytes+26)==0x0a000264,"T2 broadcast retains leased source");
    auto rebound=f.reply(5,0x0a000264,f.xid(),0x0a000209); f.inject(rebound);
    check(f.status().flags==DHCP_BOUND&&f.status().metric==0x0a000209,"rebind accepts new server");
    f.tick(10100); f.tick(14100); f.tick(16100);
    check(f.status().flags==DHCP_SELECTING&&f.port.address_count()==1&&f.port.owns(0x0a0002c8)&&!f.port.owns(0x0a000264),"expiry preserves manual alias");
    bool manual=false;
    for (unsigned i=0;;++i) { r=request(); r.index=i; if (!f.manager.execute(NETCTL_ROUTE,r)) break; if (r.address==0xcb007100) manual=true; check(!(r.flags&NET_CONFIG_DHCP),"expiry removes only owned routes"); }
    check(manual&&!f.routes.lookup(0xcb007101,selection,0x0a000264),"manual preferred-source route retained but inactive");
    auto next=f.reply(2,0x0a000265,f.xid()); f.inject(next); f.tick(16200);
    auto nak=f.reply(6,0,f.xid()); f.inject(nak); check(f.status().flags==DHCP_SELECTING&&!f.port.owns(0x0a000265),"NAK restarts discovery");
    f.tick(17200); auto offer2=f.reply(2,0x0a000265,f.xid()); f.inject(offer2); f.tick(17300);
    auto ack2=f.reply(5,0x0a000265,f.xid()); f.inject(ack2); check(f.status().flags==DHCP_BOUND,"reacquisition");
    r=request(); check(f.dhcp.execute(NETCTL_DHCP_STOP,r)==0&&f.status().flags==DHCP_OFF&&f.port.address_count()==1,"stop preserves static configuration");
    check(!f.port.dhcp_active(),"stop releases interface ownership");
  }
  check(net_test_pages()==baseline,"DHCP state machine owns no leaked pages");
  {
    Fixture f; f.start(); f.tick(0); auto xid=f.xid();
    auto offer=f.reply(2,0x0a000264,xid); f.inject(offer); f.tick(100);
    for (unsigned i=0;i<4;++i) { auto r=request(); r.address=0x0a0002c0+i; r.prefix=24; check(f.manager.execute(NETCTL_ADD_ADDRESS,r)==0,"fill address capacity while selecting"); }
    auto ack=f.reply(5,0x0a000264,xid); f.inject(ack);
    check(f.status().flags==DHCP_SELECTING&&f.status().index==105&&f.port.address_count()==4&&!f.port.owns(0x0a000264),"lease transaction capacity rollback");
  }
  {
    Fixture f; f.start(); f.tick(0); auto xid=f.xid();
    unsigned char valid[300]; auto reply=f.reply(2,0x0a000264,xid); std::memcpy(valid,reply.data()+42,300);
    DhcpReply decoded; check(DhcpCodec::decode(valid,300,decoded),"valid codec");
    check(!DhcpCodec::decode(valid,239,decoded),"short BOOTP");
    valid[240]=254; valid[241]=255; check(!DhcpCodec::decode(valid,300,decoded),"TLV length overflow");
    auto retry=f.reply(2,0x0a000264,xid); auto *payload=retry.data()+42;
    payload[285]=53; payload[286]=1; payload[287]=2; payload[288]=255;
    check(!DhcpCodec::decode(payload,300,decoded),"duplicate message type rejected");
    net_test_fail_after(0); f.tick(2000); net_test_fail_after(-1);
    check(f.status().flags==DHCP_SELECTING,"allocation failure remains retryable");
    for (unsigned long now=40000;now<=240000;now+=40000) f.tick(now);
    check(f.status().flags==DHCP_FAILED||f.status().flags==DHCP_SELECTING,"bounded retry and delayed restart");
  }

  {
    Fixture f(true); auto r=request(3); r.address=0x0a000264; r.prefix=24;
    check(f.manager.execute(NETCTL_ADD_ADDRESS,r)==0,"existing address on another interface");
    f.start(); f.tick(0); auto offer=f.reply(2,0x0a000264,f.xid()); f.inject(offer); f.tick(100);
    auto ack=f.reply(5,0x0a000264,f.xid()); f.inject(ack);
    check(f.status().flags==DHCP_SELECTING&&f.status().index==98&&!f.port.address_count()&&f.other.owns(0x0a000264),"local address conflict leaves existing configuration intact");
    check(f.media.bytes[284]==4,"conflicting offer declined");
  }
  {
    Fixture f(true); f.start(); auto r=request(3); check(f.dhcp.execute(NETCTL_DHCP_START,r)==0,"second client"); f.tick(0);
    unsigned first=f.xid(),second=get32(f.other_media.bytes+46);
    auto misplaced=f.reply(2,0x0a000264,first); auto length=misplaced.size();
    f.other.receiver().receive_frame(misplaced.release(),length);
    r=request(3); f.dhcp.execute(NETCTL_DHCP_STATUS,r); check(r.flags==DHCP_SELECTING,"reply cannot cross interfaces");
    auto offer=f.reply(2,0x0a000264,first); f.inject(offer);
    auto offer2=f.reply(2,0x0a000265,second); offer2.data()[42+33]=0x61;
    length=offer2.size(); f.other.receiver().receive_frame(offer2.release(),length); f.tick(100);
    auto ack=f.reply(5,0x0a000264,first); f.inject(ack);
    auto ack2=f.reply(5,0x0a000265,second); ack2.data()[42+33]=0x61;
    length=ack2.size(); f.other.receiver().receive_frame(ack2.release(),length);
    r=request(3); f.dhcp.execute(NETCTL_DHCP_STATUS,r);
    check(f.status().flags==DHCP_BOUND&&r.flags==DHCP_BOUND&&r.address==0x0a000265,"independent transactions on multiple interfaces");
    r=request(); f.dhcp.execute(NETCTL_DHCP_STOP,r);
    check(f.other.owns(0x0a000265)&&f.other.dhcp_active(),"stopping one lease preserves another");
  }
  {
    Fixture f; f.start(); f.tick(0); auto xid=f.xid(); auto offer=f.reply(2,0x0a000264,xid); f.inject(offer); f.tick(100);
    auto ack=f.reply(5,0x0a000264,xid); put32(ack.data()+42+269,0xffffffff); f.inject(ack);
    f.tick(1000000000UL); check(f.status().flags==DHCP_BOUND&&f.port.owns(0x0a000264),"infinite lease does not renew or expire");
    DhcpReply decoded; auto sample=f.reply(2,0x0a000264,xid);
    check(DhcpCodec::decode(sample.data()+42,300,decoded),"option parser reference");
    DhcpLease lease; decoded.mask=0xff00ff00; check(!DhcpCodec::lease(decoded,lease),"noncontiguous mask rejected");
    decoded.mask=0xfffffffe; decoded.gateway=0x0a000265; check(DhcpCodec::lease(decoded,lease)&&lease.prefix==31,"point-to-point prefix is not a broadcast subnet");
    auto *d=sample.data()+42; std::memset(d+240,0,60); std::memset(d+108,0,128);
    d[240]=53; d[241]=1; d[242]=2; d[243]=54; d[244]=4; put32(d+245,0x0a000202);
    d[249]=52; d[250]=1; d[251]=1; d[252]=255;
    d[108]=1; d[109]=4; put32(d+110,0xffffff00); d[114]=51; d[115]=4; put32(d+116,8); d[120]=255;
    check(DhcpCodec::decode(d,300,decoded)&&DhcpCodec::lease(decoded,lease),"overloaded BOOTP file options");
  }

  {
    Fixture f; f.start(); f.tick(0); auto xid=f.xid();
    auto r=request(); r.address=0xc0000201; r.prefix=24;
    check(f.manager.execute(NETCTL_ADD_ADDRESS,r)==0,"manual alias precedes leased address");
    auto offer=f.reply(2,0x0a000264,xid); f.inject(offer); f.tick(100);
    auto ack=f.reply(5,0x0a000264,xid); f.inject(ack);
    RouteSelection selection;
    check(f.routes.lookup(0x08080808,selection)&&selection.source==0x0a000264,"DHCP default route prefers leased source");
  }
  check(net_test_pages()==baseline,"DHCP failure paths release every page");
  std::puts("DHCP: codec, ownership, renewal, rebinding, expiry and rollback passed");
}
