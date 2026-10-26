#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"
#include "user/runtime/http/http.h"
using namespace xv::http;
namespace
{
void check(bool good, const char *message)
{ if (!good) { fprintf(2, "httptest: FAIL: %s\n", message); exit(1); } }
unsigned hash_byte(unsigned hash, unsigned byte) { return (hash ^ byte)*16777619U; }
class Capture final : public Sink
{
public:
  unsigned hash = 2166136261U, count = 0, header_bytes = 0;
  bool reject = false;
  bool headers(const char *, unsigned n) override { header_bytes += n; return !reject; }
  bool body(const char *data, unsigned n) override
  { if (reject) return false; for (unsigned i = 0; i < n; ++i) hash = hash_byte(hash, static_cast<unsigned char>(data[i])); count += n; return true; }
};
void make_url(char *out, unsigned port, const char *path, const char *host = "10.0.2.2")
{
  strcpy(out, "http://"); strcpy(out+strlen(out), host); unsigned at = strlen(out); out[at++] = ':';
  char digits[6]; unsigned n = 0; do { digits[n++] = char('0'+port%10); port /= 10; } while (port);
  while (n) out[at++] = digits[--n];
  out[at] = 0; strcpy(out+at, path);
}
unsigned expected(unsigned count)
{ unsigned hash = 2166136261U; for (unsigned i = 0; i < count; ++i) hash = hash_byte(hash, (i*37+11)&255); return hash; }
void request(unsigned port, const char *path, int wanted, unsigned bytes = 0, Options options = {})
{
  char url[128]; make_url(url, port, path); Capture sink; Response response; Client client;
  int status = client.get(url, options, sink, response);
  if (status != wanted) { fprintf(2, "httptest: %s wanted %d got %d\n", path, wanted, status); check(false, "HTTP result"); }
  if (!status) check(sink.count == bytes && sink.hash == expected(bytes) && response.body_bytes == bytes, "streamed binary body");
}
void local_dns()
{
  int channel[2]; check(pipe(channel) == 0, "local server pipe");
  int child = fork(); check(child >= 0, "local server fork");
  if (!child)
  {
    close(channel[0]); int fd = socket(AF_INET, SOCK_STREAM, 0); check(fd >= 0, "local listener");
    sockaddr_in local{}; local.sin_family = AF_INET; local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    check(!bind(fd, reinterpret_cast<sockaddr *>(&local), sizeof(local)) && !listen(fd, 1), "listen localhost");
    socklen_t length = sizeof(local); check(!getsockname(fd, reinterpret_cast<sockaddr *>(&local), &length), "local port");
    unsigned port = ntohs(local.sin_port); check(write(channel[1], &port, sizeof(port)) == sizeof(port), "publish port"); close(channel[1]);
    int peer = accept(fd, nullptr, nullptr); check(peer >= 0, "accept hostname request");
    char buffer[512]; check(read(peer, buffer, sizeof(buffer)) > 0, "hostname request");
    const char reply[] = "HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\n\x0b";
    check(write(peer, reply, sizeof(reply)-1) == sizeof(reply)-1, "hostname response"); close(peer); close(fd); exit(0);
  }
  close(channel[1]); unsigned port; check(read(channel[0], &port, sizeof(port)) == sizeof(port), "receive port"); close(channel[0]);
  char url[128]; make_url(url, port, "/", "localhost"); Capture sink; Response response; Client client;
  check(!client.get(url, {}, sink, response) && sink.count == 1 && sink.hash == expected(1), "DHCP DNS to localhost HTTP");
  int status; check(waitpid(child, &status, 0) == child && !status, "local server exit");
}
}
extern "C" int main(int argc, char **argv)
{
  if (argc == 3 && !strcmp(argv[1], "--file"))
  {
    int fd = open(argv[2], O_RDONLY); check(fd >= 0, "open downloaded file");
    char data[512]; Capture sink;
    for (;;) { int n = read(fd, data, sizeof(data)); check(n >= 0, "read downloaded file"); if (!n) break; sink.body(data, unsigned(n)); }
    close(fd); check(sink.count == 24000 && sink.hash == expected(24000), "download file content");
    printf("httptest: FILE PASSED\n"); exit(0);
  }
  check(argc == 2, "host HTTP server port required"); unsigned port = atoi(argv[1]);
  request(port, "/fixed", 0, 24000);
  request(port, "/chunked", 0, 7000);
  request(port, "/eof", 0, 4000);
  request(port, "/informational", 0, 13);
  request(port, "/empty", 0);
  request(port, "/notfound", 0, 3);
  Options options; options.fail_http = true; request(port, "/notfound", HttpFailure, 0, options);
  options = {}; options.head = true; request(port, "/fixed", 0, 0, options);
  options = {}; options.follow = true;
  request(port, "/dir/start", 0, 17, options);
  request(port, "/absolute", 0, 17, options);
  request(port, "/network", 0, 17, options);
  request(port, "/query", 0, 17, options);
  options.max_redirects = 2; request(port, "/loop", RedirectLimit, 0, options);
  request(port, "/https", Unsupported, 0, options);
  const char *malformed[]{"/conflict", "/ambiguous", "/badheader", "/badchunk", "/badcrlf"};
  for (auto path : malformed) request(port, path, Protocol);
  request(port, "/short", Partial);
  request(port, "/chunkshort", Partial);
  request(port, "/encoding", Unsupported);
  options = {}; options.timeout_ms = 500;
  unsigned start = unsigned(uptime()); request(port, "/slow", Timeout, 0, options);
  check(unsigned(uptime())-start < 15, "whole-operation deadline");
  request(port, "/trickle", Timeout, 0, options);
  Capture sink; sink.reject = true; Response response; Client client; char url[128]; make_url(url, port, "/fixed");
  check(client.get(url, {}, sink, response) == Output, "sink failure");
  check(client.get("https://example.com/", {}, sink, response) == Unsupported, "HTTPS rejected");
  check(client.get("http://bad@host/", {}, sink, response) == InvalidUrl, "invalid authority rejected");
  local_dns();
  printf("httptest: ALL PASSED\n"); exit(0);
}
