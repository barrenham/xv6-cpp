#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include "kernel/net/filter/decoder.h"
#include "kernel/net/filter/engine.h"
import xv.net;
namespace
{
using xv::net::filter::Engine;
using xv::net::filter::Metadata;
using BuiltinChain = Engine::BuiltinChain;
void check(bool okay,const char *why)
{ if (!okay) { std::fprintf(stderr,"firewall FAIL: %s\n",why); std::exit(1); } }
firewall_request request(const char *name="INPUT")
{ firewall_request r{}; r.version=FW_VERSION; r.size=sizeof(r); std::strcpy(r.chain,name); return r; }
int append(Engine &engine,const char *chain,unsigned action,const char *target="")
{ auto r=request(chain); r.rule.action=action; std::strcpy(r.rule.target,target); return engine.execute(FW_APPEND,r); }
}
extern "C" void run_firewall_contracts()
{
  Engine engine;
  Metadata p{0x0a000202,0x0a00020f,17,9000,2000,2,0,33,true};
  check(engine.accept(BuiltinChain::Input,p),"initial accept policy");
  auto r=request(); r.version=2; check(engine.execute(FW_FLUSH,r)==-22,"version validation");
  r=request(); r.size=0; check(engine.execute(FW_FLUSH,r)==-22,"size validation");
  r=request(); r.reserved[1]=1; check(engine.execute(FW_FLUSH,r)==-22,"reserved ABI validation");
  r=request(); r.policy=FW_DROP; check(!engine.execute(FW_POLICY,r) && !engine.accept(BuiltinChain::Input,p),"drop policy");
  r=request("SERVICES"); check(!engine.execute(FW_NEW_CHAIN,r),"create user chain");
  r=request("UDP"); check(!engine.execute(FW_NEW_CHAIN,r),"create nested chain");
  check(!append(engine,"INPUT",FW_JUMP,"SERVICES") && !append(engine,"SERVICES",FW_JUMP,"UDP"),"nested jumps");
  check(append(engine,"UDP",FW_JUMP,"SERVICES")==-22 && append(engine,"UDP",FW_JUMP,"UDP")==-22,"reject cycles");
  check(append(engine,"UDP",FW_JUMP,"OUTPUT")==-22 && append(engine,"UDP",FW_JUMP,"MISSING")==-22,"reject builtin or missing target");
  r=request("UDP"); r.rule={0x0a000200,0x0a00020f,24,32,17,8999,9001,2000,2000,2,0,FW_ACCEPT,{}};
  check(!engine.execute(FW_APPEND,r) && engine.accept(BuiltinChain::Input,p),"CIDR protocol port range interface matching");
  auto miss=p; miss.source_port=9002; check(!engine.accept(BuiltinChain::Input,miss),"port mismatch returns to policy");
  miss=p; miss.input_interface=3; check(!engine.accept(BuiltinChain::Input,miss),"interface mismatch");
  miss=p; miss.destination=0x0a000210; check(!engine.accept(BuiltinChain::Input,miss),"destination mismatch");
  miss=p; miss.source=0x0a000302; check(!engine.accept(BuiltinChain::Input,miss),"source mismatch");
  miss=p; miss.protocol=1; miss.has_ports=false; check(!engine.accept(BuiltinChain::Input,miss),"protocol mismatch");
  r=request("UDP"); check(engine.execute(FW_RULE,r)==1 && r.packets==1 && r.bytes==33,"matching counters only");
  r=request("SERVICES"); check(engine.execute(FW_DELETE_CHAIN,r)==-16,"referenced nonempty chain busy");
  r=request("UDP"); check(!engine.execute(FW_FLUSH,r) && engine.execute(FW_DELETE_CHAIN,r)==-16,"referenced empty chain busy");
  check(!append(engine,"UDP",FW_RETURN) && !append(engine,"SERVICES",FW_ACCEPT),"explicit return");
  check(engine.accept(BuiltinChain::Input,p),"return continues caller's next rule");
  r=request(); r.rule.action=FW_DROP; check(!engine.execute(FW_INSERT,r) && !engine.accept(BuiltinChain::Input,p),"insert takes priority");
  r=request(); check(!engine.execute(FW_DELETE,r) && engine.accept(BuiltinChain::Input,p),"delete restores order");
  r=request(); check(!engine.execute(FW_FLUSH,r),"flush input");
  check(!append(engine,"INPUT",FW_RETURN) && !engine.accept(BuiltinChain::Input,p),"builtin RETURN uses policy");
  r=request(); check(!engine.execute(FW_ZERO,r),"zero counters");
  check(engine.execute(FW_CHAIN,r)==1 && r.packets==0,"zero policy counters");
  r=request("UDP"); r.policy=FW_DROP; check(engine.execute(FW_POLICY,r)==-22,"user chain has no policy");
  r=request(); engine.execute(FW_FLUSH,r);
  r=request(); r.rule.action=FW_DROP; r.rule.source_prefix=33; check(engine.execute(FW_APPEND,r)==-22,"reject CIDR overflow");
  r=request(); r.rule.action=FW_DROP; r.rule.dport_high=80; check(engine.execute(FW_APPEND,r)==-22,"ports require TCP or UDP");
  for (unsigned i=0;i<FW_MAX_RULES;++i) check(!append(engine,"INPUT",FW_ACCEPT),"bounded rule fill");
  check(append(engine,"INPUT",FW_DROP)==-28 && engine.accept(BuiltinChain::Input,p),"capacity failure preserves rules");
  r=request("SERVICES"); engine.execute(FW_FLUSH,r); r=request("UDP"); engine.execute(FW_FLUSH,r);
  check(!engine.execute(FW_DELETE_CHAIN,r),"delete unreferenced empty chain");
  for (unsigned i=0;i<7;++i)
  { r=request(); std::snprintf(r.chain,sizeof(r.chain),"C%u",i); check(!engine.execute(FW_NEW_CHAIN,r),"bounded chain fill"); }
  r=request("EXCESS"); check(engine.execute(FW_NEW_CHAIN,r)==-28,"chain capacity");
  unsigned char raw[48]{}; raw[0]=0x46; raw[2]=0; raw[3]=32; raw[9]=17; raw[24]=0x23; raw[25]=0x28; raw[26]=7; raw[27]=0xd0; raw[29]=8;
  Metadata decoded{};
  check(xv::net::filter::decode(raw,32,2,0,decoded) && decoded.source_port==9000 && decoded.destination_port==2000,"IPv4 options locate ports");
  raw[6]=0x20; check(!xv::net::filter::decode(raw,32,2,0,decoded),"unreassembled fragments rejected"); raw[6]=0;
  raw[29]=9; check(!xv::net::filter::decode(raw,32,2,0,decoded),"malformed UDP length rejected");
  for (unsigned i=0;i<32;++i) check(!xv::net::filter::decode(raw,i,2,0,decoded),"truncated decoder input");

  // Selecting an OUTPUT entry must not reuse INPUT's policy or counters.
  Engine directions;
  r=request(); r.policy=FW_DROP;
  check(!directions.execute(FW_POLICY,r),"set input-only drop policy");
  check(!directions.accept(BuiltinChain::Input,p) && directions.accept(BuiltinChain::Output,p),"independent builtin entry points");
  r=request(); r.index=1;
  check(directions.execute(FW_CHAIN,r)==1 && !std::strcmp(r.chain,"OUTPUT") && r.packets==1 && r.bytes==33,"output policy counters");
  check(!directions.accept(static_cast<BuiltinChain>(99),p),"invalid builtin entry rejected");
  r=request(); r.index=1;
  check(directions.execute(FW_CHAIN,r)==1 && r.packets==1,"invalid entry does not count");

  // TCP options use the IPv4 IHL and the TCP data offset independently.
  unsigned char tcp[48]{};
  tcp[0]=0x46; tcp[3]=48; tcp[9]=6;
  tcp[24]=0x23; tcp[25]=0x28; tcp[26]=7; tcp[27]=0xd0; tcp[36]=0x60;
  check(xv::net::filter::decode(tcp,sizeof(tcp),0,3,decoded) && decoded.has_ports &&
        decoded.source_port==9000 && decoded.destination_port==2000 && decoded.output_interface==3 &&
        decoded.input_interface==0 && decoded.total_length==48,"TCP metadata after IPv4 options");
  tcp[36]=0x70;
  check(!xv::net::filter::decode(tcp,sizeof(tcp),0,3,decoded),"TCP header beyond payload rejected");
  tcp[36]=0x40;
  check(!xv::net::filter::decode(tcp,sizeof(tcp),0,3,decoded),"TCP header below minimum rejected");
  tcp[9]=1;
  check(xv::net::filter::decode(tcp,sizeof(tcp),2,0,decoded) && !decoded.has_ports &&
        decoded.source_port==0 && decoded.destination_port==0,"nontransport metadata clears previous ports");

  Engine expensive;
  r=request("A"); check(!expensive.execute(FW_NEW_CHAIN,r),"budget chain A");
  r=request("B"); check(!expensive.execute(FW_NEW_CHAIN,r),"budget chain B");
  for (unsigned i=0;i<FW_MAX_RULES;++i)
    check(!append(expensive,"INPUT",FW_JUMP,"A") && !append(expensive,"A",FW_JUMP,"B"),"repeated acyclic jumps");
  check(!expensive.accept(BuiltinChain::Input,p),"work budget fails closed");

  // Exercise the real singleton's lock with simultaneous control calls.
  auto work=[] { for (unsigned i=0;i<2000;++i) { auto q=request(); check(!xv::net::firewall_control(FW_ZERO,q),"concurrent zero"); q=request(); check(xv::net::firewall_control(FW_CHAIN,q)==1,"concurrent query"); } };
  std::thread a(work),b(work),c(work); work(); a.join(); b.join(); c.join();

  // Real IPv4 send and receive hooks, with asynchronous loopback delivery.
  auto *socket=xv::net::create_socket(2,17); check(socket && !socket->bind({0x7f000001,2190}),"loopback socket");
  xv::net::Endpoint peer{0x7f000001,2190}; char data[4];
  auto send=[&] { return socket->send_to({reinterpret_cast<unsigned long>("x"),false},1,&peer); };
  auto recv=[&] { return socket->receive_from({reinterpret_cast<unsigned long>(data),true},sizeof(data),nullptr); };
  r=request("OUTPUT"); r.rule.action=FW_DROP; check(!xv::net::firewall_control(FW_APPEND,r) && send()==-13,"OUTPUT blocks actual socket send");
  r=request("OUTPUT"); xv::net::firewall_control(FW_FLUSH,r);
  r=request(); r.rule.action=FW_DROP; check(!xv::net::firewall_control(FW_APPEND,r) && send()==1,"INPUT does not fail sending");
  xv::net::timer_tick(); check(recv()==-11,"INPUT blocks actual receive");
  r=request(); check(xv::net::firewall_control(FW_RULE,r)==1 && r.packets==1 && r.bytes==29,"actual hook counters");
  r=request(); xv::net::firewall_control(FW_FLUSH,r);
  check(send()==1,"restored send"); xv::net::timer_tick(); check(recv()==1 && data[0]=='x',"restored receive"); socket->close();
  std::puts("firewall contract: all passed");
}
