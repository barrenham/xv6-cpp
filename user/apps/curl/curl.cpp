#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"
#include "user/runtime/http/http.h"
namespace
{
class FileSink final : public xv::http::Sink
{
  int fd_;
  bool write_all(const char *data, unsigned size)
  {
    while (size)
    { int n = write(fd_, data, int(size)); if (n <= 0) return false; data += n; size -= unsigned(n); }
    return true;
  }
public:
  explicit FileSink(int fd) : fd_(fd) {}
  bool headers(const char *data, unsigned size) override { return write_all(data, size); }
  bool body(const char *data, unsigned size) override { return write_all(data, size); }
};
void usage()
{ fprintf(2, "usage: curl [-i] [-I] [-L] [-f] [-o FILE] [-m SECONDS] HTTP-URL\n"); }
bool seconds(const char *text, unsigned &ms)
{
  unsigned value = 0;
  if (!*text) return false;
  while (*text)
  { if (*text < '0' || *text > '9' || value > 3600) return false; value = value*10+unsigned(*text++-'0'); }
  if (!value || value > 3600) return false;
  ms = value*1000; return true;
}
}
extern "C" int main(int argc, char **argv)
{
  xv::http::Options options; const char *url = nullptr, *output = nullptr; bool positional = false;
  for (int i = 1; i < argc; ++i)
  {
    const char *arg = argv[i];
    if (!positional && !strcmp(arg, "--")) { positional = true; continue; }
    if (!positional && (!strcmp(arg, "--help") || !strcmp(arg, "-h"))) { usage(); exit(0); }
    if (!positional && (!strcmp(arg, "-o") || !strcmp(arg, "--output")))
    { if (++i == argc || output) { usage(); exit(2); } output = argv[i]; continue; }
    if (!positional && (!strcmp(arg, "-m") || !strcmp(arg, "--max-time")))
    { if (++i == argc || !seconds(argv[i], options.timeout_ms)) { usage(); exit(2); } continue; }
    if (!positional && arg[0] == '-')
    {
      if (!strcmp(arg, "--location")) { options.follow = true; continue; }
      if (!strcmp(arg, "--head")) { options.head = true; continue; }
      if (!strcmp(arg, "--include")) { options.include_headers = true; continue; }
      if (!strcmp(arg, "--fail")) { options.fail_http = true; continue; }
      if (!arg[1]) { usage(); exit(2); }
      for (unsigned j = 1; arg[j]; ++j)
      {
        switch (arg[j])
        {
          case 'i': options.include_headers = true; break;
          case 'I': options.head = true; break;
          case 'L': options.follow = true; break;
          case 'f': options.fail_http = true; break;
          default: usage(); exit(2);
        }
      }
    }
    else { if (url) { usage(); exit(2); } url = arg; }
  }
  if (!url) { usage(); exit(2); }
  int fd = 1;
  if (output && strcmp(output, "-"))
  { fd = open(output, O_WRONLY | O_CREATE | O_TRUNC); if (fd < 0) { fprintf(2, "curl: cannot open %s\n", output); exit(23); } }
  FileSink sink(fd); xv::http::Client client; xv::http::Response response;
  int status = client.get(url, options, sink, response);
  if (fd != 1 && close(fd) && !status) status = xv::http::Output;
  if (status) fprintf(2, "curl: (%d) %s (HTTP %u)\n", status, xv::http::error_string(status), response.status);
  exit(status);
}
