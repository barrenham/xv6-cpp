#pragma once
namespace xv::http
{
enum Error { Ok = 0, Unsupported = 1, InvalidUrl = 3, Resolve = 6, Connect = 7,
             Protocol = 8, Partial = 18, HttpFailure = 22, Output = 23,
             Memory = 27, Timeout = 28, RedirectLimit = 47 };
struct Options
{
  unsigned timeout_ms = 30000;
  unsigned max_redirects = 5;
  bool head = false;
  bool follow = false;
  bool include_headers = false;
  bool fail_http = false;
};
struct Response
{
  unsigned status = 0;
  unsigned redirects = 0;
  unsigned long long body_bytes = 0;
};
class Sink
{
public:
  virtual bool headers(const char *data, unsigned size) = 0;
  virtual bool body(const char *data, unsigned size) = 0;
protected:
  ~Sink() = default;
};
class Client
{
public:
  int get(const char *url, const Options &options, Sink &sink, Response &response);
};
const char *error_string(int error);
}
