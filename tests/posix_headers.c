/* Public IPC headers must remain usable by an independent C11 program. */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>

_Static_assert((ssize_t)-1 < 0, "ssize_t is signed");
_Static_assert((off_t)-1 < 0, "off_t is signed");

int
main(void) {
  int endpoints[2];
  struct sockaddr_un address = {0};
  sa_family_t family = AF_UNIX;
  address.sun_family = family;
  struct iovec vector = {&address, sizeof(address)};
  struct msghdr message = {0};
  message.msg_iov = &vector;
  message.msg_iovlen = 1;
  if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, endpoints) < 0)
  {
    return errno;
  }
  ssize_t sent = sendmsg(endpoints[0], &message, 0);
  struct pollfd event = {endpoints[1], POLLIN, 0};
  nfds_t count = 1;
  if (sent >= 0 && poll(&event, count, 0) > 0)
  {
    (void)recvmsg(endpoints[1], &message, 0);
  }
  int memory = shm_open("/header-contract", O_CREAT | O_RDWR, (mode_t)0600);
  (void)ftruncate(memory, (off_t)4096);
  void *mapping = mmap(0, (size_t)4096, PROT_READ | PROT_WRITE, MAP_SHARED, memory, 0);
  if (mapping != MAP_FAILED)
  {
    (void)munmap(mapping, 4096);
  }
  (void)shm_unlink("/header-contract");
  (void)close(memory);
  (void)close(endpoints[0]);
  return close(endpoints[1]);
}
