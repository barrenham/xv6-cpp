#include "kernel/support/types.h"
#include "kernel/fs/poll.h"
#include "user/runtime/user.h"
#include "user/runtime/icmp/echo.h"
#include "user/runtime/parse.h"
namespace
{
volatile sig_atomic_t stopped = 0;
void interrupt(int) { stopped = 1; }
void usage() { fprintf(2, "usage: ping [-q] [-c COUNT] [-i SECONDS] [-W SECONDS] [-s BYTES] HOST\n"); }
bool number(const char *s, unsigned limit, unsigned &out)
{
  unsigned n = 0; if (!*s) return false;
  while (*s) { if (*s < '0' || *s > '9' || n > (limit - unsigned(*s-'0'))/10) return false; n = n*10 + unsigned(*s++-'0'); }
  out = n; return n <= limit;
}
void address(unsigned ip) { printf("%u.%u.%u.%u", ip>>24, (ip>>16)&255, (ip>>8)&255, ip&255); }
void milliseconds(unsigned long us)
{ printf("%lu.%c%c%c", us/1000, char('0'+us/100%10), char('0'+us/10%10), char('0'+us%10)); }
struct Workspace { unsigned char request[1472], previous[1472], reply[2048]; };
}
extern "C" int main(int argc, char **argv)
{
  unsigned count = 0, interval = 1000, timeout = 1000, payload = 56;
  bool quiet = false, positional = false; const char *host = nullptr;
  for (int i = 1; i < argc; ++i)
  {
    const char *arg = argv[i];
    if (!positional && !strcmp(arg,"--")) { positional = true; continue; }
    if (!positional && (!strcmp(arg,"--help") || !strcmp(arg,"-h"))) { usage(); exit(0); }
    if (!positional && !strcmp(arg,"-q")) { quiet = true; continue; }
    if (!positional && arg[0]=='-')
    {
      if (strlen(arg)!=2 || ++i==argc) { usage(); exit(2); }
      bool valid = false;
      switch (arg[1])
      {
        case 'c': valid = number(argv[i],65535,count) && count; break;
        case 's': valid = number(argv[i],1400,payload); break;
        case 'i': valid = xv::cli::duration_ms(argv[i],interval); break;
        case 'W': valid = xv::cli::duration_ms(argv[i],timeout); break;
      }
      if (!valid) { usage(); exit(2); }
    }
    else { if (host) { usage(); exit(2); } host = arg; }
  }
  if (!host) { usage(); exit(2); }
  struct sigaction action{interrupt,0,0};
  if (sigaction(SIGINT,&action,nullptr)<0) { fprintf(2,"ping: cannot install signal handler\n"); exit(2); }
  dns_result resolved{}; dns_options dns{}; dns.total_timeout_ms = timeout;
  int status = dns_lookup(host,&resolved,&dns);
  if (status || !resolved.count) { fprintf(2,"ping: %s\n",dns_strerror(status ? status : DNS_NO_ADDRESS)); exit(2); }
  auto *w = static_cast<Workspace *>(malloc(sizeof(Workspace)));
  if (!w) { fprintf(2,"ping: out of memory\n"); exit(2); }
  int fd = socket(AF_INET,SOCK_DGRAM|SOCK_NONBLOCK,IPPROTO_ICMP);
  sockaddr_in peer{}; peer.sin_family=AF_INET; peer.sin_addr.s_addr=htonl(resolved.addresses[0]);
  if (fd<0 || connect(fd,reinterpret_cast<sockaddr *>(&peer),sizeof(peer))<0)
  { fprintf(2,"ping: no usable ICMP route or socket\n"); if(fd>=0)close(fd); free(w); exit(2); }
  sockaddr_in local{}; socklen_t size=sizeof(local);
  if (getsockname(fd,reinterpret_cast<sockaddr *>(&local),&size)<0)
  { fprintf(2,"ping: cannot read echo identifier\n"); close(fd);free(w);exit(2); }
  unsigned identifier=ntohs(local.sin_port), length=payload+8;
  printf("PING %s (",host); address(resolved.addresses[0]); printf(") %u data bytes\n",payload);
  unsigned long sent=0,received=0,duplicates=0,minimum=~0UL,maximum=0,sum=0;
  bool previous=false; int failure=0;
  while (!stopped && (!count || sent<count))
  {
    auto start=clockticks(); unsigned sequence=unsigned(sent+1)&65535;
    xv::echo::request(w->request,length,sequence,start);
    int n=sendto(fd,w->request,int(length),0,nullptr,0);
    if(n!=int(length)) { fprintf(2,"ping: send failed (%d)\n",n);failure=2;break; }
    ++sent; bool answered=false;
    auto deadline=start+uint64(timeout)*10000;
    while (!stopped && clockticks()<deadline)
    {
      auto now=clockticks(); if(now>=deadline)break;
      pollfd wait{fd,POLLIN,0};
      int ready=poll(&wait,1,int((deadline-now+9999)/10000));
      if(ready<0) { if(!stopped)failure=2;break; }
      if(!ready)break;
      sockaddr_in from{}; socklen_t from_size=sizeof(from);
      n=recvfrom(fd,w->reply,sizeof(w->reply),MSG_DONTWAIT,reinterpret_cast<sockaddr *>(&from),&from_size);
      if(n==-11)continue;
      if(n<0) { if(!stopped)failure=2;break; }
      if(from_size!=sizeof(from) || from.sin_addr.s_addr!=peer.sin_addr.s_addr)continue;
      if(xv::echo::matches(w->reply,unsigned(n),w->request,length,identifier))
      {
        auto us=(clockticks()-start)/10;
        ++received;sum+=us;if(us<minimum)minimum=us;if(us>maximum)maximum=us;
        if(!quiet) { printf("%d bytes from ",n);address(resolved.addresses[0]);printf(": icmp_seq=%u time=",sequence);milliseconds(us);printf(" ms\n"); }
        memmove(w->previous,w->request,int(length));previous=true;answered=true;break;
      }
      if(previous && xv::echo::matches(w->reply,unsigned(n),w->previous,length,identifier))++duplicates;
    }
    if(failure || stopped)break;
    if(!answered && !quiet)printf("Request timeout for icmp_seq=%u\n",sequence);
    if(count && sent>=count)break;
    auto next=start+uint64(interval)*10000;
    while(!stopped && clockticks()<next)
    { auto now=clockticks();if(now>=next)break; if(poll(nullptr,0,int((next-now+9999)/10000))<0 && !stopped) { failure=2;break; } }
    if(failure)break;
  }
  close(fd);free(w);
  printf("\n--- %s ping statistics ---\n%lu packets transmitted, %lu received, %lu duplicates, %lu%% packet loss\n",host,sent,received,duplicates,sent ? (sent-received)*100/sent : 0);
  if(received) { printf("rtt min/avg/max = ");milliseconds(minimum);printf("/");milliseconds(sum/received);printf("/");milliseconds(maximum);printf(" ms\n"); }
  exit(failure ? failure : received ? 0 : 1);
}
