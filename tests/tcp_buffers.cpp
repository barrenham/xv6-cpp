module;
#include <cstdio>
#include <cstdlib>
#include <cstring>
module xv.net;
import :tcp_socket;
import :stream_buffer;
import :congestion;
import :retransmission_timer;
namespace
{
void check(bool good, const char *message)
{ if (!good) { std::fprintf(stderr,"TCP buffer FAIL: %s\n",message); std::exit(1); } }
}
extern "C" void run_tcp_buffer_contracts()
{
  using namespace xv::net;
  TcpSendBuffer tx; TcpReceiveBuffer rx;
  check(tx.initialize() && rx.initialize(),"buffer allocation");
  unsigned base = 0xfffffff0;
  unsigned char bytes[TcpPageBuffer::Capacity], output[TcpPageBuffer::Capacity];
  for (unsigned i = 0; i < sizeof(bytes); ++i) bytes[i] = i*19+7;
  tx.reset(base);
  check(tx.append(bytes,sizeof(bytes)) == sizeof(bytes) && !tx.space(),"bounded TX ring across sequence wrap");
  unsigned offset = 0;
  while (offset < sizeof(bytes))
  { unsigned amount = sizeof(bytes)-offset; auto *span = tx.span(base+offset,amount); std::memcpy(output+offset,span,amount); offset += amount; }
  check(!std::memcmp(bytes,output,sizeof(bytes)),"paged TX spans preserve bytes");
  check(tx.discard(base+8192) == 8192 && tx.space() == 8192,"partial cumulative ACK frees TX space");
  check(tx.append(bytes,8192) == 8192 && tx.size() == sizeof(bytes),"TX ring reuse");
  check(tx.discard(tx.end()) == sizeof(bytes) && !tx.size(),"full cumulative ACK frees TX ring");
  rx.reset(base);
  check(rx.insert(base+1000,bytes+1000,1000,false) == 0 && !rx.readable(),"out of order waits for gap");
  unsigned char conflict[500]; std::memset(conflict,0xff,sizeof(conflict));
  rx.insert(base+1200,conflict,500,false);
  check(rx.insert(base,bytes,1000,false) == 2000 && rx.readable() == 2000,"gap promotes all contiguous bytes across sequence wrap");
  check(rx.read(output,2000) == 2000 && !std::memcmp(bytes,output,2000),"first arrival wins overlaps");
  check(rx.insert(base+2000,bytes,sizeof(bytes),false) == sizeof(bytes) && !rx.window(),"full RX window");
  check(!rx.acceptable(rx.next(),1) && rx.acceptable(rx.next(),0),"zero window segment acceptance");
  check(rx.read(output,4096) == 4096 && rx.window() == 4096,"application read reopens RX window");
  rx.discard(); rx.reset(50);
  rx.insert(55,nullptr,0,true); check(!rx.finished(),"out of order FIN waits for bytes");
  check(rx.insert(50,bytes,5,false) == 6 && rx.finished() && rx.readable() == 5 && rx.next() == 56,"FIN consumes sequence after gap");
  check(rx.read(output,10) == 5 && !std::memcmp(bytes,output,5),"FIN is excluded from byte stream");
  check(!rx.insert(56,bytes,10,false),"data after FIN is not delivered");
  tx.release(); rx.release();
  TcpCongestion congestion; congestion.initialize(1000);
  check(congestion.window() == 2000,"initial congestion window");
  congestion.acknowledged(1000,1000); check(congestion.window() == 3000,"slow start growth");
  check(!congestion.duplicate(1000,4000,5000) && !congestion.duplicate(1000,4000,5000) && congestion.duplicate(1000,4000,5000),"three duplicate ACKs enter fast recovery");
  check(congestion.window() == 5000,"recovery inflation");
  check(congestion.acknowledged(1000,2000),"NewReno partial ACK requests next retransmission");
  check(!congestion.acknowledged(3000,5000) && congestion.window() == 2000,"full ACK exits recovery");
  congestion.acknowledged(1000,6000); check(congestion.window() == 2000,"congestion avoidance credits partial window");
  congestion.acknowledged(1000,7000); check(congestion.window() == 3000,"additive increase");
  congestion.timeout(6000,13000); check(congestion.window() == 1000,"RTO reduces congestion window");
  for (unsigned i = 0; i < 4; ++i) check(!congestion.duplicate(7000,6000,13000),"old duplicate ACKs cannot reenter recovery after RTO");
  RetransmissionTimer timer; timer.reset(); timer.start(0); timer.sent(100,0);
  check(!timer.expired(999) && timer.expired(1000),"initial RTO");
  timer.retransmit(1000); timer.acknowledge(100,1100,false);
  check(!timer.expired(3099) && timer.expired(3100),"Karn excludes retransmitted RTT sample and retains backoff");
  timer.sent(200,4000); timer.acknowledge(200,4100,false);
  check(!timer.expired(5099) && timer.expired(5100),"fresh RTT updates timer after recovery");
  std::puts("TCP buffers: rings, reordering, wraparound, NewReno and RTO passed");
}

extern "C" void tcp_test_tick(xv::net::Socket *socket,unsigned long now)
{
  xv::sync::ScopedLock guard(xv::net::socket_registry().lock());
  static_cast<xv::net::TcpSocket *>(socket)->tick(now);
}
