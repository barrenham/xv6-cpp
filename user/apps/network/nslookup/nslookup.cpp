#include "user/runtime/ui.h"
#include "application.h"
#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include "user/runtime/dns/dns.h"

namespace
{
void
address(const char *label, unsigned ip) {
  ui::info("%s%u.%u.%u.%u\n", label, ip >> 24, (ip >> 16) & 255, (ip >> 8) & 255,
           ip & 255);
}
} // namespace

extern "C" int
app_main(int argc, char **argv) {
  dns_options options{};
  if (argc < 2 || argc > 3 ||
      (argc == 3 && (dns_parse_ipv4(argv[2], &options.server) || !options.server)))
  {
    ui::usage("usage: nslookup HOST [DNS-SERVER-IP]\n");
    exit(1);
  }
  dns_result result{};
  int status = dns_lookup(argv[1], &result, &options);
  if (status)
  {
    ui::error("nslookup: %s\n", dns_strerror(status));
    exit(1);
  }
  if (result.server)
  {
    address("Server: ", result.server);
  }
  ui::info("Name: %s\n", result.canonical);
  for (unsigned i = 0; i < result.count; ++i)
  {
    address("Address: ", result.addresses[i]);
  }
  if (result.server)
  {
    ui::muted("TTL: %u seconds\n", result.ttl);
  }
  exit(0);
}
