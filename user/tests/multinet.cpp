#include "kernel/support/types.h"
#include "kernel/fs/poll.h"
#include "user/runtime/user.h"
namespace
{
char payload[1472], output[1472];
void check(bool good,const char *message)
{ if (!good) { fprintf(2,"multinet: FAIL: %s\n",message); exit(1); } }
}
extern "C" int main(int argc,char **argv)
{
  check(argc==2,"host UDP port required");
  int port=atoi(argv[1]), descriptors[2];
  for (int i=0;i<1472;++i) payload[i]=i*17+3;
  for (int n=0;n<2;++n)
  {
    descriptors[n]=socket(AF_INET,SOCK_DGRAM|SOCK_NONBLOCK,0);
    sockaddr_in local{}; local.sin_family=AF_INET; local.sin_port=htons(2030);
    local.sin_addr.s_addr=htonl(0x0a00020f+(n<<8));
    check(descriptors[n]>=0 && bind(descriptors[n],reinterpret_cast<sockaddr *>(&local),sizeof(local))==0,"bind each physical interface address");
  }
  for (int round=0;round<40;++round)
    for (int n=0;n<2;++n)
    {
      sockaddr_in remote{}; remote.sin_family=AF_INET; remote.sin_port=htons(port);
      remote.sin_addr.s_addr=htonl(0x0a000202+(n<<8));
      int length=round==0 ? 1472 : 17+round;
      check(sendto(descriptors[n],payload,length,0,reinterpret_cast<sockaddr *>(&remote),sizeof(remote))==length,"send through selected interface");
      pollfd wait{descriptors[n],POLLIN,0};
      check(poll(&wait,1,3000)==1,"each NIC interrupt wakes socket");
      sockaddr_in source{}; socklen_t size=sizeof(source);
      check(recvfrom(descriptors[n],output,sizeof(output),0,reinterpret_cast<sockaddr *>(&source),&size)==length &&
            !memcmp(payload,output,length) && source.sin_addr.s_addr==remote.sin_addr.s_addr && source.sin_port==remote.sin_port,
            "interface-specific UDP reply");
    }
  close(descriptors[0]);close(descriptors[1]);
  printf("multinet: two NICs and shared INTx passed\n"); exit(0);
}
