#include "kernel/support/types.h"
#include "kernel/fs/io_status.h"
#include "kernel/fs/poll.h"
#include "kernel/net/filter/control.h"
#include "user/runtime/user.h"
namespace
{
void check(bool okay,const char *why)
{ if (!okay) { fprintf(2,"firewalltest: FAIL: %s\n",why); exit(1); } }
firewall_request request(const char *chain="INPUT")
{ firewall_request r{}; r.version=FW_VERSION; r.size=sizeof(r); strcpy(r.chain,chain); return r; }
int call(unsigned op,firewall_request &r) { return netctl(op,reinterpret_cast<netctl_request *>(&r)); }
void clear(const char *chain)
{ auto r=request(chain); check(!call(FW_FLUSH,r),"flush"); r.policy=FW_ACCEPT; check(!call(FW_POLICY,r),"restore policy"); }
int send(int fd,const sockaddr_in &peer)
{
  int result;
  do { result=sendto(fd,"filter",6,0,reinterpret_cast<const sockaddr *>(&peer),sizeof(peer)); } while (result==-EAGAIN);
  return result;
}
void reply(int fd)
{
  pollfd p{fd,POLLIN,0}; check(poll(&p,1,3000)==1,"allowed echo ready");
  char data[16]; check(read(fd,data,sizeof(data))==6 && !memcmp(data,"filter",6),"allowed echo content");
}
}
extern "C" int main(int argc,char **argv)
{
  check(argc==2,"echo port required"); unsigned port=atoi(argv[1]);
  clear("INPUT"); clear("OUTPUT");
  auto r=request(); r.version=9; check(call(FW_CHAIN,r)==-22,"ABI version");
  check(netctl(FW_CHAIN,reinterpret_cast<netctl_request *>(~0UL))==-14,"bad user pointer");
  int pid=fork(); check(pid>=0,"permission fork");
  if (!pid)
  {
    check(!setuid(12345),"drop credentials"); auto q=request(); check(call(FW_CHAIN,q)==1,"unprivileged query");
    const unsigned ops[]={FW_APPEND,FW_INSERT,FW_DELETE,FW_FLUSH,FW_POLICY,FW_NEW_CHAIN,FW_DELETE_CHAIN,FW_ZERO};
    for (auto op:ops) { q=request(); check(call(op,q)==-1,"unprivileged mutation denied"); }
    auto *invalid = reinterpret_cast<netctl_request *>(~0UL);
    check(netctl(FW_APPEND, invalid) == -1, "firewall permission before copy");
    check(netctl(NETCTL_SET_LINK, invalid) == -14, "network copy before permission");
    netctl_request network{};
    network.version = NETCTL_VERSION; network.size = sizeof(network); network.ifindex = 2;
    check(netctl(NETCTL_SET_LINK, &network) == -1, "network mutation permission");
    check(netctl(NETCTL_LINK, &network) == 1, "unprivileged network query");
    exit(0);
  }
  int status; check(waitpid(pid,&status,0)==pid && !status,"permission checks");
  int fd=socket(AF_INET,SOCK_DGRAM|SOCK_NONBLOCK,0); check(fd>=0,"UDP socket");
  sockaddr_in peer{}; peer.sin_family=AF_INET; peer.sin_port=htons(port); peer.sin_addr.s_addr=htonl(0x0a000202);
  check(send(fd,peer)==6,"baseline send"); reply(fd);
  r=request("OUTPUT"); r.rule.protocol=17; r.rule.dport_low=r.rule.dport_high=port; r.rule.action=FW_DROP;
  check(!call(FW_APPEND,r) && send(fd,peer)==-13,"OUTPUT drop errno");
  r=request("OUTPUT"); check(call(FW_RULE,r)==1 && r.packets==1 && r.bytes==34,"OUTPUT counters");
  clear("OUTPUT");
  r=request(); r.rule.protocol=17; r.rule.sport_low=r.rule.sport_high=port; r.rule.action=FW_DROP;
  check(!call(FW_APPEND,r) && send(fd,peer)==6,"send while INPUT blocked");
  pollfd p{fd,POLLIN,0}; check(poll(&p,1,500)==0,"INPUT silently drops reply");
  r=request(); check(call(FW_RULE,r)==1 && r.packets>=1 && r.bytes>=34,"INPUT counters");
  clear("INPUT"); check(send(fd,peer)==6,"send after flush"); reply(fd);
  // Rule edits race with unrelated loopback traffic on another hart.
  pid=fork(); check(pid>=0,"traffic fork");
  if (!pid)
  {
    int loop=socket(AF_INET,SOCK_DGRAM|SOCK_NONBLOCK,0); check(loop>=0,"loopback socket");
    sockaddr_in local{}; local.sin_family=AF_INET; local.sin_port=htons(2198); local.sin_addr.s_addr=htonl(0x7f000001);
    check(!bind(loop,reinterpret_cast<sockaddr *>(&local),sizeof(local)),"loopback bind");
    for (unsigned i=0;i<32;++i) { check(send(loop,local)==6,"concurrent loopback send"); reply(loop); }
    close(loop); exit(0);
  }
  for (unsigned i=0;i<100;++i)
  {
    r=request("OUTPUT"); r.rule.protocol=17; r.rule.destination=0xc0000201; r.rule.destination_prefix=32; r.rule.action=FW_DROP;
    check(!call(FW_APPEND,r),"concurrent append"); r=request("OUTPUT"); check(!call(FW_FLUSH,r),"concurrent flush");
  }
  check(waitpid(pid,&status,0)==pid && !status,"concurrent traffic");
  clear("INPUT"); clear("OUTPUT"); close(fd);
  printf("firewalltest: ALL PASSED\n"); exit(0);
}
