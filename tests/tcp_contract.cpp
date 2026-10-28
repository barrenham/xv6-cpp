#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
import xv.net;
extern "C" void run_tcp_buffer_contracts();
extern "C" void run_tcp_registry_contracts();
extern "C" void tcp_test_tick(xv::net::Socket *,unsigned long);
extern "C" void *net_test_allocate();
extern "C" void net_test_deliver(void *,unsigned);
extern "C" void net_test_capture(void *,unsigned *);
extern "C" int net_test_pages();
extern "C" void net_test_clock(unsigned long);
extern "C" void net_test_fail_after(int);
extern "C" void net_test_clear_frames();
extern "C" unsigned net_test_frame_count();
extern "C" unsigned net_test_frame(unsigned,void *);
namespace
{
void check(bool good,const char *message)
{ if (!good) { std::fprintf(stderr,"TCP FAIL: %s\n",message); std::exit(1); } }
unsigned word(const unsigned char *p) { return unsigned(p[0])*256+p[1]; }
unsigned number(const unsigned char *p) { return word(p)*65536+word(p+2); }
void put16(unsigned char *p,unsigned value) { p[0]=value>>8; p[1]=value; }
void put32(unsigned char *p,unsigned value) { put16(p,value>>16); put16(p+2,value); }
unsigned checksum(const unsigned char *p,unsigned n)
{ unsigned sum=0; while (n>=2) { sum+=word(p); p+=2; n-=2; } if (n) sum+=unsigned(*p)*256; while (sum>>16) sum=(sum&65535)+(sum>>16); return (~sum)&65535; }
bool before(unsigned a,unsigned b) { return int(a-b)<0; }
unsigned long clock_ms=300000;
void advance(unsigned amount) { clock_ms+=amount; net_test_clock(clock_ms); xv::net::timer_tick(); }
unsigned long address(const void *p) { return reinterpret_cast<unsigned long>(p); }
struct Frame
{
  unsigned char bytes[4096]; unsigned size;
  unsigned seq() const { return number(bytes+38); }
  unsigned ack() const { return number(bytes+42); }
  unsigned flags() const { return bytes[47]; }
  unsigned window() const { return word(bytes+48); }
  unsigned data_size() const { return word(bytes+16)-20-(bytes[46]>>4)*4; }
};
std::vector<Frame> frames()
{
  std::vector<Frame> result;
  for (unsigned i=0;i<net_test_frame_count();++i)
  {
    Frame frame{}; frame.size=net_test_frame(i,frame.bytes);
    if (frame.size>=54 && word(frame.bytes+12)==0x800 && frame.bytes[23]==6) result.push_back(frame);
  }
  return result;
}
struct Peer
{
  xv::net::Socket *socket;
  Frame syn{};
  unsigned base, next, receive, advertised=16384;
  explicit Peer(bool handshake=true,unsigned remote_sequence=4000,unsigned peer_window=16384)
  {
    net_test_clock(clock_ms); net_test_clear_frames();
    socket=xv::net::create_socket(1,6); check(socket,"allocate TCP socket");
    check(socket->connect({0x0a000202,9000},true)==-115,"active open");
    net_test_capture(syn.bytes,&syn.size); check(syn.size==58 && syn.flags()==2,"SYN with MSS");
    base=next=syn.seq()+1; receive=remote_sequence+1; advertised=peer_window;
    if (handshake) { send(remote_sequence,base,18,nullptr,0,peer_window,true); check(socket->poll(nullptr)&4,"established"); }
    net_test_clear_frames();
  }
  void send(unsigned seq,unsigned ack,unsigned flags,const void *data=nullptr,unsigned n=0,unsigned window=16384,bool option=false)
  {
    unsigned header=option?24:20;
    std::vector<unsigned char> bytes(34+header+n,0);
    auto *p=bytes.data(); std::memcpy(p,syn.bytes,34); std::memcpy(p,syn.bytes+6,6); std::memcpy(p+6,syn.bytes,6);
    std::memcpy(p+26,syn.bytes+30,4); std::memcpy(p+30,syn.bytes+26,4);
    put16(p+34,word(syn.bytes+36)); put16(p+36,word(syn.bytes+34)); put32(p+38,seq); put32(p+42,ack);
    p[46]=(header/4)<<4; p[47]=flags; put16(p+48,window);
    if (option) { p[54]=2; p[55]=4; put16(p+56,1460); }
    if (n) std::memcpy(p+34+header,data,n);
    std::vector<unsigned char> pseudo(12+header+n,0); std::memcpy(pseudo.data(),p+26,8); pseudo[9]=6;
    put16(pseudo.data()+10,header+n); std::memcpy(pseudo.data()+12,p+34,header+n); put16(p+50,checksum(pseudo.data(),pseudo.size()));
    put16(p+16,20+header+n); p[24]=p[25]=0; put16(p+24,checksum(p+14,20));
    void *page=net_test_allocate(); check(page,"inject allocation"); std::memcpy(page,p,bytes.size()); net_test_deliver(page,bytes.size());
  }
  unsigned sent()
  {
    for (const auto &frame:frames())
    { unsigned end=frame.seq()+frame.data_size()+((frame.flags()&1)?1:0); if (before(next,end)) next=end; }
    return next;
  }
  void ack(unsigned value,unsigned window=16384) { send(receive,value,16,nullptr,0,window); }
  void drain(unsigned bytes)
  {
    unsigned end=base+bytes;
    for (unsigned guard=0;guard<100 && before(base,end);++guard)
    {
      unsigned emitted=sent(); if (emitted==base) std::fprintf(stderr,"drain stalled base=%u target=%u next=%u frames=%u poll=%u\n",base,end,next,net_test_frame_count(),socket->poll(nullptr)); check(emitted!=base,"sender makes progress");
      ack(emitted); base=emitted; sent(); net_test_clear_frames();
    }
    check(base==end,"all queued bytes acknowledged");
  }
  void reset()
  {
    send(receive,sent(),20); socket->close(); socket=nullptr;
    check(net_test_pages()==0,"RST close releases all stream pages");
  }
};
void basic_send()
{
  {
    Peer timing;
    check(timing.socket->write({address("clock"),true},5)==5,"queue stale timer fixture");
    tcp_test_tick(timing.socket,clock_ms-1);
    check(!(timing.socket->poll(nullptr)&8),"stale timer snapshot cannot underflow elapsed time");
    timing.drain(5); timing.reset();
  }
  Peer peer;
  unsigned char data[1024]; for (unsigned i=0;i<sizeof(data);++i) data[i]=i*13;
  unsigned accepted=0;
  for (;;)
  {
    int n=peer.socket->write({address(data),true},sizeof(data));
    if (n==-11) break;
    check(n>0,"queue write"); accepted+=n;
  }
  check(accepted==16384 && !(peer.socket->poll(nullptr)&4),"send buffer backpressure and poll");
  auto sent=frames(); unsigned count=0, flight=0;
  for (const auto &frame:sent) if (frame.data_size()) { ++count; flight+=frame.data_size(); }
  check(count>=2 && flight<=2920 && flight>1460,"multiple segments in flight limited by initial congestion window");
  unsigned next=peer.sent(); peer.ack(next+1000);
  check(peer.socket->write({address(data),true},1)==-11,"future ACK cannot free unsent data");
  net_test_clear_frames(); peer.ack(peer.base+500);
  check(peer.socket->poll(nullptr)&4,"partial ACK publishes writable space");
  check(peer.socket->write({address(data),true},sizeof(data))==500,"write consumes precisely the space released by ACK");
  unsigned target=peer.base+accepted+500; peer.base+=500;
  peer.drain(target-peer.base); peer.reset();
}
void loss_recovery()
{
  Peer peer; unsigned char data[1024]; std::memset(data,0x5a,sizeof(data));
  for (unsigned i=0;i<10;++i) check(peer.socket->write({address(data),true},sizeof(data))==sizeof(data),"queue pipeline for loss");
  unsigned first=peer.base; peer.sent(); net_test_clear_frames();
  peer.ack(first); peer.ack(first); check(frames().empty(),"two duplicate ACKs do not retransmit");
  peer.ack(first); auto retransmitted=frames();
  check(!retransmitted.empty() && retransmitted[0].seq()==first && retransmitted[0].data_size(),"third duplicate ACK retransmits oldest without RTO");
  peer.sent(); net_test_clear_frames(); peer.ack(first+1024); retransmitted=frames();
  check(!retransmitted.empty() && retransmitted[0].seq()==first+1024,"NewReno partial ACK repairs next loss");
  peer.base=first+1024; peer.drain(9*1024); peer.reset();
  Peer timeout;
  check(timeout.socket->write({address("hello"),true},5)==5,"RTO data queued");
  timeout.sent(); net_test_clear_frames(); advance(1001); retransmitted=frames();
  check(!retransmitted.empty() && retransmitted[0].seq()==timeout.base && retransmitted[0].data_size()==5,"RTO retransmits retained bytes");
  timeout.ack(timeout.base+5); timeout.base+=5; timeout.reset();
  Peer reset;
  check(reset.socket->write({address("reset"),true},5)==5,"queue before peer reset");
  reset.send(reset.receive,reset.sent(),20); net_test_clear_frames(); advance(4000);
  check(frames().empty() && reset.socket->error()==104,"closed connection cannot retransmit after RST");
  reset.socket->close(); reset.socket=nullptr; check(!net_test_pages(),"reset idle timer storage released");
}
void receive_reordering()
{
  Peer peer(true,0xffffff00U);
  unsigned char bytes[2500],output[2500]; for (unsigned i=0;i<sizeof(bytes);++i) bytes[i]=i*7;
  peer.send(peer.receive+2000,peer.base,24,bytes+2000,500);
  check(peer.socket->read({address(output),true},sizeof(output))==-11,"out of order data not exposed early");
  peer.send(peer.receive,peer.base,24,bytes,1000); peer.send(peer.receive+1000,peer.base,24,bytes+1000,1000); peer.receive+=2500;
  unsigned done=0;
  while (done<sizeof(output)) { int n=peer.socket->read({address(output+done),true},sizeof(output)-done); check(n>0,"drain reordered stream"); done+=n; }
  check(!std::memcmp(bytes,output,sizeof(bytes)),"sequence wrap and out of order data recovered without retransmission");
  peer.send(peer.receive-2500,peer.base,24,bytes,1000);
  check(peer.socket->read({address(output),true},1)==-11,"duplicate bytes not delivered");
  unsigned char block[1000]; std::memset(block,1,sizeof(block));
  unsigned remaining=16384;
  while (remaining) { unsigned n=remaining>sizeof(block)?sizeof(block):remaining; peer.send(peer.receive,peer.base,24,block,n); peer.receive+=n; remaining-=n; }
  auto emitted=frames(); check(!emitted.empty() && emitted.back().window()==0,"full receive buffer advertises zero window");
  net_test_clear_frames(); check(peer.socket->read({address(output),true},1024)==1024,"read from full buffer");
  emitted=frames(); check(!emitted.empty() && emitted.back().window()>0,"read reopens zero window");
  peer.send(peer.receive+1,peer.base,4); check(!(peer.socket->poll(nullptr)&8),"inexact reset is challenged"); peer.reset();
}
void persist_and_window()
{
  Peer peer(true,8000,0);
  check(peer.socket->write({address("abc"),true},3)==3,"zero peer window still admits application queue");
  check(frames().empty(),"zero window stops ordinary send");
  advance(1000); auto probe=frames(); check(!probe.empty() && probe.back().data_size()==1 && probe.back().seq()==peer.base,"zero window data probe");
  peer.sent(); net_test_clear_frames(); peer.ack(peer.base,0); advance(1000);
  probe=frames(); check(!probe.empty() && probe.back().seq()==peer.base,"persist repeats earliest unacknowledged byte");
  net_test_clear_frames(); peer.ack(peer.base,1000); auto reopened=frames();
  check(!reopened.empty() && reopened[0].seq()==peer.base,"window reopening immediately repairs rejected probe");
  peer.ack(peer.base+3,1000); peer.base+=3; peer.sent();
  net_test_clear_frames(); peer.ack(peer.base-1,0);
  check(peer.socket->write({address("new"),true},3)==3,"write after stale ACK");
  auto output=frames(); bool data=false; for (const auto &frame:output) data|=frame.data_size()!=0;
  check(data,"stale ACK does not replace newer peer window");
  peer.drain(3); peer.reset();
}
void teardown()
{
  Peer peer; unsigned char block[1000]; std::memset(block,7,sizeof(block));
  for (unsigned i=0;i<6;++i) check(peer.socket->write({address(block),true},sizeof(block))==sizeof(block),"queue before shutdown");
  check(!peer.socket->shutdown(1),"half close with queued data");
  for (const auto &frame:frames()) check(!(frame.flags()&1),"FIN waits behind unsent data");
  unsigned start=peer.base; bool fin=false;
  for (unsigned i=0;i<100 && !fin;++i)
  {
    unsigned sent=peer.sent(); peer.ack(sent);
    for (const auto &frame:frames()) if (frame.flags()&1) { check(frame.seq()==start+6000,"FIN follows the entire queued stream"); fin=true; }
  }
  check(fin,"queued half close eventually sends FIN");
  peer.send(peer.receive,start+6001,17); ++peer.receive;
  unsigned char byte{}; check(peer.socket->read({address(&byte),true},1)==0,"peer FIN produces EOF");
  peer.socket->close(); peer.socket=nullptr; advance(31000); check(!net_test_pages(),"TIME_WAIT releases stream storage");
  Peer passive;
  passive.send(passive.receive,passive.base,17); ++passive.receive;
  check(passive.socket->write({address("tail"),true},4)==4,"CLOSE_WAIT permits final response");
  passive.sent(); check(!passive.socket->shutdown(1),"CLOSE_WAIT shutdown");
  passive.ack(passive.base+5); passive.socket->close(); passive.socket=nullptr; check(!net_test_pages(),"LAST_ACK closes after FIN acknowledgement");
  Peer simultaneous;
  check(!simultaneous.socket->shutdown(1),"simultaneous local FIN");
  simultaneous.send(simultaneous.receive,simultaneous.base,17); ++simultaneous.receive;
  simultaneous.ack(simultaneous.base+1); simultaneous.socket->close(); simultaneous.socket=nullptr;
  advance(31000); check(!net_test_pages(),"simultaneous close transitions through CLOSING and TIME_WAIT");
}
void handshakes()
{
  Peer retransmission(false); advance(1001); auto emitted=frames();
  check(!emitted.empty() && emitted.back().flags()==2 && emitted.back().seq()==retransmission.base-1,"SYN retransmits same sequence");
  retransmission.send(retransmission.receive-1,retransmission.base,18,nullptr,0,16384,true);
  check(retransmission.socket->poll(nullptr)&4,"connection recovers from SYN loss"); retransmission.reset();
  Peer simultaneous(false,12000);
  simultaneous.send(simultaneous.receive-1,0,2,nullptr,0,16384,true);
  net_test_clear_frames(); simultaneous.send(simultaneous.receive-1,simultaneous.base,18,nullptr,0,16384,true);
  emitted=frames(); check((simultaneous.socket->poll(nullptr)&4) && !emitted.empty() && emitted.back().flags()==16,"simultaneous open completes with final ACK"); simultaneous.reset();
  Peer timeout(false);
  for (unsigned i=0;i<200;++i) advance(1000);
  check(timeout.socket->error()==110,"unanswered SYN times out"); timeout.socket->close(); timeout.socket=nullptr;
  check(!net_test_pages(),"handshake timeout releases resources");
}
}
extern "C" void run_tcp_contracts()
{
  net_test_clock(clock_ms); run_tcp_buffer_contracts(); check(!net_test_pages(),"buffer contracts release all pages");
  run_tcp_registry_contracts();
  for (int count=0;count<9;++count)
  { net_test_fail_after(count); check(!xv::net::create_socket(1,6) && !net_test_pages(),"all partial stream allocations roll back"); }
  net_test_fail_after(-1);
  std::puts("TCP testing pipeline"); basic_send();
  std::puts("TCP testing loss"); loss_recovery();
  std::puts("TCP testing receive"); receive_reordering();
  std::puts("TCP testing persist"); persist_and_window();
  std::puts("TCP testing teardown"); teardown();
  std::puts("TCP testing handshakes"); handshakes();
  std::puts("TCP contract: sliding windows, NewReno, persist, reordering, FIN and lifecycle passed");
}
