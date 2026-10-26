#include "user/runtime/http/codec.h"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <string>
using namespace xv::http;
int main()
{
  Url base{}, out{};
  assert(!parse_url("HTTP://Example.COM:8080/a/b/../c?x=1#frag", base));
  assert(!strcmp(base.host, "example.com") && base.port == 8080 && !strcmp(base.target, "/a/c?x=1"));
  assert(!parse_url("http://example.com?x=1", out) && !strcmp(out.target, "/?x=1"));
  assert(!parse_url("http://example.com", out) && !strcmp(out.target, "/"));
  for (auto bad : {"http:///x", "http://a@b/", "http://x:0/", "http://x:65536/", "http://x:1x/", "http://x/a\r\nb", "http://x/a b", "http://x/%", "http://x/%0", "http://x/%zz", "http://x/\\evil", "http://[::1]/", "http://a..b/"})
    assert(parse_url(bad, out) == InvalidUrl);
  assert(parse_url("https://example.com/", out) == Unsupported);
  assert(!parse_url("http://a/b/c/d;p?q", base));
  struct Reference { const char *input, *expected; } references[]{
    {"g", "/b/c/g"}, {"./g", "/b/c/g"}, {"g/", "/b/c/g/"}, {"/g", "/g"},
    {"?y", "/b/c/d;p?y"}, {"g?y", "/b/c/g?y"}, {"#s", "/b/c/d;p?q"},
    {"", "/b/c/d;p?q"}, {".", "/b/c/"}, {"..", "/b/"}, {"../g", "/b/g"},
    {"../..", "/"}, {"../../g", "/g"}, {"../../../g", "/g"}, {"/./g", "/g"},
    {"/../g", "/g"}, {"g/./h", "/b/c/g/h"}, {"g/../h", "/b/c/h"},
    {"g?y/../x", "/b/c/g?y/../x"}, {"%2e%2e/x", "/b/c/%2e%2e/x"}
  };
  for (auto test : references)
  { assert(!resolve_location(base, test.input, out)); if (strcmp(out.target, test.expected)) { printf("bad reference %s -> %s expected %s\n", test.input, out.target, test.expected); return 1; } }
  assert(!resolve_location(base, "//other:90/x", out) && !strcmp(out.host, "other") && out.port == 90);
  assert(resolve_location(base, "https://other/", out) == Unsupported);
  HeaderParser parser;
  assert(!parser.status_line("HTTP/1.1 200 OK") && parser.value.status == 200);
  assert(!parser.field("Content-Length: 123 \t") && parser.value.length == 123);
  assert(!parser.field("content-length: 123") && !parser.finish());
  assert(parser.field("Content-Length: 124") == Protocol);
  assert(parser.field("Content-Length: -1") == Protocol);
  assert(parser.field("Content-Length: 18446744073709551616") == Protocol);
  assert(parser.field("Content-Length: 123, 123") == Protocol);
  assert(parser.field(" Bad: folded") == Protocol);
  assert(parser.field("Bad Name: value") == Protocol);
  assert(parser.field("Bad : value") == Protocol);
  assert(parser.field("Bad: val\001ue") == Protocol);
  assert(parser.field("Content-Length: 1", true) == Protocol);
  assert(!parser.field("X-Trailer: yes", true));
  assert(!parser.status_line("HTTP/1.0 200 OK"));
  assert(!parser.field("Transfer-Encoding: Chunked") && parser.value.chunked);
  assert(!parser.field("Content-Length: 0") && parser.finish() == Protocol);
  assert(!parser.status_line("HTTP/1.1 204 No Content"));
  assert(parser.field("Transfer-Encoding: gzip, chunked") != 0);
  assert(parser.field("Content-Encoding: gzip") == Unsupported);
  for (auto bad : {"", "HTTP/2.0 200 OK", "HTTP/1.1 20 OK", "HTTP/1.1 abc OK", "HTTP/1.1 099 Bad", "HTTP/1.1 200X"}) assert(parser.status_line(bad) == Protocol);
  unsigned long long size;
  assert(!chunk_size("1a;name=value", size) && size == 26);
  assert(!chunk_size("0", size) && !size);
  for (auto bad : {"", "-1", "1z", " 1", "10000000000000000", "1;x=\001"}) assert(chunk_size(bad, size) == Protocol);
  unsigned seed = 31;
  for (unsigned i = 0; i < 10000; ++i)
  {
    seed = seed*1664525U+1013904223U; std::string text(seed%2100, 'x');
    for (auto &c : text) { seed = seed*1664525U+1013904223U; c = char(1+(seed>>24)%127); }
    (void)parse_url(text.c_str(), out);
    (void)resolve_location(base, text.c_str(), out);
    (void)parser.status_line(text.c_str());
    (void)parser.field(text.c_str());
    (void)chunk_size(text.c_str(), size);
  }
  puts("HTTP codec: URLs, relative references, response headers, chunks and mutation corpus passed");
}
