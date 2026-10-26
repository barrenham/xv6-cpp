#include "kernel/support/types.h"
#include "user/runtime/user.h"
namespace
{
void check(bool good, const char *message)
{ if (!good) { fprintf(2, "dnstest: FAIL: %s\n", message); exit(1); } }
}
extern "C" int main(int argc, char **argv)
{
  check(argc == 2, "DNS test server port required");
  dns_options options{0x0a000202, static_cast<unsigned short>(atoi(argv[1])), 700, 3};
  dns_result result{};
  check(!dns_lookup("192.0.2.7", &result) && result.count == 1 && result.addresses[0] == 0xc0000207 && !result.server, "IPv4 literal without DNS");
  check(dns_lookup("bad..test", &result, &options) == DNS_INVALID && !result.count, "invalid name");
  check(!dns_lookup("MiXeD.TeSt.", &result, &options) && result.count == 2 && result.addresses[0] == 0xc0000201 && result.addresses[1] == 0xc0000202 && result.ttl == 90, "multiple addresses and case folding");
  check(!dns_lookup("alias.test", &result, &options) && result.addresses[0] == 0xc0000209 && !strcmp(result.canonical, "target.test") && result.ttl == 20, "compressed CNAME and unordered answer");
  check(!dns_lookup("follow.test", &result, &options) && result.addresses[0] == 0xc0000209 && !strcmp(result.canonical, "target.test") && result.ttl == 10, "CNAME follow-up query and minimum TTL");
  check(!dns_lookup("tcp.test", &result, &options) && result.addresses[0] == 0xc000020a, "truncated UDP falls back to framed TCP");
  check(!dns_lookup("retry.test", &result, &options) && result.count == 1, "UDP timeout retry");
  check(!dns_lookup("forged.test", &result, &options) && result.addresses[0] == 0xc0000209, "wrong transaction/question/source and invalid compression ignored");
  check(dns_lookup("missing.test", &result, &options) == DNS_NOT_FOUND && !result.count, "NXDOMAIN");
  check(dns_lookup("empty.test", &result, &options) == DNS_NO_ADDRESS && !result.count, "unrelated answers ignored");
  check(dns_lookup("loop.test", &result, &options) == DNS_LIMIT && !result.count, "CNAME loop bounded");
  check(dns_lookup("fail.test", &result, &options) == DNS_SERVER_ERROR, "SERVFAIL bounded retries");
  options.timeout_ms = 200; options.attempts = 2;
  unsigned start = unsigned(uptime());
  check(dns_lookup("silent.test", &result, &options) == DNS_TIMEOUT && unsigned(uptime()) - start < 20, "bounded timeout");
  options.attempts = 5; options.timeout_ms = 1500; options.total_timeout_ms = 300;
  start = unsigned(uptime());
  check(dns_lookup("silent.test", &result, &options) == DNS_TIMEOUT && unsigned(uptime()) - start < 10, "DNS total deadline across retries");
  options.total_timeout_ms = 0;
  options.attempts = 6;
  check(dns_lookup("mixed.test", &result, &options) == DNS_INVALID, "invalid retry bound");
  check(!dns_lookup("localhost", &result) && result.count && result.server == 0x0a000203, "DHCP DNS selection and real QEMU proxy");
  printf("dnstest: ALL PASSED\n"); exit(0);
}
