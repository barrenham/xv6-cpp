#include "kernel/support/types.h"
#include "kernel/fs/poll.h"
#include "user/runtime/user.h"
#include "codec.h"
namespace
{
struct Buffer
{
  unsigned char query[272];
  unsigned char reply[4096];
  char name[DNS_NAME_SIZE];
  dns_result answer;
};
struct Descriptor
{
  int fd;
  ~Descriptor() { if (fd >= 0) close(fd); }
};
netctl_request request(unsigned id = 0)
{ netctl_request r{}; r.version = NETCTL_VERSION; r.size = sizeof(r); r.ifindex = id; return r; }
__attribute__((noinline)) int select_server(unsigned &server, unsigned &source)
{
  unsigned best = ~0U; bool selected = false;
  for (unsigned index = 0; index < 8; ++index)
  {
    auto link = request(); link.index = index;
    if (netctl(NETCTL_LINK, &link) != 1) break;
    if ((link.flags & (NET_LINK_UP | NET_LINK_CARRIER | NET_LINK_SLAVE)) != (NET_LINK_UP | NET_LINK_CARRIER)) continue;
    auto lease = request(link.ifindex);
    if (netctl(NETCTL_DHCP_STATUS, &lease) != 1 || lease.flags < DHCP_BOUND || lease.flags > DHCP_REBINDING || !lease.source) continue;
    unsigned metric = ~0U;
    for (unsigned i = 0; i < 32; ++i)
    {
      auto route = request(); route.index = i;
      if (netctl(NETCTL_ROUTE, &route) != 1) break;
      if (route.ifindex == link.ifindex && route.prefix == 0 && route.metric < metric) metric = route.metric;
    }
    if (!selected || metric < best)
    { best = metric; server = lease.source; source = lease.address; selected = true; }
  }
  return selected ? 0 : DNS_NO_SERVER;
}
sockaddr_in endpoint(unsigned ip, unsigned port)
{ sockaddr_in r{}; r.sin_family = AF_INET; r.sin_port = htons(port); r.sin_addr.s_addr = htonl(ip); return r; }
unsigned elapsed(unsigned start) { return (unsigned(uptime()) - start) * 100U; }
int ready(int fd, short events, unsigned start, unsigned timeout)
{
  unsigned spent = elapsed(start);
  if (spent >= timeout) return DNS_TIMEOUT;
  pollfd p{fd, events, 0}; int r = poll(&p, 1, int(timeout - spent));
  if (r < 0) return DNS_NETWORK;
  if (!r) return DNS_TIMEOUT;
  if (p.revents & events) return 0;
  return DNS_NETWORK;
}
int open_peer(bool tcp, unsigned server, unsigned source, unsigned port, unsigned start, unsigned timeout)
{
  int fd = socket(AF_INET, (tcp ? SOCK_STREAM : SOCK_DGRAM) | SOCK_NONBLOCK, 0);
  if (fd < 0) return DNS_NETWORK;
  if (source)
  {
    auto local = endpoint(source, 0);
    if (bind(fd, reinterpret_cast<sockaddr *>(&local), sizeof(local))) { close(fd); return DNS_NETWORK; }
  }
  auto peer = endpoint(server, port);
  int status = connect(fd, reinterpret_cast<sockaddr *>(&peer), sizeof(peer));
  if (tcp && status == -115)
  {
    status = ready(fd, POLLOUT, start, timeout);
    int error = 0; socklen_t size = sizeof(error);
    if (!status && (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &size) || error)) status = DNS_NETWORK;
  }
  if (status) { close(fd); return status == DNS_TIMEOUT ? status : DNS_NETWORK; }
  return fd;
}
int transfer(int fd, unsigned char *data, unsigned length, bool writing, unsigned start, unsigned timeout)
{
  unsigned offset = 0;
  while (offset < length)
  {
    if (elapsed(start) >= timeout) return DNS_TIMEOUT;
    int n = writing ? write(fd, data + offset, int(length - offset)) : read(fd, data + offset, int(length - offset));
    if (n == -11)
    { int status = ready(fd, writing ? POLLOUT : POLLIN, start, timeout); if (status) return status; }
    else if (n <= 0) return DNS_NETWORK;
    else offset += unsigned(n);
  }
  return 0;
}
int tcp_query(Buffer &buffer, unsigned length, unsigned server, unsigned source, unsigned port, unsigned timeout)
{
  unsigned start = unsigned(uptime());
  Descriptor peer{open_peer(true, server, source, port, start, timeout)};
  if (peer.fd < 0) return peer.fd;
  unsigned char prefix[2]{static_cast<unsigned char>(length >> 8), static_cast<unsigned char>(length)};
  int status = transfer(peer.fd, prefix, 2, true, start, timeout);
  if (!status) status = transfer(peer.fd, buffer.query, length, true, start, timeout);
  if (!status) status = transfer(peer.fd, prefix, 2, false, start, timeout);
  if (status) return status;
  unsigned size = (unsigned(prefix[0]) << 8) | prefix[1];
  if (size > sizeof(buffer.reply)) return DNS_LIMIT;
  if (size < 12) return DNS_MALFORMED;
  status = transfer(peer.fd, buffer.reply, size, false, start, timeout);
  return status ? status : int(size);
}
int query(Buffer &buffer, const char *name, unsigned server, unsigned source, const dns_options &options, dns_result &result, unsigned began)
{
  int last = DNS_TIMEOUT;
  for (unsigned attempt = 0; attempt < options.attempts; ++attempt)
  {
    unsigned timeout = options.timeout_ms;
    if (options.total_timeout_ms)
    {
      unsigned spent = elapsed(began);
      if (spent >= options.total_timeout_ms) return DNS_TIMEOUT;
      unsigned left = options.total_timeout_ms - spent;
      if (timeout > left) timeout = left;
    }
    unsigned start = unsigned(uptime());
    Descriptor peer{open_peer(false, server, source, options.port, start, timeout)};
    if (peer.fd < 0) return peer.fd;
    sockaddr_in local{}; socklen_t size = sizeof(local);
    if (getsockname(peer.fd, reinterpret_cast<sockaddr *>(&local), &size)) return DNS_NETWORK;
    unsigned short id = static_cast<unsigned short>((start * 40503U) ^ (unsigned(getpid()) * 7919U) ^ ntohs(local.sin_port));
    int length = xv::dns::make_query(name, id, buffer.query);
    if (length < 0) return length;
    if (write(peer.fd, buffer.query, length) != length) return DNS_NETWORK;
    for (;;)
    {
      int status = ready(peer.fd, POLLIN, start, timeout);
      if (status) { last = status; break; }
      int n = read(peer.fd, buffer.reply, 513);
      if (n == -11) continue;
      if (n < 0) return DNS_NETWORK;
      if (n > 512) { last = DNS_MALFORMED; continue; }
      status = xv::dns::parse_reply(buffer.reply, unsigned(n), id, name, result);
      if (status == DNS_MALFORMED) { last = status; continue; }
      if (status == xv::dns::truncated)
      {
        unsigned tcp_timeout = options.timeout_ms;
        if (options.total_timeout_ms)
        {
          unsigned spent = elapsed(began);
          if (spent >= options.total_timeout_ms) return DNS_TIMEOUT;
          unsigned left = options.total_timeout_ms - spent;
          if (tcp_timeout > left) tcp_timeout = left;
        }
        n = tcp_query(buffer, unsigned(length), server, source, options.port, tcp_timeout);
        if (n < 0) { last = n; break; }
        status = xv::dns::parse_reply(buffer.reply, unsigned(n), id, name, result);
        if (status == xv::dns::truncated) status = DNS_MALFORMED;
      }
      if (status == DNS_SERVER_ERROR) { last = status; break; }
      return status;
    }
  }
  return last;
}
}
extern "C" int dns_lookup(const char *name, dns_result *result, const dns_options *configuration)
{
  if (!result) return DNS_INVALID;
  *result = {};
  Buffer *buffer = static_cast<Buffer *>(malloc(sizeof(Buffer)));
  if (!buffer) return DNS_MEMORY;
  auto &current = buffer->name;
  if (xv::dns::normalize(name, current)) { free(buffer); return DNS_INVALID; }
  unsigned literal;
  if (!dns_parse_ipv4(name, &literal))
  { result->addresses[0] = literal; result->count = 1; strcpy(result->canonical, current); free(buffer); return 0; }
  dns_options options = configuration ? *configuration : dns_options{};
  if (!options.port) options.port = 53;
  if (!options.timeout_ms) options.timeout_ms = 1500;
  if (!options.attempts) options.attempts = 3;
  if (options.timeout_ms > 30000 || options.attempts > 5 || options.total_timeout_ms > 3600000)
  { free(buffer); return DNS_INVALID; }
  unsigned server = options.server, source = 0;
  if (!server) { int status = select_server(server, source); if (status) { free(buffer); return status; } }
  unsigned began = unsigned(uptime());
  auto &answer = buffer->answer; answer = {}; unsigned ttl = ~0U; int status = DNS_LIMIT;
  for (unsigned depth = 0; depth < 9; ++depth)
  {
    status = query(*buffer, current, server, source, options, answer, began);
    if (status != 0 && status != xv::dns::alias) break;
    if (answer.ttl < ttl) ttl = answer.ttl;
    if (!status) { answer.ttl = ttl; answer.server = server; *result = answer; break; }
    strcpy(current, answer.canonical);
    status = DNS_LIMIT;
  }
  free(buffer); return status;
}
extern "C" const char *dns_strerror(int status)
{
  switch (status)
  {
    case 0: return "success";
    case DNS_INVALID: return "invalid name or options";
    case DNS_NO_SERVER: return "no DHCP DNS server; specify a server address";
    case DNS_TIMEOUT: return "DNS query timed out";
    case DNS_NOT_FOUND: return "name does not exist (NXDOMAIN)";
    case DNS_NO_ADDRESS: return "no IPv4 address record";
    case DNS_SERVER_ERROR: return "DNS server failed or refused the query";
    case DNS_MALFORMED: return "invalid DNS response";
    case DNS_NETWORK: return "socket or network error";
    case DNS_LIMIT: return "DNS message or CNAME chain exceeds limits";
    case DNS_MEMORY: return "out of memory";
    default: return "unknown DNS error";
  }
}
