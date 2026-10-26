#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/dns/dns.h"
namespace
{
void address(unsigned ip)
{ printf("%u.%u.%u.%u", ip >> 24, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255); }
}
extern "C" int main(int argc, char **argv)
{
  dns_options options{};
  if (argc < 2 || argc > 3 || (argc == 3 && (dns_parse_ipv4(argv[2], &options.server) || !options.server)))
  { fprintf(2, "usage: nslookup HOST [DNS-SERVER-IP]\n"); exit(1); }
  dns_result result{};
  int status = dns_lookup(argv[1], &result, &options);
  if (status) { fprintf(2, "nslookup: %s\n", dns_strerror(status)); exit(1); }
  if (result.server) { printf("Server: "); address(result.server); printf("\n"); }
  printf("Name: %s\n", result.canonical);
  for (unsigned i = 0; i < result.count; ++i)
  { printf("Address: "); address(result.addresses[i]); printf("\n"); }
  if (result.server) printf("TTL: %u seconds\n", result.ttl);
  exit(0);
}
