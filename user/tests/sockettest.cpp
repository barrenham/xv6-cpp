#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/poll.h"
#include "kernel/net/errno.h"
#include "user/runtime/user.h"
namespace
{
char sent[262144], received[262144];
const socklen_t fixed_capacity = sizeof(sockaddr_in);
void check(bool good, const char *message)
{ if (!good) { fprintf(2, "sockettest: FAIL: %s\n", message); exit(1); } }
sockaddr_in address(unsigned ip, unsigned port)
{ sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(port); a.sin_addr.s_addr = htonl(ip); return a; }
sockaddr *sa(sockaddr_in &a) { return reinterpret_cast<sockaddr *>(&a); }
void write_all(int fd, const char *data, int length)
{
  for (int offset = 0; offset < length;)
  { int n = write(fd, data+offset, length-offset); if (n<=0) fprintf(2,"sockettest: write result %d at %d/%d\n",n,offset,length); check(n > 0, "stream write"); offset += n; }
}
void read_all(int fd, char *data, int length)
{
  for (int offset = 0; offset < length;)
  { int n = read(fd, data+offset, length-offset); check(n > 0, "stream read"); offset += n; }
}
void wait_ok(int child)
{ int status = -1; check(waitpid(child, &status, 0) == child && status == 0, "child exit"); }
}
extern "C" int main(int argc, char **argv)
{
  check(argc == 4, "host UDP/TCP/refused ports required");
  unsigned udp_port = atoi(argv[1]), tcp_port = atoi(argv[2]), refused_port = atoi(argv[3]);
  for (int i = 0; i < sizeof(sent); ++i) sent[i] = i*19+7;
  int udp = socket(AF_INET, SOCK_DGRAM|SOCK_NONBLOCK, 0);
  check(udp >= 0, "UDP socket");
  struct stat info; check(fstat(udp, &info) == 0 && info.type == T_SOCKET, "socket stat");
  auto local = address(INADDR_ANY, 2020);
  check(bind(udp, sa(local), sizeof(local)) == 0, "UDP bind");
  unsigned size = sizeof(local);
  check(getsockname(udp, sa(local), &size) == 0 && ntohs(local.sin_port) == 2020, "local name");
  auto truncated = address(0x11223344, 0x5566);
  size = sizeof(truncated.sin_family);
  check(getsockname(udp, sa(truncated), &size) == 0 && size == sizeof(truncated) &&
        truncated.sin_family == AF_INET && ntohs(truncated.sin_port) == 0x5566,
        "name output respects capacity and reports full length");
  truncated = {};
  check(getsockname(udp, sa(truncated), const_cast<socklen_t *>(&fixed_capacity)) == -1 &&
        ntohs(truncated.sin_port) == 2020, "name length failure retains first output");
  check(recv(udp, received, 10, 0) == -EAGAIN, "nonblocking UDP");
  pollfd p{udp, POLLIN, 0}; check(poll(&p, 1, 0) == 0, "UDP poll empty");
  auto host = address(0x0a000202, udp_port);
  check(sendto(udp, sent, 1301, 0, sa(host), sizeof(host)) == 1301, "sendto");
  check(poll(&p, 1, 3000) == 1 && (p.revents&POLLIN), "UDP poll ready");
  sockaddr_in peer{}; size = sizeof(peer);
  check(recvfrom(udp, received, sizeof(received), 0, sa(peer), &size) == 1301 &&
        ntohs(peer.sin_port) == udp_port && !memcmp(sent, received, 1301), "recvfrom");
  check(connect(udp, sa(host), sizeof(host)) == 0, "UDP connect");
  int duplicate = dup(udp); check(duplicate >= 0, "dup socket");
  check(fcntl(duplicate, F_SETFL, 0) == 0 && !(fcntl(udp, F_GETFL)&O_NONBLOCK), "shared status flags");
  close(duplicate);
  int child = fork(); check(child >= 0, "fork socket");
  if (!child) { check(write(udp, sent, 31) == 31 && read(udp, received, sizeof(received)) == 31, "inherited UDP"); exit(0); }
  wait_ok(child);
  check(write(udp, sent, 0) == 0 && read(udp, received, sizeof(received)) == 0, "empty UDP");
  check(close(udp) == 0, "close UDP");
  printf("sockettest: UDP descriptors, poll, dup/fork OK\n");

  int tcp = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, IPPROTO_TCP);
  check(tcp >= 0, "TCP socket"); host = address(0x0a000202, tcp_port);
  check(connect(tcp, sa(host), sizeof(host)) == -EINPROGRESS, "nonblocking TCP connect");
  p = {tcp, POLLOUT, 0}; check(poll(&p, 1, 5000) == 1 && (p.revents&POLLOUT), "connect poll");
  int error = -1; size = sizeof(error);
  check(getsockopt(tcp, SOL_SOCKET, SO_ERROR, &error, &size) == 0 && error == 0, "connect status");
  check(fcntl(tcp, F_SETFL, 0) == 0, "blocking TCP");
  size = sizeof(peer); check(getpeername(tcp, sa(peer), &size) == 0 && ntohs(peer.sin_port) == tcp_port, "peer name");
  write_all(tcp, sent, 262144);
  check(shutdown(tcp, SHUT_WR) == 0, "TCP half close");
  read_all(tcp, received, 262144);
  check(!memcmp(sent, received, 262144), "TCP stream content");
  check(read(tcp, received, 1) == 0, "TCP EOF"); close(tcp);
  tcp = socket(AF_INET, SOCK_STREAM, 0); host = address(0x0a000202, refused_port);
  check(connect(tcp, sa(host), sizeof(host)) == -ECONNREFUSED, "TCP refusal"); close(tcp);
  printf("sockettest: TCP active open, stream, half close, refusal OK\n");

  int server = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
  local = address(INADDR_ANY, 2021);
  check(server >= 0 && bind(server, sa(local), sizeof(local)) == 0 && listen(server, 4) == 0, "listen");
  check(accept(server, 0, 0) == -EAGAIN, "nonblocking accept");
  printf("sockettest: listening\n");
  p = {server, POLLIN, 0}; check(poll(&p, 1, 10000) == 1, "listener poll");
  size = sizeof(peer); int connection = accept(server, sa(peer), &size);
  check(connection >= 0, "accept");
  read_all(connection, received, 262144); write_all(connection, received, 262144);
  check(shutdown(connection, SHUT_WR) == 0, "accepted shutdown");
  check(read(connection, received, 1) == 0, "accepted peer EOF");
  close(connection); close(server);
  printf("sockettest: TCP passive open OK\n");

  server = socket(AF_INET, SOCK_STREAM, 0); local = address(INADDR_LOOPBACK, 0);
  check(server >= 0 && bind(server, sa(local), sizeof(local)) == 0 && listen(server, 2) == 0, "loopback listen");
  size = sizeof(local); check(getsockname(server, sa(local), &size) == 0, "loopback name");
  child = fork(); check(child >= 0, "loopback fork");
  if (!child)
  {
    close(server); int client = socket(AF_INET, SOCK_STREAM, 0);
    check(connect(client, sa(local), sizeof(local)) == 0, "loopback connect");
    write_all(client, sent, 1500); read_all(client, received, 1500);
    check(!memcmp(sent, received, 1500), "loopback data"); close(client); exit(0);
  }
  connection = accept(server, 0, 0); check(connection >= 0, "loopback accept");
  read_all(connection, received, 1500); write_all(connection, received, 1500);
  close(connection); close(server); wait_ok(child);
  printf("sockettest: loopback OK\n");
  printf("sockettest: ALL PASSED\n"); exit(0);
}
