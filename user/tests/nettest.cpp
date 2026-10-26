#include "kernel/support/types.h"
#include "kernel/fs/io_status.h"
#include "user/runtime/user.h"

namespace
{
constexpr uint32 Host = 0x0a000202;
int host_port = 26099;
volatile int caught = 0;
void handler(int) { caught = 1; }
void check(bool value, const char *message)
{ if (!value) { fprintf(2, "nettest: FAIL: %s\n", message); exit(1); } }
int datagram(unsigned port)
{
  int fd = socket(AF_INET, SOCK_DGRAM, 0); check(fd >= 0, "UDP socket");
  sockaddr_in local{}; local.sin_family = AF_INET; local.sin_port = htons(port);
  if (bind(fd, reinterpret_cast<sockaddr *>(&local), sizeof(local)) < 0) { close(fd); return -1; }
  return fd;
}
int send_echo(int fd, const char *data, int length)
{
  sockaddr_in peer{}; peer.sin_family = AF_INET; peer.sin_addr.s_addr = htonl(Host); peer.sin_port = htons(host_port);
  return sendto(fd, data, length, 0, reinterpret_cast<sockaddr *>(&peer), sizeof(peer));
}
void echo(int fd, char *data, int length, int capacity = 1472)
{
  int result;
  do { result = send_echo(fd, data, length); } while (result == -EAGAIN);
  check(result == length, "sendto");
  sockaddr_in source{}; socklen_t size = sizeof(source); char received[1472];
  int n = recvfrom(fd, received, capacity, 0, reinterpret_cast<sockaddr *>(&source), &size);
  int expected = length < capacity ? length : capacity;
  check(n == expected && ntohl(source.sin_addr.s_addr) == Host && ntohs(source.sin_port) == host_port &&
        memcmp(data, received, expected) == 0, "UDP echo metadata/content");
}
void wait_ok(int pid)
{ int status = -1; check(pid > 0 && waitpid(pid, &status, 0) == pid && status == 0, "child status"); }
}
extern "C" int main(int argc, char **argv)
{
  if (argc == 2) host_port = atoi(argv[1]);
  int fd = datagram(2000); check(fd >= 0 && datagram(2000) < 0, "exclusive bind");
  char data[1473]; for (int i = 0; i < 1473; ++i) data[i] = i * 13;
  for (int i = 0; i < 80; ++i) echo(fd, data, 1 + i * 17);
  echo(fd, data, 1472); echo(fd, data, 0); echo(fd, data, 31, 7); echo(fd, data, 3);
  check(send_echo(fd, data, 1473) < 0, "MTU bound");
  check(send_echo(fd, reinterpret_cast<char *>(~0UL), 1) < 0, "bad send address");
  check(recvfrom(-1, data, sizeof(data), 0, nullptr, nullptr) < 0, "invalid receive descriptor");
  printf("nettest: ring wrap, MTU, zero length, truncation OK\n");

  int child = fork(); check(child >= 0, "fork socket");
  if (!child)
  {
    check(close(fd) == 0 && datagram(2000) < 0, "shared socket lifetime");
    int own = datagram(2001); check(own >= 0, "child bind"); echo(own, data, 17); exit(0);
  }
  wait_ok(child); echo(fd, data, 29);
  int shared = datagram(2001); check(shared >= 0, "exit released child socket");
  child = fork(); check(child >= 0, "fork receive");
  if (!child) { echo(shared, data, 23); exit(0); }
  wait_ok(child); check(close(shared) == 0, "close shared socket");
  shared = datagram(2001); check(shared >= 0 && close(shared) == 0, "close/rebind");
  printf("nettest: fork inheritance and exit cleanup OK\n");

  int signal_fd = datagram(2002); check(signal_fd >= 0, "signal socket");
  struct sigaction action{}; action.sa_handler = handler;
  check(sigaction(SIGUSR1, &action, 0) == 0, "sigaction"); int parent = getpid();
  child = fork(); check(child >= 0, "signal fork");
  if (!child) { pause(2); sigsend(parent, SIGUSR1); exit(0); }
  check(recv(signal_fd, data, sizeof(data), 0) == -EINTR && caught, "signal interrupts recvfrom");
  wait_ok(child); caught = 0; action.sa_flags = SA_RESTART;
  check(sigaction(SIGUSR1, &action, 0) == 0, "restart action");
  child = fork(); check(child >= 0, "restart fork");
  if (!child)
  {
    pause(2); sigsend(parent, SIGUSR1); pause(2);
    check(send_echo(signal_fd, data, 7) == 7, "restart reply send"); exit(0);
  }
  check(recv(signal_fd, data, sizeof(data), 0) == 7 && caught, "SA_RESTART resumes recvfrom");
  wait_ok(child); check(close(signal_fd) == 0 && close(fd) == 0, "final close");
  printf("nettest: signal interruption OK\nnettest: ALL PASSED\n"); exit(0);
}
