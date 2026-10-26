#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include "kernel/net/control.h"
import xv.net;
import xv.drivers;
extern "C" int net_test_pages();
extern "C" void net_test_free(void *);
extern "C" void net_test_clock(unsigned long);
extern "C" void net_test_fail_after(int);
namespace
{
void check(bool ok,const char *message)
{ if (!ok) { std::fprintf(stderr,"bridge FAIL: %s\n",message); std::exit(1); } }
class Media final : public xv::drivers::EthernetDevice
{
public:
  bool ready=true, accept=true, promisc=false, supports=true;
  unsigned count=0, length=0;
  unsigned char bytes[1600]{};
  bool online() const override { return ready; }
  bool promiscuous(bool enable) override { if (!supports) return false; promisc=enable; return true; }
  void mac_address(unsigned char *to) const override { std::memcpy(to,"\x52\x54\x00\x11\x22\x33",6); }
  bool transmit(void *page,unsigned size) override
  {
    if (!accept) return false;
    check(size<=sizeof(bytes),"frame capacity"); std::memcpy(bytes,page,size); length=size; ++count;
    net_test_free(page); return true;
  }
};
class Receiver final : public xv::net::IpReceiver
{ public: void receive(xv::net::NetworkInterface &,xv::net::Packet &) override {} };
netctl_request request(unsigned id=16)
{ netctl_request r{}; r.version=NETCTL_VERSION; r.size=sizeof(r); r.ifindex=id; return r; }
void inject(xv::net::EthernetInterface &port,unsigned source,unsigned destination,unsigned type=0x88b5)
{
  auto frame=xv::net::Packet::allocate(60); check(bool(frame),"test packet allocation");
  std::memset(frame.data(),0,60); frame.data()[0]=2; frame.data()[5]=destination;
  if (destination==255) std::memset(frame.data(),255,6);
  frame.data()[6]=2; frame.data()[11]=source; frame.data()[12]=type>>8; frame.data()[13]=type;
  std::memcpy(frame.data()+14,"preserve frame",14);
  port.receiver().receive_frame(frame.release(),60);
}
}
extern "C" void run_bridge_contracts()
{
  using namespace xv::net;
  int baseline=net_test_pages();
  InterfaceRegistry interfaces; RoutingTable routes(interfaces); Receiver receiver;
  LoopbackInterface loop; loop.initialize(); loop.add_address({0x7f000001,8}); loop.set_up(true); interfaces.add(loop);
  EthernetInterface a(2,"a"),b(3,"b"),c(4,"c"); Media ma,mb,mc;
  a.initialize(ma); b.initialize(mb); c.initialize(mc);
  check(a.set_up(true)&&b.set_up(true)&&c.set_up(true),"port configuration");
  check(interfaces.add(a)&&interfaces.add(b)&&interfaces.add(c)&&interfaces.seal(),"port registry"); routes.seal();
  BridgeManager manager(interfaces,routes,receiver); manager.initialize();
  NetworkManager network(interfaces,routes,loop,&manager);
  auto r=request(); std::memcpy(r.name,"br0",4); check(manager.execute(NETCTL_BRIDGE_CREATE,r)==0,"create bridge");
  check(manager.execute(NETCTL_BRIDGE_CREATE,r)==-17,"duplicate bridge rejected");
  auto *bridge=interfaces.find(16); check(bridge&&bridge->is_bridge()&&bridge->mac(),"bridge host interface");
  r=request(); r.source=2;
  ma.supports=false; check(manager.execute(NETCTL_BRIDGE_ADD_PORT,r)==-95&&!a.master(),"unsupported promisc rollback"); ma.supports=true;
  for (unsigned id=2;id<=4;++id)
  { r=request(); r.source=id; check(manager.execute(NETCTL_BRIDGE_ADD_PORT,r)==0,"add bridge port"); }
  check(ma.promisc&&mb.promisc&&mc.promisc&&a.master()==16,"promiscuous port binding");
  auto second=request(); std::memcpy(second.name,"br1",4);
  check(manager.execute(NETCTL_BRIDGE_CREATE,second)==0,"second independent bridge");
  second=request(17); second.source=2;
  check(manager.execute(NETCTL_BRIDGE_ADD_PORT,second)==-16,"port cannot belong to two bridges");
  second=request(17); check(manager.execute(NETCTL_BRIDGE_DELETE,second)==0,"delete unused second bridge");
  r=request(); r.source=2; check(manager.execute(NETCTL_BRIDGE_ADD_PORT,r)==-16,"duplicate membership rejected");
  r=request(); r.source=16; check(manager.execute(NETCTL_BRIDGE_ADD_PORT,r)==-95,"bridge cannot enslave itself");
  check(!routes.add(0,0,a),"enslaved port cannot be routed");
  net_test_clock(300000);
  inject(a,0xa1,0xb2); check(!ma.count&&!mb.count&&!mc.count,"bridge starts down");
  check(!bridge->set_up(true),"runtime bridge uses control API");
  r=request(); r.flags=NET_LINK_UP; check(network.execute(NETCTL_SET_LINK,r)==0,"enable bridge");
  inject(a,0xa1,0xb2); check(!ma.count&&mb.count==1&&mc.count==1,"unknown unicast floods other ports");
  check(mb.bytes[6]==2&&mb.bytes[11]==0xa1&&!std::memcmp(mb.bytes+14,"preserve frame",14),"forward preserves original MAC and payload");
  inject(b,0xb2,0xa1); check(ma.count==1&&mc.count==1,"learned unicast excludes unrelated port");
  inject(a,0xa1,0xb2); check(mb.count==2&&mc.count==1,"reverse learned destination");
  inject(c,0xb2,0xa1); inject(a,0xa1,0xb2); check(ma.count==2&&mc.count==2&&mb.count==2,"MAC movement updates port");
  inject(a,0xa1,0xa1); check(ma.count==2&&mc.count==2&&mb.count==2,"same-port destination filtered");
  inject(a,0xa1,255); check(mb.count==3&&mc.count==3&&ma.count==2,"broadcast floods without ingress reflection");
  inject(a,0xa1,255,0x8100); check(mb.count==3&&mc.count==3,"unsupported VLAN does not leak");
  r=request(); r.index=0; check(manager.execute(NETCTL_BRIDGE_FDB,r)==1,"FDB snapshot");
  r=request(); r.source=2; r.address=0x0a140002; r.prefix=24; r.ifindex=2;
  check(network.execute(NETCTL_ADD_ADDRESS,r)==-16,"member cannot acquire IP address");
  r=request(4); r.flags=0; check(network.execute(NETCTL_SET_LINK,r)==0,"disable member");
  inject(a,0xa1,0xb2); check(mc.count==3&&mb.count==3,"known down-port destination is filtered");
  r=request(4); r.flags=NET_LINK_UP; check(network.execute(NETCTL_SET_LINK,r)==0,"enable member");
  net_test_clock(600001); bridge->tick(600001);
  r=request(); check(manager.execute(NETCTL_BRIDGE_FDB,r)==0,"FDB entries age out");
  auto frame=Packet::allocate(60); std::memset(frame.data(),0,60); frame.data()[0]=2; frame.data()[5]=0xfd;
  frame.data()[6]=2; frame.data()[11]=0xa1; frame.data()[12]=0x88; frame.data()[13]=0xb5;
  net_test_fail_after(0); a.receiver().receive_frame(frame.release(),60); net_test_fail_after(-1);
  check(net_test_pages()==baseline,"clone allocation failure releases original page");
  mb.accept=false; inject(a,0xa1,0xfe); mb.accept=true;
  check(mc.count==4&&mb.count==3&&net_test_pages()==baseline,"partial forwarding failure releases failed clone");
  for (unsigned source=1;source<=70;++source) inject(a,source,0xfe);
  r=request(); r.index=64; check(manager.execute(NETCTL_BRIDGE_FDB,r)==0,"bounded MAC learning table");
  r=request(); check(manager.execute(NETCTL_BRIDGE_FLUSH_FDB,r)==0,"flush learned table");
  std::thread traffic([&] { for (unsigned i=0;i<200;++i) inject(a,0xa1,255); });
  for (unsigned i=0;i<30;++i)
  {
    auto change=request(); change.source=4;
    check(manager.execute(NETCTL_BRIDGE_DEL_PORT,change)==0&&manager.execute(NETCTL_BRIDGE_ADD_PORT,change)==0,"concurrent membership changes");
  }
  traffic.join(); check(net_test_pages()==baseline,"concurrent forwarding has no page leaks");
  r=request(); r.flags=0; check(network.execute(NETCTL_SET_LINK,r)==0,"disable bridge");
  r=request(); check(manager.execute(NETCTL_BRIDGE_DELETE,r)==-16,"bridge with members cannot be deleted");
  for (unsigned id=2;id<=4;++id)
  { r=request(); r.source=id; check(manager.execute(NETCTL_BRIDGE_DEL_PORT,r)==0,"remove ports"); }
  check(!ma.promisc&&!mb.promisc&&!mc.promisc,"detach restores receive mode");
  r=request(); check(manager.execute(NETCTL_BRIDGE_DELETE,r)==0&&!interfaces.find(16),"delete unpublished bridge");
  check(net_test_pages()==baseline,"bridge lifecycle retains no pages");
  std::puts("bridge: learning, aging, ownership and concurrent membership passed");
}
