#include "kernel/support/types.h"
#include "user/runtime/user.h"
namespace
{
netctl_request request()
{ netctl_request r{}; r.version = NETCTL_VERSION; r.size = sizeof(r); return r; }
[[noreturn]] void fail(const char *message, int error)
{ fprintf(2, "netctl: %s (%d)\n", message, error); exit(1); }
int call(unsigned operation, netctl_request &r)
{ int result = netctl(operation, &r); if (result < 0) fail("operation failed", result); return result; }
bool number(const char *s, unsigned &value, unsigned limit)
{
  if (!*s) return false;
  value = 0;
  for (; *s; ++s)
  {
    if (*s < '0' || *s > '9' || value > limit/10 || (value == limit/10 && unsigned(*s-'0') > limit%10)) return false;
    value = value*10+(*s-'0');
  }
  return true;
}
bool ipv4(const char *s, unsigned &value)
{
  value = 0;
  for (unsigned n = 0; n < 4; ++n)
  {
    unsigned part = 0, digits = 0;
    while (*s >= '0' && *s <= '9') { if (++digits > 3) return false; part = part*10+(*s++-'0'); }
    if (!digits || part > 255) return false;
    value = (value<<8)|part;
    if (n < 3) { if (*s++ != '.') return false; }
  }
  return !*s;
}
bool prefix(const char *s, unsigned &address, unsigned &bits)
{
  if (!strcmp(s, "default")) { address = bits = 0; return true; }
  char text[20]; unsigned i = 0;
  while (*s && *s != '/' && i < sizeof(text)-1) text[i++] = *s++;
  text[i] = 0;
  if (*s++ != '/') return false;
  return ipv4(text, address) && number(s, bits, 32);
}
void ip(unsigned value)
{ printf("%u.%u.%u.%u", value>>24, (value>>16)&255, (value>>8)&255, value&255); }
void mac(const unsigned char *value)
{
  const char *hex = "0123456789abcdef";
  for (unsigned i = 0; i < 6; ++i) printf("%s%c%c", i ? ":" : "", hex[value[i]>>4], hex[value[i]&15]);
}
unsigned device(const char *name)
{
  for (unsigned i = 0;; ++i)
  {
    auto r = request(); r.index = i;
    if (!call(NETCTL_LINK, r)) fail("unknown interface", -19);
    if (!strcmp(r.name, name)) return r.ifindex;
  }
}
[[noreturn]] void usage()
{
  fprintf(2, "usage: netctl link|addr|route|neigh|stats [show [DEV]]\n"
             "       netctl link set DEV up|down\n"
             "       netctl addr add|del IP/PREFIX dev DEV\n"
             "       netctl route add|del NET/PREFIX|default dev DEV [via IP] [metric N] [src IP]\n"
             "       netctl neigh flush DEV\n"
             "       netctl bridge create|delete|flush|fdb BR\n"
             "       netctl bridge addport|delport BR DEV\n"
             "       netctl bridge show [BR]\n");
  exit(1);
}
void show(const char *what, unsigned filter)
{
  if (!strcmp(what, "route"))
  {
    for (unsigned n = 0;; ++n)
    {
      auto r = request(); r.index = n;
      if (!call(NETCTL_ROUTE, r)) return;
      if (filter && filter != r.ifindex) continue;
      auto link = request(); link.ifindex = r.ifindex; call(NETCTL_LINK, link);
      ip(r.address); printf("/%u dev %s", r.prefix, link.name);
      if (r.gateway) { printf(" via "); ip(r.gateway); }
      printf(" metric %u", r.metric);
      if (r.source) { printf(" src "); ip(r.source); }
      printf("%s\n", r.flags & NET_ROUTE_AUTOMATIC ? " automatic" : "");
    }
  }
  for (unsigned n = 0;; ++n)
  {
    auto r = request(); r.index = n;
    if (!call(NETCTL_LINK, r)) return;
    if (filter && filter != r.ifindex) continue;
    if (!strcmp(what, "link"))
    {
      printf("%u: %s %s carrier %s mtu %u", r.ifindex, r.name, r.flags & NET_LINK_UP ? "UP" : "DOWN",
             r.flags & NET_LINK_CARRIER ? "up" : "down", r.mtu);
      if (r.flags & NET_LINK_MAC) { printf(" mac "); mac(r.mac); }
      if (r.flags & NET_LINK_BRIDGE) printf(" bridge");
      if (r.flags & NET_LINK_SLAVE) printf(" master %u",r.source);
      printf("%s\n", r.flags & NET_LINK_TUNNEL ? " tunnel" : "");
    }
    else if (!strcmp(what, "stats"))
    {
      printf("%s RX packets %llu bytes %llu dropped %llu\n", r.name, r.rx_packets, r.rx_bytes, r.rx_dropped);
      printf("%s TX packets %llu bytes %llu errors %llu\n", r.name, r.tx_packets, r.tx_bytes, r.tx_errors);
    }
    else
    {
      for (unsigned i = 0;; ++i)
      {
        auto item = request(); item.ifindex = r.ifindex; item.index = i;
        if (!call(!strcmp(what,"addr") ? NETCTL_ADDRESS : NETCTL_NEIGHBOR, item)) break;
        printf("%s ", r.name); ip(item.address);
        if (!strcmp(what,"addr")) printf("/%u\n", item.prefix);
        else { printf(" "); mac(item.mac); printf(" expires %llu ms\n", item.expires_ms); }
      }
    }
  }
}
void bridge_command(int argc, char **argv)
{
  if (argc == 2 || (argc >= 3 && !strcmp(argv[2],"show")))
  {
    if (argc > 4) usage();
    unsigned filter = argc == 4 ? device(argv[3]) : 0;
    for (unsigned i = 0;; ++i)
    {
      auto link = request(); link.index = i;
      if (!call(NETCTL_LINK, link)) return;
      if (!(link.flags & NET_LINK_BRIDGE) || (filter && link.ifindex != filter)) continue;
      printf("%s id %u %s ports:", link.name, link.ifindex, link.flags & NET_LINK_UP ? "UP" : "DOWN");
      for (unsigned n = 0;; ++n)
      {
        auto port = request(); port.ifindex = link.ifindex; port.index = n;
        if (!call(NETCTL_BRIDGE_PORT,port)) break;
        auto name = request(); name.ifindex = port.source; call(NETCTL_LINK,name);
        printf(" %s",name.name);
      }
      printf("\n");
    }
  }
  auto r = request(); unsigned operation;
  if (argc == 4 && !strcmp(argv[2],"create"))
  {
    unsigned length = strlen(argv[3]); if (length >= sizeof(r.name)) usage();
    memmove(r.name,argv[3],length+1); operation = NETCTL_BRIDGE_CREATE;
  }
  else if (argc == 4 && !strcmp(argv[2],"delete"))
  { r.ifindex = device(argv[3]); operation = NETCTL_BRIDGE_DELETE; }
  else if (argc == 5 && (!strcmp(argv[2],"addport") || !strcmp(argv[2],"delport")))
  {
    r.ifindex = device(argv[3]); r.source = device(argv[4]);
    operation = !strcmp(argv[2],"addport") ? NETCTL_BRIDGE_ADD_PORT : NETCTL_BRIDGE_DEL_PORT;
  }
  else if (argc == 4 && !strcmp(argv[2],"flush"))
  { r.ifindex = device(argv[3]); operation = NETCTL_BRIDGE_FLUSH_FDB; }
  else if (argc == 4 && !strcmp(argv[2],"fdb"))
  {
    unsigned id = device(argv[3]);
    for (unsigned i = 0;; ++i)
    {
      auto entry = request(); entry.ifindex = id; entry.index = i;
      if (!call(NETCTL_BRIDGE_FDB,entry)) return;
      auto port = request(); port.ifindex = entry.source; call(NETCTL_LINK,port);
      mac(entry.mac); printf(" dev %s expires %llu ms\n",port.name,entry.expires_ms);
    }
  }
  else { usage(); }
  call(operation,r);
}

}
extern "C" int main(int argc, char **argv)
{
  if (argc < 2) usage();
  if (!strcmp(argv[1],"bridge")) { bridge_command(argc,argv); exit(0); }
  const char *what = argv[1];
  if (strcmp(what,"link") && strcmp(what,"addr") && strcmp(what,"route") && strcmp(what,"neigh") && strcmp(what,"stats")) usage();
  if (argc == 2 || (argc >= 3 && !strcmp(argv[2],"show")))
  {
    if (argc > 4) usage();
    show(what, argc == 4 ? device(argv[3]) : 0); exit(0);
  }
  auto r = request(); unsigned operation;
  if (!strcmp(what,"link") && argc == 5 && !strcmp(argv[2],"set"))
  {
    r.ifindex = device(argv[3]);
    if (!strcmp(argv[4],"up")) r.flags = NET_LINK_UP;
    else if (strcmp(argv[4],"down")) usage();
    operation = NETCTL_SET_LINK;
  }
  else if (!strcmp(what,"neigh") && argc == 4 && !strcmp(argv[2],"flush"))
  { r.ifindex = device(argv[3]); operation = NETCTL_FLUSH_NEIGHBORS; }
  else
  {
    bool add = !strcmp(argv[2],"add");
    if ((!add && strcmp(argv[2],"del")) || argc < 6 || !prefix(argv[3], r.address, r.prefix) || strcmp(argv[4],"dev")) usage();
    r.ifindex = device(argv[5]);
    if (!strcmp(what,"addr"))
    {
      if (argc != 6) usage();
      operation = add ? NETCTL_ADD_ADDRESS : NETCTL_DEL_ADDRESS;
    }
    else if (!strcmp(what,"route"))
    {
      unsigned seen = 0;
      for (int i = 6; i < argc; i += 2)
      {
        if (i+1 == argc) usage();
        unsigned flag;
        if (!strcmp(argv[i],"via")) { flag = 1; if (!ipv4(argv[i+1],r.gateway)) usage(); }
        else if (!strcmp(argv[i],"metric")) { flag = 2; if (!number(argv[i+1],r.metric,~0U)) usage(); }
        else if (!strcmp(argv[i],"src")) { flag = 4; if (!ipv4(argv[i+1],r.source)) usage(); }
        else { usage(); }
        if (seen & flag) usage(); seen |= flag;
      }
      operation = add ? NETCTL_ADD_ROUTE : NETCTL_DEL_ROUTE;
    }
    else { usage(); }
  }
  call(operation,r); exit(0);
}
