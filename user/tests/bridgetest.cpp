#include "kernel/support/types.h"
#include "kernel/fs/poll.h"
#include "user/runtime/user.h"
namespace
{
void check(bool ok,const char *message)
{ if (!ok) { fprintf(2,"bridgetest: FAIL: %s\n",message); exit(1); } }
}
extern "C" int main()
{
  int fd=socket(AF_INET,SOCK_DGRAM|SOCK_NONBLOCK,0);
  sockaddr_in local{}; local.sin_family=AF_INET; local.sin_port=htons(2033);
  check(fd>=0&&bind(fd,reinterpret_cast<sockaddr *>(&local),sizeof(local))==0,"bind bridge listener");
  printf("bridgetest: ready\n");
  pollfd pending{fd,POLLIN,0}; check(poll(&pending,1,5000)==1,"bridge local input");
  char data[64]; sockaddr_in peer{}; socklen_t size=sizeof(peer);
  int n=recvfrom(fd,data,sizeof(data),0,reinterpret_cast<sockaddr *>(&peer),&size);
  check(n==11&&!memcmp(data,"bridge-host",11),"bridge payload");
  check(sendto(fd,data,n,0,reinterpret_cast<sockaddr *>(&peer),size)==n,"bridge local output");
  close(fd); printf("bridgetest: ALL PASSED\n"); exit(0);
}
