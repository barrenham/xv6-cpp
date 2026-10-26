#include "kernel/support/types.h"
#include "kernel/fs/poll.h"
#include "kernel/net/errno.h"
#include "user/runtime/user.h"
namespace
{
unsigned char output[1024],input[1024];
void check(bool good,const char *message)
{ if (!good) { fprintf(2,"tcpduplex: FAIL: %s\n",message); exit(1); } }
}
extern "C" int main(int argc,char **argv)
{
  check(argc==2,"echo port required");
  int fd=socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0); check(fd>=0,"socket");
  sockaddr_in peer{}; peer.sin_family=AF_INET; peer.sin_addr.s_addr=htonl(0x0a000202); peer.sin_port=htons(atoi(argv[1]));
  check(connect(fd,reinterpret_cast<sockaddr *>(&peer),sizeof(peer))==-EINPROGRESS,"connect");
  pollfd event{fd,POLLOUT,0}; check(poll(&event,1,5000)==1,"connect readiness");
  int error=-1; socklen_t size=sizeof(error); check(!getsockopt(fd,SOL_SOCKET,SO_ERROR,&error,&size)&&!error,"connect result");
  constexpr unsigned Total=512*1024;
  unsigned sent=0,received=0,start=unsigned(uptime()); bool finished=false;
  while (received<Total || !finished)
  {
    check(unsigned(uptime())-start<300,"duplex deadline");
    event={fd,static_cast<short>(POLLIN|(sent<Total?POLLOUT:0)),0};
    check(poll(&event,1,5000)>0 && !(event.revents&POLLERR),"duplex readiness");
    if (event.revents&POLLOUT)
    {
      unsigned n=Total-sent; if (n>sizeof(output)) n=sizeof(output);
      for (unsigned i=0;i<n;++i) output[i]=(sent+i)*29+3;
      int wrote=write(fd,output,n); check(wrote>0 || wrote==-EAGAIN,"nonblocking write");
      if (wrote>0) sent+=wrote;
      if (sent==Total) check(!shutdown(fd,SHUT_WR),"queued half close");
    }
    if (event.revents&POLLIN)
    {
      int n=read(fd,input,sizeof(input)); check(n>=0 || n==-EAGAIN,"nonblocking read");
      if (!n) { check(received==Total && sent==Total,"EOF follows both complete streams"); finished=true; }
      if (n>0)
      {
        for (int i=0;i<n;++i) check(received+unsigned(i)<Total && input[i]==static_cast<unsigned char>((received+i)*29+3),"duplex stream bytes");
        received+=n;
      }
    }
  }
  close(fd); printf("tcpduplex: 512 KiB duplex ALL PASSED\n"); exit(0);
}
