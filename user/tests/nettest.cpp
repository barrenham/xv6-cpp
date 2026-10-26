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
{
  if (!value) { fprintf(2, "nettest: FAIL: %s\n", message); exit(1); }
}
void echo(int port, char *data, int length, int capacity = 1472)
{
  int result;
  do { result = send(port, Host, host_port, data, length); } while (result == -EAGAIN);
  check(result == 0, "send");
  uint32 source = 0;
  uint16 sport = 0;
  char received[1472];
  int n = recv(port, &source, &sport, received, capacity);
  int expected = length < capacity ? length : capacity;
  check(n == expected && source == Host && sport == host_port &&
        memcmp(data, received, expected) == 0, "UDP echo metadata/content");
}
void wait_ok(int pid)
{
  int status = -1;
  check(pid > 0 && waitpid(pid, &status, 0) == pid && status == 0, "child status");
}
}
extern "C" int main(int argc, char **argv)
{
  if (argc == 2) host_port = atoi(argv[1]);
  check(bind(2000) == 0 && bind(2000) < 0, "exclusive bind");
  char data[1473];
  for (int i = 0; i < 1473; ++i) data[i] = i * 13;
  for (int i = 0; i < 80; ++i) echo(2000, data, 1 + i * 17);
  echo(2000, data, 1472);
  echo(2000, data, 0);
  echo(2000, data, 31, 7);
  echo(2000, data, 3);
  check(send(2000, Host, host_port, data, 1473) < 0, "MTU bound");
  check(send(2000, Host, host_port, reinterpret_cast<char *>(~0UL), 1) < 0,
        "bad send address");
  uint32 source;
  uint16 sport;
  check(recv(2222, &source, &sport, data, sizeof(data)) < 0, "unbound receive");
  printf("nettest: ring wrap, MTU, zero length, truncation OK\n");

  int child = fork();
  check(child >= 0, "fork binding");
  if (!child)
  {
    check(unbind(2000) == 0 && bind(2000) < 0, "shared binding lifetime");
    check(bind(2001) == 0, "child bind");
    echo(2001, data, 17);
    exit(0);
  }
  wait_ok(child);
  echo(2000, data, 29);
  check(bind(2001) == 0, "exit released child port");
  child = fork();
  check(child >= 0, "fork receive");
  if (!child) { echo(2001, data, 23); exit(0); }
  wait_ok(child);
  check(unbind(2001) == 0 && bind(2001) == 0, "unbind/rebind");
  check(unbind(2001) == 0, "unbind");
  printf("nettest: fork inheritance and exit cleanup OK\n");

  check(bind(2002) == 0, "signal port");
  struct sigaction action{};
  action.sa_handler = handler;
  check(sigaction(SIGUSR1, &action, 0) == 0, "sigaction");
  int parent = getpid();
  child = fork();
  check(child >= 0, "signal fork");
  if (!child) { pause(2); sigsend(parent, SIGUSR1); exit(0); }
  check(recv(2002, &source, &sport, data, sizeof(data)) == -EINTR && caught,
        "signal interrupts recv");
  wait_ok(child);
  caught = 0;
  action.sa_flags = SA_RESTART;
  check(sigaction(SIGUSR1, &action, 0) == 0, "restart action");
  child = fork();
  check(child >= 0, "restart fork");
  if (!child)
  {
    pause(2); sigsend(parent, SIGUSR1); pause(2);
    check(send(2002, Host, host_port, data, 7) == 0, "restart reply send");
    exit(0);
  }
  check(recv(2002, &source, &sport, data, sizeof(data)) == 7 && caught,
        "SA_RESTART resumes recv");
  wait_ok(child);
  check(unbind(2002) == 0, "signal unbind");
  check(unbind(2000) == 0, "final unbind");
  printf("nettest: signal interruption OK\n");
  printf("nettest: ALL PASSED\n");
  exit(0);
}
