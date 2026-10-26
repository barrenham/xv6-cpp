#include "user/runtime/dns/codec.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
using Bytes = std::vector<unsigned char>;
namespace
{
void word(Bytes &b, unsigned n) { b.push_back(n >> 8); b.push_back(n); }
Bytes query()
{
  unsigned char b[272]; int n = xv::dns::make_query("example.test", 0x1234, b);
  assert(n > 0); return Bytes(b, b+n);
}
void address(Bytes &b, unsigned char last)
{
  word(b, 0xc00c); word(b, 1); word(b, 1); word(b, 0); word(b, 60); word(b, 4);
  b.insert(b.end(), {192, 0, 2, last});
}
int parse(const Bytes &b)
{ dns_result result{}; return xv::dns::parse_reply(b.data(), b.size(), 0x1234, "example.test", result); }
}
int main()
{
  char normalized[DNS_NAME_SIZE]; unsigned ip;
  assert(!xv::dns::normalize("ExAmPlE.TeSt.", normalized) && !strcmp(normalized, "example.test"));
  for (auto bad : {"", ".", ".test", "test..", "test name", "test\x01"}) assert(xv::dns::normalize(bad, normalized) == DNS_INVALID);
  char long_label[65]; memset(long_label, 'a', 64); long_label[64] = 0;
  assert(xv::dns::normalize(long_label, normalized) == DNS_INVALID);
  char maximum[255]; unsigned at = 0;
  for (unsigned label = 0; label < 4; ++label)
  { unsigned n = label == 3 ? 61 : 63; memset(maximum+at, 'a', n); at += n; maximum[at++] = '.'; }
  maximum[at] = 0;
  assert(at == 254 && !xv::dns::normalize(maximum, normalized) && strlen(normalized) == 253);
  assert(!dns_parse_ipv4("255.0.2.1", &ip) && ip == 0xff000201);
  for (auto bad : {"1.2.3", "1.2.3.4.5", "1.2.3.256", "1..2.3", "-1.2.3.4", "1.2.3.4x"}) assert(dns_parse_ipv4(bad, &ip) == DNS_INVALID);
  Bytes valid = query(); valid[2] = 0x81; valid[3] = 0x80; valid[7] = 1; address(valid, 1);
  assert(parse(valid) == 0);
  for (unsigned i = 0; i < valid.size(); ++i) assert(parse(Bytes(valid.begin(), valid.begin()+i)) < 0);
  Bytes b = valid; b[0] ^= 1; assert(parse(b) == DNS_MALFORMED);
  b = valid; b[13] = 'z'; assert(parse(b) == DNS_MALFORMED);
  b = valid; b[2] &= 0x7f; assert(parse(b) == DNS_MALFORMED);
  b = valid; b[2] |= 8; assert(parse(b) == DNS_MALFORMED);
  b = valid; b[3] |= 0x40; assert(parse(b) == DNS_MALFORMED);
  b = valid; b.push_back(0); assert(parse(b) == DNS_MALFORMED);
  b = valid; b[b.size()-5] = 5; assert(parse(b) == DNS_MALFORMED);
  b = valid; b[query().size()] = 0xc0; b[query().size()+1] = 0xff; assert(parse(b) == DNS_MALFORMED);
  b = query(); b[2] = 0x83; b[3] = 0x80; b[7] = 100; assert(parse(b) == xv::dns::truncated);
  b = query(); b[2] = 0x81; b[3] = 0x83; assert(parse(b) == DNS_NOT_FOUND);
  b[3] = 0x82; assert(parse(b) == DNS_SERVER_ERROR);
  b[3] = 0x80; assert(parse(b) == DNS_NO_ADDRESS);
  b = query(); b[2] = 0x81; b[3] = 0x80; b[7] = 10;
  for (unsigned i = 0; i < 10; ++i) address(b, i+1);
  dns_result result{};
  assert(!xv::dns::parse_reply(b.data(), b.size(), 0x1234, "example.test", result) && result.count == DNS_MAX_ADDRESSES);
  // Mutation corpus is also run with ASan/UBSan to check all parser boundaries.
  for (unsigned offset = 0; offset < valid.size(); ++offset)
    for (unsigned value = 0; value < 256; ++value)
    { b = valid; b[offset] = value; (void)parse(b); }
  unsigned seed = 0x9812;
  for (unsigned i = 0; i < 10000; ++i)
  {
    seed = seed * 1664525U + 1013904223U; b.resize(seed % 1024);
    for (auto &byte : b) { seed = seed * 1664525U + 1013904223U; byte = seed >> 24; }
    (void)parse(b);
  }
  puts("DNS codec: boundaries, compression, malformed messages and mutation corpus passed");
}
