module;
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "kernel/fs/poll.h"
#include "kernel/net/errno.h"
module xv.net;
import :registry;
import :loopback;
extern "C" int net_test_pages();
extern "C" void net_test_clock(unsigned long);

namespace
{
using namespace xv::net;

void check_registry(bool condition, const char *message)
{
  if (condition)
  {
    return;
  }
  std::fprintf(stderr, "TCP registry FAIL: %s\n", message);
  std::exit(1);
}

struct LocalStack
{
  InterfaceRegistry interfaces;
  RoutingTable routes{interfaces};
  Ipv4Layer ip{interfaces, routes};
  LoopbackInterface loopback;
  IcmpProtocol icmp{ip};
  UdpProtocol udp{ip, icmp};
  TcpProtocol tcp{ip};
  SocketRegistry sockets{ip, udp, tcp};

  LocalStack()
  {
    ip.initialize();
    loopback.initialize();
    sockets.initialize();
    check_registry(loopback.add_address({0x7f000001, 8}) && loopback.set_up(true) &&
                       interfaces.add(loopback) && routes.add(0x7f000000, 8, loopback),
                   "local stack configuration");
    loopback.attach(ip);
    ip.attach(6, tcp);
    tcp.attach(sockets);
    check_registry(interfaces.seal(), "local stack seal");
    routes.seal();
  }
};

unsigned long buffer_address(const void *data)
{
  return reinterpret_cast<unsigned long>(data);
}
} // namespace

extern "C" void run_tcp_registry_contracts()
{
  using namespace xv::net;
  int baseline = net_test_pages();
  unsigned long now = detail::milliseconds();
  LocalStack first, second;
  Endpoint endpoint{0x7f000001, 2022};
  auto *first_listener = first.sockets.create(1, 6);
  auto *second_listener = second.sockets.create(1, 6);
  auto *first_client = first.sockets.create(1, 6);
  auto *second_client = second.sockets.create(1, 6);
  check_registry(first_listener && second_listener && first_client && second_client,
                 "independent socket allocation");
  check_registry(!first_listener->bind(endpoint) && !second_listener->bind(endpoint) &&
                     !first_listener->listen(2) && !second_listener->listen(2),
                 "same listener address in two registries");
  check_registry(first_client->connect(endpoint, true) == -EINPROGRESS &&
                     second_client->connect(endpoint, true) == -EINPROGRESS &&
                     first_client->local().port == second_client->local().port,
                 "same client tuple in two registries");

  first.loopback.tick(now);
  int error = 0;
  check_registry(!second_listener->accept(true, error) && error == -EAGAIN &&
                     !(second_listener->poll(nullptr) & POLLIN),
                 "first stack delivery does not advance second handshake");
  check_registry(first_listener->poll(nullptr) & POLLIN,
                 "ready child reported through registry");
  auto *first_server = first_listener->accept(true, error);
  check_registry(first_server && !error && !(first_listener->poll(nullptr) & POLLIN),
                 "accept transfers ready child once");
  second.loopback.tick(now);
  auto *second_server = second_listener->accept(true, error);
  check_registry(second_server && !error,
                 "second stack handshake uses its own transmitter");

  char received[8]{};
  check_registry(first_client->write({buffer_address("first"), true}, 5) == 5,
                 "first stack write");
  first.loopback.tick(now);
  check_registry(
      first_server->read({buffer_address(received), true}, sizeof(received)) == 5 &&
          !std::memcmp(received, "first", 5) &&
          second_server->read({buffer_address(received), true}, sizeof(received)) ==
              -EAGAIN,
      "first payload remains in first registry");
  check_registry(second_client->write({buffer_address("second"), true}, 6) == 6,
                 "second stack write");
  second.loopback.tick(now);
  check_registry(
      second_server->read({buffer_address(received), true}, sizeof(received)) == 6 &&
          !std::memcmp(received, "second", 6) &&
          first_server->read({buffer_address(received), true}, sizeof(received)) ==
              -EAGAIN,
      "second payload remains in second registry");

  check_registry(!first_listener->listen(1), "set one pending connection backlog");
  auto *pending_client = first.sockets.create(1, 6);
  auto *overflow_client = first.sockets.create(1, 6);
  check_registry(pending_client && overflow_client &&
                     pending_client->connect(endpoint, true) == -EINPROGRESS &&
                     overflow_client->connect(endpoint, true) == -EINPROGRESS,
                 "two new clients compete for backlog");
  first.loopback.tick(now);
  check_registry((first_listener->poll(nullptr) & POLLIN) &&
                     !(overflow_client->poll(nullptr) & POLLOUT) &&
                     !overflow_client->error(),
                 "full backlog silently defers additional SYN");
  auto *pending_server = first_listener->accept(true, error);
  check_registry(pending_server && !error, "accept frees a pending slot");
  net_test_clock(now + 1001);
  first.sockets.tick(now + 1001);
  first.loopback.tick(now + 1001);
  check_registry((overflow_client->poll(nullptr) & POLLOUT) &&
                     (first_listener->poll(nullptr) & POLLIN),
                 "deferred SYN retry enters the freed backlog slot");
  first_listener->close();
  first.loopback.tick(now + 1001);
  check_registry(overflow_client->error() == ECONNRESET,
                 "listener close resets its unaccepted child");
  check_registry(first_client->write({buffer_address("kept"), true}, 4) == 4,
                 "accepted child survives listener close");
  first.loopback.tick(now + 1001);
  check_registry(
      first_server->read({buffer_address(received), true}, sizeof(received)) == 4 &&
          !std::memcmp(received, "kept", 4),
      "accepted connection is detached from pending children");
  pending_client->close();
  pending_server->close();
  overflow_client->close();

  first_client->close();
  first_server->close();
  second_client->close();
  second_server->close();
  second_listener->close();
  first.loopback.tick(now);
  second.loopback.tick(now);
  net_test_clock(now + 60000);
  first.sockets.tick(now + 60000);
  second.sockets.tick(now + 60000);
  first.loopback.tick(now + 60000);
  second.loopback.tick(now + 60000);
  net_test_clock(now);
  check_registry(net_test_pages() == baseline,
                 "independent stacks release connections and queued pages");
  std::puts(
      "TCP registry: same tuples, isolated handshakes, payloads and lifecycle passed");
}
