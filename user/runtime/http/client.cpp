#include "kernel/support/types.h"
#include "kernel/fs/poll.h"
#include "user/runtime/user.h"
#include "codec.h"
namespace xv::http
{
namespace
{
class Deadline
{
  uint64 ends_;
public:
  explicit Deadline(unsigned timeout) : ends_(clockticks()+uint64(timeout)*10000) {}
  unsigned remaining() const
  { auto now=clockticks(); return now>=ends_ ? 0 : unsigned((ends_-now+9999)/10000); }
  int wait(int fd, short events) const
  {
    unsigned left = remaining(); if (!left) return Timeout;
    pollfd p{fd, events, 0}; int status = poll(&p, 1, int(left));
    if (status < 0) return Connect;
    if (!status) return Timeout;
    return p.revents & (events | POLLHUP) ? 0 : Connect;
  }
};
class Connection
{
  int fd_ = -1;
  Deadline &deadline_;
  char *data_;
  Response &response_;
  uint64 began_;
  bool saw_first_ = false;
  unsigned cursor_ = 0, used_ = 0;
public:
  Connection(Deadline &deadline, char *buffer, Response &response, uint64 began)
      : deadline_(deadline), data_(buffer), response_(response), began_(began) {}
  ~Connection() { reset(); }
  void reset() { if (fd_ >= 0) close(fd_); fd_ = -1; cursor_ = used_ = 0; }
  int open(const Url &url, dns_result &addresses, unsigned connect_timeout)
  {
    reset(); dns_options options{};
    auto started=clockticks();
    unsigned budget=deadline_.remaining();
    if(connect_timeout && connect_timeout<budget)budget=connect_timeout;
    Deadline connecting(budget);
    options.total_timeout_ms = budget;
    if (!options.total_timeout_ms) return Timeout;
    int status = dns_lookup(url.host, &addresses, &options);
    response_.namelookup_us+=(clockticks()-started)/10;
    if (status) return status == DNS_TIMEOUT ? Timeout : Resolve;
    for (unsigned i = 0; i < addresses.count; ++i)
    {
      if (!connecting.remaining()) return Timeout;
      fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
      if (fd_ < 0) return Connect;
      sockaddr_in peer{}; peer.sin_family = AF_INET; peer.sin_port = htons(url.port); peer.sin_addr.s_addr = htonl(addresses.addresses[i]);
      status = connect(fd_, reinterpret_cast<sockaddr *>(&peer), sizeof(peer));
      if (status == -115)
      {
        status = connecting.wait(fd_, POLLOUT);
        if (!status)
        {
          int error = 0; socklen_t size = sizeof(error);
          status = getsockopt(fd_, SOL_SOCKET, SO_ERROR, &error, &size) || error ? Connect : 0;
        }
      }
      if (!status) { ++response_.connects;response_.connect_us+=(clockticks()-started)/10;return 0; }
      reset(); if (status == Timeout) return Timeout;
    }
    return Connect;
  }
  int send(const char *data, unsigned length)
  {
    unsigned done = 0;
    while (done < length)
    {
      if (!deadline_.remaining()) return Timeout;
      ++response_.socket_writes;
      int n = write(fd_, data+done, int(length-done));
      if (n == -11) { int status = deadline_.wait(fd_, POLLOUT); if (status) return status; }
      else if (n <= 0) return Connect;
      else done += unsigned(n);
    }
    return 0;
  }
  int receive(char *data, unsigned length)
  {
    if (!deadline_.remaining()) return -Timeout;
    if (cursor_ < used_)
    {
      unsigned count = used_-cursor_; if (count > length) count = length;
      memmove(data, data_+cursor_, int(count)); cursor_ += count; return int(count);
    }
    for (;;)
    {
      if (!deadline_.remaining()) return -Timeout;
      ++response_.socket_reads;
      int n = read(fd_, data, int(length));
      if(n>0 && !saw_first_) { response_.first_byte_us=(clockticks()-began_)/10;saw_first_=true; }
      if (n != -11) return n < 0 ? -Connect : n;
      int status = deadline_.wait(fd_, POLLIN); if (status) return -status;
    }
  }
  int line(char *out, unsigned capacity)
  {
    unsigned n = 0; bool cr = false;
    for (;;)
    {
      if (cursor_ == used_)
      {
        cursor_ = used_ = 0;
        int received = receive(data_, 8192);
        if (received <= 0) return received < 0 ? -received : Partial;
        used_ = unsigned(received);
      }
      char c = data_[cursor_++];
      if (cr) { if (c != '\n') return Protocol; out[n] = 0; return 0; }
      if (c == '\r') { cr = true; continue; }
      if (!c || c == '\n' || n+1 >= capacity) return Protocol;
      out[n++] = c;
    }
  }
};
struct Workspace
{
  Url current{}, next{};
  HeaderParser parser;
  char line[line_size];
  char request[target_size+512];
  char body[8192];
  char network[8192];
  dns_result addresses;
};
class RequestBuilder
{
  char *buffer_; unsigned capacity_, used_ = 0;
public:
  RequestBuilder(char *buffer, unsigned capacity) : buffer_(buffer), capacity_(capacity) {}
  bool append(const char *text)
  { while (*text) { if (used_ == capacity_) return false; buffer_[used_++] = *text++; } return true; }
  bool port(unsigned port)
  { char digits[6]; unsigned n = 0; do { digits[n++] = char('0'+port%10); port /= 10; } while (port); while (n) { char c[2]{digits[--n], 0}; if (!append(c)) return false; } return true; }
  unsigned size() const { return used_; }
};
bool redirect(unsigned code) { return code == 301 || code == 302 || code == 303 || code == 307 || code == 308; }
int emit_header(Sink &sink, const char *line)
{ return sink.headers(line, strlen(line)) && sink.headers("\r\n", 2) ? 0 : Output; }
int exact_body(Connection &connection, Workspace &w, unsigned long long count, Sink &sink, Response &response)
{
  while (count)
  {
    unsigned take = count > sizeof(w.body) ? sizeof(w.body) : unsigned(count);
    int n = connection.receive(w.body, take);
    if (n <= 0) return n < 0 ? -n : Partial;
    if (!sink.body(w.body, unsigned(n))) return Output;
    if (response.body_bytes > ~0ULL-unsigned(n)) return Protocol;
    response.body_bytes += unsigned(n); count -= unsigned(n);
  }
  return 0;
}
int read_body(Connection &connection, Workspace &w, Sink &sink, Response &response)
{
  if (w.parser.value.chunked)
  {
    for (;;)
    {
      int status = connection.line(w.line, sizeof(w.line)); if (status) return status;
      unsigned long long size;
      status = chunk_size(w.line, size); if (status) return status;
      if (!size) break;
      status = exact_body(connection, w, size, sink, response); if (status) return status;
      status = connection.line(w.line, sizeof(w.line)); if (status) return status;
      if (w.line[0]) return Protocol;
    }
    unsigned total = 0;
    for (;;)
    {
      int status = connection.line(w.line, sizeof(w.line)); if (status) return status;
      total += strlen(w.line)+2; if (total > 16384) return Protocol;
      if (!w.line[0]) return 0;
      status = w.parser.field(w.line, true); if (status) return status;
    }
  }
  if (w.parser.value.has_length) return exact_body(connection, w, w.parser.value.length, sink, response);
  for (;;)
  {
    int n = connection.receive(w.body, sizeof(w.body));
    if (n <= 0) return n < 0 ? -n : 0;
    if (!sink.body(w.body, unsigned(n))) return Output;
    if (response.body_bytes > ~0ULL-unsigned(n)) return Protocol;
    response.body_bytes += unsigned(n);
  }
}
int perform(Workspace &w, const Options &options, Sink &sink, Response &response, uint64 began)
{
  Deadline deadline(options.timeout_ms); Connection connection(deadline, w.network, response, began);
  for (;;)
  {
    int status = connection.open(w.current, w.addresses, options.connect_timeout_ms); if (status) return status;
    RequestBuilder request(w.request, sizeof(w.request));
    if (!request.append(options.head ? "HEAD " : "GET ") || !request.append(w.current.target) ||
        !request.append(" HTTP/1.1\r\nHost: ") || !request.append(w.current.host)) return InvalidUrl;
    if (w.current.port != 80 && (!request.append(":") || !request.port(w.current.port))) return InvalidUrl;
    if (!request.append("\r\nUser-Agent: xv-curl/1\r\nAccept: */*\r\nAccept-Encoding: identity\r\nConnection: close\r\n\r\n")) return InvalidUrl;
    status = connection.send(w.request, request.size()); if (status) return status;
    unsigned informational = 0;
    for (;;)
    {
      status = connection.line(w.line, sizeof(w.line)); if (status) return status;
      status = w.parser.status_line(w.line); if (status) return status;
      if (options.head || options.include_headers) { status = emit_header(sink, w.line); if (status) return status; }
      unsigned total = strlen(w.line)+2;
      for (;;)
      {
        status = connection.line(w.line, sizeof(w.line)); if (status) return status;
        total += strlen(w.line)+2; if (total > 16384) return Protocol;
        if (options.head || options.include_headers) { status = emit_header(sink, w.line); if (status) return status; }
        if (!w.line[0]) break;
        status = w.parser.field(w.line); if (status) return status;
      }
      status = w.parser.finish(); if (status) return status;
      if (w.parser.value.status >= 200) break;
      if (w.parser.value.status == 101 || ++informational > 8) return Protocol;
    }
    response.status = w.parser.value.status;
    if (options.follow && redirect(response.status) && w.parser.value.has_location)
    {
      if (response.redirects >= options.max_redirects) return RedirectLimit;
      status = resolve_location(w.current, w.parser.value.location, w.next); if (status) return status;
      w.current = w.next; ++response.redirects; connection.reset(); continue;
    }
    if (options.fail_http && response.status >= 400) return HttpFailure;
    if (options.head || response.status == 204 || response.status == 304) return 0;
    return read_body(connection, w, sink, response);
  }
}
}
int Client::get(const char *url, const Options &options, Sink &sink, Response &response)
{
  response = {};
  const auto began=clockticks();
  if (!options.timeout_ms || options.timeout_ms > 3600000 || options.max_redirects > 20 || options.connect_timeout_ms > 3600000) return InvalidUrl;
  Workspace *w = static_cast<Workspace *>(malloc(sizeof(Workspace)));
  if (!w) return Memory;
  *w = {};
  int status = parse_url(url, w->current);
  if (!status) status = perform(*w, options, sink, response, began);
  free(w); response.total_us=(clockticks()-began)/10;return status;
}
const char *error_string(int error)
{
  switch (error)
  {
    case Ok: return "success";
    case Unsupported: return "unsupported protocol or encoding (HTTP only)";
    case InvalidUrl: return "invalid URL or options";
    case Resolve: return "could not resolve host";
    case Connect: return "connection or socket error";
    case Protocol: return "malformed HTTP response";
    case Partial: return "response ended before the message was complete";
    case HttpFailure: return "HTTP status indicates failure";
    case Output: return "could not write output";
    case Memory: return "out of memory";
    case Timeout: return "operation timed out";
    case RedirectLimit: return "too many redirects";
    default: return "unknown HTTP error";
  }
}
}
