#include "kernel/support/types.h"
#include "kernel/fs/poll.h"
#include "user/runtime/user.h"
namespace
{
void check(bool good,const char *message)
{ if (!good) { fprintf(2,"dhcptest: FAIL: %s\n",message); exit(1); } }
netctl_request request(unsigned id=2)
{ netctl_request r{}; r.version=NETCTL_VERSION; r.size=sizeof(r); r.ifindex=id; return r; }
netctl_request wait(unsigned id)
{
  for (unsigned i=0;i<200;++i)
  {
    auto r=request(id); check(netctl(NETCTL_DHCP_STATUS,&r)==1,"query DHCP state");
    if (r.flags==DHCP_BOUND) return r;
    pause(1);
  }
  check(false,"lease timeout"); return {};
}
sockaddr_in endpoint(unsigned ip,unsigned port)
{ sockaddr_in value{}; value.sin_family=AF_INET; value.sin_addr.s_addr=htonl(ip); value.sin_port=htons(port); return value; }
void exchange(const netctl_request &lease,unsigned port)
{
  check(lease.prefix==24&&lease.gateway==0x0a000202&&lease.source==0x0a000203&&lease.metric==0x0a000202,"lease options");
  check(lease.address>=0x0a000264&&lease.address<=0x0a000273&&lease.expires_ms>0,"configured dynamic pool, not static .15");
  int fd=socket(AF_INET,SOCK_DGRAM|SOCK_NONBLOCK,0); check(fd>=0,"UDP socket");
  auto local=endpoint(lease.address,0),peer=endpoint(lease.gateway,port);
  check(bind(fd,reinterpret_cast<sockaddr *>(&local),sizeof(local))==0,"bind leased address");
  check(sendto(fd,"dhcp-real",9,0,reinterpret_cast<sockaddr *>(&peer),sizeof(peer))==9,"send from dynamic address");
  pollfd input{fd,POLLIN,0}; check(poll(&input,1,3000)==1,"dynamic address reply");
  char data[16]; check(read(fd,data,sizeof(data))==9&&!memcmp(data,"dhcp-real",9),"dynamic UDP payload"); close(fd);
}
void bridge(unsigned operation,unsigned source=0)
{ auto r=request(16); r.source=source; check(netctl(operation,&r)==0,"bridge configuration"); }
}
extern "C" int main(int argc,char **argv)
{
  check(argc==2,"echo port required"); unsigned port=atoi(argv[1]);
  auto lease=wait(2); exchange(lease,port);
  auto r=request(); r.address=lease.address; r.prefix=lease.prefix;
  check(netctl(NETCTL_DEL_ADDRESS,&r)==-16,"leased address is protected");
  r=request(); check(netctl(NETCTL_DHCP_RENEW,&r)==0,"request renewal"); exchange(wait(2),port);
  r=request(); r.address=0x0a0002c8; r.prefix=24; check(netctl(NETCTL_ADD_ADDRESS,&r)==0,"manual alias alongside DHCP");
  r=request(); r.address=0xcb007100; r.prefix=24; r.gateway=0x0a000202; r.source=0x0a0002c8; r.metric=7;
  check(netctl(NETCTL_ADD_ROUTE,&r)==0,"manual preferred source route");
  r=request(); check(netctl(NETCTL_DHCP_STOP,&r)==0,"stop DHCP");
  r=request(); check(netctl(NETCTL_LINK,&r)==1&&r.address_count==1,"stop preserves manual alias");
  r=request(); check(netctl(NETCTL_DHCP_START,&r)==-16,"start does not erase static configuration");
  bool manual=false;
  for (unsigned i=0;;++i)
  {
    r=request(); r.index=i; if (!netctl(NETCTL_ROUTE,&r)) break;
    check(!(r.flags&NET_CONFIG_DHCP),"owned routes removed");
    if (r.address==0xcb007100&&r.source==0x0a0002c8) manual=true;
  }
  check(manual,"manual route remains");
  r=request(); r.address=0x0a0002c8; r.prefix=24; check(netctl(NETCTL_DEL_ADDRESS,&r)==0,"remove manual alias");
  r=request(); memmove(r.name,"br0",4); check(netctl(NETCTL_BRIDGE_CREATE,&r)==0,"create DHCP bridge");
  bridge(NETCTL_BRIDGE_ADD_PORT,2);
  r=request(16); r.flags=NET_LINK_UP; check(netctl(NETCTL_SET_LINK,&r)==0,"enable bridge");
  r=request(16); check(netctl(NETCTL_DHCP_START,&r)==0,"start DHCP on bridge");
  exchange(wait(16),port);
  r=request(16); check(netctl(NETCTL_DHCP_STOP,&r)==0,"stop bridge DHCP");
  r=request(16); r.flags=0; check(netctl(NETCTL_SET_LINK,&r)==0,"disable bridge");
  bridge(NETCTL_BRIDGE_DEL_PORT,2); bridge(NETCTL_BRIDGE_DELETE);
  r=request(); check(netctl(NETCTL_DHCP_START,&r)==0,"restart physical DHCP"); exchange(wait(2),port);
  printf("dhcptest: ALL PASSED\n"); exit(0);
}
