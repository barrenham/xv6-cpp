#include "kernel/support/types.h"
#include "kernel/net/filter/control.h"
#include "user/runtime/user.h"
namespace
{
[[noreturn]] void fail(const char *message)
{ fprintf(2,"iptables: %s\n",message); exit(1); }
[[noreturn]] void usage()
{
  fprintf(2,"usage: iptables [-t filter] -L [CHAIN] [-n] [-v] [--line-numbers]\n"
            "       iptables -A CHAIN | -I CHAIN [N] [matches] -j TARGET\n"
            "       iptables -D CHAIN N | -P INPUT|OUTPUT ACCEPT|DROP\n"
            "       iptables -N CHAIN | -X CHAIN | -F [CHAIN] | -Z [CHAIN]\n"
            "matches: -s IP[/BITS] -d IP[/BITS] -p tcp|udp|icmp|all|N\n"
            "         --sport PORT[:PORT] --dport PORT[:PORT] -i DEV -o DEV\n"
            "targets: ACCEPT, DROP, RETURN, or a user chain (max 15 characters)\n"); exit(1);
}
firewall_request request(const char *chain=nullptr)
{
  firewall_request r{}; r.version=FW_VERSION; r.size=sizeof(r);
  if (chain) { if (strlen(chain)>15) fail("chain name too long"); strcpy(r.chain,chain); }
  return r;
}
int call(unsigned op, firewall_request &r)
{
  int result=netctl(op,reinterpret_cast<netctl_request *>(&r));
  if (result<0) { fprintf(2,"iptables: operation failed (%d)\n",result); exit(1); }
  return result;
}
bool number(const char *s,unsigned &n,unsigned max)
{
  n=0; if (!*s) return false;
  for (;*s;++s) { if (*s<'0' || *s>'9' || n>max/10 || (n==max/10 && unsigned(*s-'0')>max%10)) return false; n=n*10+*s-'0'; }
  return true;
}
bool address(const char *s,unsigned &value,unsigned &bits)
{
  value=0; bits=32;
  for (unsigned i=0;i<4;++i)
  {
    unsigned n=0,digits=0;
    while (*s>='0' && *s<='9') { if (++digits>3) return false; n=n*10+*s++-'0'; }
    if (!digits || n>255) return false; value=(value<<8)|n;
    if (i<3) { if (*s!='.') return false; ++s; }
  }
  if (*s=='/') { if (!number(s+1,bits,32)) return false; }
  else if (*s) return false;
  value&=bits ? (~0U<<(32-bits)) : 0; return true;
}
bool ports(const char *s,unsigned &low,unsigned &high)
{
  char text[6]; unsigned i=0;
  while (*s && *s!=':') { if (i==5) return false; text[i++]=*s++; } text[i]=0;
  if (!number(text,low,65535)) return false;
  if (*s==':') { if (!number(s+1,high,65535)) return false; } else high=low;
  // Zero high is the wire representation for an unrestricted port.
  return high && low<=high;
}
unsigned device(const char *name)
{
  for (unsigned i=0;;++i)
  {
    netctl_request r{}; r.version=NETCTL_VERSION; r.size=sizeof(r); r.index=i;
    if (netctl(NETCTL_LINK,&r)<=0) fail("unknown interface");
    if (!strcmp(name,r.name)) return r.ifindex;
  }
}
void ip(unsigned n,unsigned bits)
{ printf("%u.%u.%u.%u/%u",n>>24,(n>>16)&255,(n>>8)&255,n&255,bits); }
const char *action(unsigned n)
{ return n==FW_ACCEPT ? "ACCEPT" : n==FW_DROP ? "DROP" : n==FW_RETURN ? "RETURN" : "JUMP"; }
void interface(unsigned id)
{
  if (!id) { printf("*"); return; }
  netctl_request r{}; r.version=NETCTL_VERSION; r.size=sizeof(r); r.ifindex=id;
  if (netctl(NETCTL_LINK,&r)>0) printf("%s",r.name); else printf("#%u",id);
}
void list(const firewall_request &chain)
{
  printf("Chain %s (%s %llu packets, %llu bytes)\n",chain.chain,chain.policy==FW_RETURN ? "return" : action(chain.policy),chain.packets,chain.bytes);
  printf("num packets bytes target proto in out source destination ports\n");
  for (unsigned i=0;;++i)
  {
    auto r=request(chain.chain); r.index=i; if (!call(FW_RULE,r)) break;
    auto &v=r.rule;
    printf("%u %llu %llu %s %u ",i+1,r.packets,r.bytes,v.action==FW_JUMP ? v.target : action(v.action),v.protocol);
    interface(v.input); printf(" "); interface(v.output); printf(" "); ip(v.source,v.source_prefix); printf(" "); ip(v.destination,v.destination_prefix);
    if (v.sport_high) printf(" sport %u:%u",v.sport_low,v.sport_high);
    if (v.dport_high) printf(" dport %u:%u",v.dport_low,v.dport_high);
    printf("\n");
  }
}
}
extern "C" int main(int argc,char **argv)
{
  int pos=1;
  if (pos<argc && !strcmp(argv[pos],"-t"))
  { if (pos+1>=argc || strcmp(argv[pos+1],"filter")) fail("only the filter table is supported"); pos+=2; }
  if (pos==argc) usage();
  const char *command=argv[pos++];
  unsigned op;
  if (!strcmp(command,"-L")) op=FW_CHAIN;
  else if (!strcmp(command,"-A")) op=FW_APPEND;
  else if (!strcmp(command,"-I")) op=FW_INSERT;
  else if (!strcmp(command,"-D")) op=FW_DELETE;
  else if (!strcmp(command,"-F")) op=FW_FLUSH;
  else if (!strcmp(command,"-Z")) op=FW_ZERO;
  else if (!strcmp(command,"-N")) op=FW_NEW_CHAIN;
  else if (!strcmp(command,"-X")) op=FW_DELETE_CHAIN;
  else if (!strcmp(command,"-P")) op=FW_POLICY;
  else usage();
  const char *chain=nullptr;
  if (pos<argc && argv[pos][0]!='-') chain=argv[pos++];
  if (!chain && op!=FW_CHAIN && op!=FW_FLUSH && op!=FW_ZERO) usage();
  if (op==FW_CHAIN || op==FW_FLUSH || op==FW_ZERO)
  {
    while (pos<argc && op==FW_CHAIN)
    { if (strcmp(argv[pos],"-n") && strcmp(argv[pos],"-v") && strcmp(argv[pos],"--line-numbers")) usage(); ++pos; }
    if (pos!=argc) usage();
    bool found=false;
    for (unsigned i=0;;++i)
    {
      auto c=request(); c.index=i; if (!call(FW_CHAIN,c)) break;
      if (chain && strcmp(chain,c.chain)) continue;
      found=true; if (op==FW_CHAIN) list(c); else { auto r=request(c.chain); call(op,r); }
    }
    if (chain && !found) fail("unknown chain"); exit(0);
  }
  auto r=request(chain);
  if (op==FW_POLICY)
  {
    if (pos+1!=argc) usage();
    if (!strcmp(argv[pos],"ACCEPT")) r.policy=FW_ACCEPT;
    else if (!strcmp(argv[pos],"DROP")) r.policy=FW_DROP; else usage();
  }
  else if (op==FW_DELETE)
  { unsigned n; if (pos+1!=argc || !number(argv[pos],n,FW_MAX_RULES) || !n) usage(); r.index=n-1; }
  else if (op==FW_NEW_CHAIN || op==FW_DELETE_CHAIN) { if (pos!=argc) usage(); }
  else
  {
    if (op==FW_INSERT && pos<argc && argv[pos][0]!='-')
    { unsigned n; if (!number(argv[pos++],n,FW_MAX_RULES) || !n) usage(); r.index=n-1; }
    unsigned seen=0;
    while (pos<argc)
    {
      const char *key=argv[pos++]; if (pos==argc) usage(); const char *value=argv[pos++]; unsigned bit=0;
      if (!strcmp(key,"-s")) { bit=1; if (!address(value,r.rule.source,r.rule.source_prefix)) usage(); }
      else if (!strcmp(key,"-d")) { bit=2; if (!address(value,r.rule.destination,r.rule.destination_prefix)) usage(); }
      else if (!strcmp(key,"-p"))
      {
        bit=4;
        if (!strcmp(value,"tcp")) r.rule.protocol=6;
        else if (!strcmp(value,"udp")) r.rule.protocol=17;
        else if (!strcmp(value,"icmp")) r.rule.protocol=1;
        else if (!strcmp(value,"all")) r.rule.protocol=0;
        else if (!number(value,r.rule.protocol,255)) usage();
      }
      else if (!strcmp(key,"--sport")) { bit=8; if (!ports(value,r.rule.sport_low,r.rule.sport_high)) usage(); }
      else if (!strcmp(key,"--dport")) { bit=16; if (!ports(value,r.rule.dport_low,r.rule.dport_high)) usage(); }
      else if (!strcmp(key,"-i")) { bit=32; r.rule.input=device(value); }
      else if (!strcmp(key,"-o")) { bit=64; r.rule.output=device(value); }
      else if (!strcmp(key,"-j"))
      {
        bit=128;
        if (!strcmp(value,"ACCEPT")) r.rule.action=FW_ACCEPT;
        else if (!strcmp(value,"DROP")) r.rule.action=FW_DROP;
        else if (!strcmp(value,"RETURN")) r.rule.action=FW_RETURN;
        else { if (strlen(value)>15) usage(); r.rule.action=FW_JUMP; strcpy(r.rule.target,value); }
      }
      else usage();
      if (seen&bit) usage(); seen|=bit;
    }
    if (!(seen&128) || ((seen&(8|16)) && r.rule.protocol!=6 && r.rule.protocol!=17) ||
        (!strcmp(chain,"INPUT") && r.rule.output) || (!strcmp(chain,"OUTPUT") && r.rule.input)) usage();
  }
  call(op,r); exit(0);
}
