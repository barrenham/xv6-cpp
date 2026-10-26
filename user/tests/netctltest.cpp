#include "kernel/support/types.h"
#include "kernel/fs/poll.h"
#include "user/runtime/user.h"
namespace
{
void check(bool good, const char *message)
{ if (!good) { fprintf(2,"netctltest: FAIL: %s\n",message); exit(1); } }
netctl_request request(unsigned id=2)
{ netctl_request r{}; r.version=NETCTL_VERSION; r.size=sizeof(r); r.ifindex=id; return r; }
int address(unsigned operation,unsigned ip)
{ auto r=request(); r.address=ip; r.prefix=24; return netctl(operation,&r); }
int state(bool up)
{ auto r=request(); r.flags=up ? NET_LINK_UP : 0; return netctl(NETCTL_SET_LINK,&r); }
sockaddr_in endpoint(unsigned ip,unsigned port)
{ sockaddr_in r{}; r.sin_family=AF_INET; r.sin_addr.s_addr=htonl(ip); r.sin_port=htons(port); return r; }
int send(int fd,sockaddr_in peer)
{ return sendto(fd,"control",7,0,reinterpret_cast<sockaddr *>(&peer),sizeof(peer)); }
void exchange(int fd,sockaddr_in peer)
{
  check(send(fd,peer)==7,"UDP send");
  pollfd pending{fd,POLLIN,0}; check(poll(&pending,1,3000)==1,"UDP response readiness");
  char data[16]; check(read(fd,data,sizeof(data))==7 && !memcmp(data,"control",7),"UDP response data");
}
int bound(unsigned ip,unsigned port=0)
{
  int fd=socket(AF_INET,SOCK_DGRAM|SOCK_NONBLOCK,0); check(fd>=0,"allocate UDP socket");
  auto local=endpoint(ip,port); check(bind(fd,reinterpret_cast<sockaddr *>(&local),sizeof(local))==0,"bind local address"); return fd;
}
}
extern "C" int main(int argc,char **argv)
{
  check(argc==2,"host echo port required");
  auto r=request(); r.version=9; check(netctl(NETCTL_LINK,&r)==-22,"reject ABI version");
  r=request(); r.size=1; check(netctl(NETCTL_LINK,&r)==-22,"reject ABI size");
  check(netctl(NETCTL_LINK,reinterpret_cast<netctl_request *>(~0UL))==-14,"bad user pointer");
  r=request(); check(netctl(NETCTL_LINK,&r)==1 && !strcmp(r.name,"eth0") && r.address_count==1,"interface snapshot");
  auto peer=endpoint(0x0a000202,atoi(argv[1]));
  int fd=bound(0x0a00020f); exchange(fd,peer);
  r=request(); check(netctl(NETCTL_NEIGHBOR,&r)==1 && r.address==0x0a000202,"ARP neighbor visible");
  r=request(); check(netctl(NETCTL_FLUSH_NEIGHBORS,&r)==0,"flush neighbors");
  r=request(); check(netctl(NETCTL_NEIGHBOR,&r)==0,"neighbor cache cleared"); exchange(fd,peer);
  check(state(false)==0 && send(fd,peer)==-101,"down interface blocks existing socket send");
  check(state(true)==0,"enable interface"); exchange(fd,peer);
  auto route=request(); bool found=false;
  for (unsigned i=0;;++i)
  {
    r=request(); r.index=i; if (!netctl(NETCTL_ROUTE,&r)) break;
    if (!r.prefix && r.ifindex==2) { route=r; found=true; }
  }
  check(found && netctl(NETCTL_DEL_ROUTE,&route)==0,"delete default route");
  check(send(fd,endpoint(0xc0000201,9999))==-101,"missing default route is observable");
  check(netctl(NETCTL_ADD_ROUTE,&route)==0,"restore default route");
  check(address(NETCTL_ADD_ADDRESS,0x0a000210)==0,"add alias and local routes");
  int alias=bound(0x0a000210,2099); auto local=endpoint(0x0a000210,2099); exchange(alias,local);
  check(address(NETCTL_DEL_ADDRESS,0x0a000210)==0 && send(alias,local)==-101,"deleted bound address becomes unreachable");
  check(address(NETCTL_ADD_ADDRESS,0x0a000210)==0,"restore alias"); exchange(alias,local); close(alias);
  check(address(NETCTL_DEL_ADDRESS,0x0a000210)==0,"remove alias");
  int pid=fork(); check(pid>=0,"fork configuration stress");
  if (!pid)
  {
    int loop=bound(0x7f000001,2098);
    for (unsigned i=0;i<32;++i) exchange(loop,endpoint(0x7f000001,2098));
    close(loop); exit(0);
  }
  for (unsigned i=0;i<24;++i)
  {
    check(address(NETCTL_ADD_ADDRESS,0x0a000210)==0,"concurrent alias add");
    check(state(false)==0 && state(true)==0,"concurrent link state");
    check(address(NETCTL_DEL_ADDRESS,0x0a000210)==0,"concurrent alias delete");
    if (!(i%2)) pause(1);
  }
  int status; check(waitpid(pid,&status,0)==pid && !status,"concurrent loopback traffic survives changes");
  exchange(fd,peer); close(fd);
  r=request(); check(netctl(NETCTL_LINK,&r)==1 && r.tx_packets>=4 && r.rx_packets>=4 && r.address_count==1 && (r.flags&NET_LINK_UP),"statistics and restored configuration");
  printf("netctltest: ALL PASSED\n"); exit(0);
}
