#include <mqueue.h>
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
  socklen_t address_length = sizeof(address);
  (void)bind(-1, (const struct sockaddr *)&address, address_length);
  (void)connect(-1, (const struct sockaddr *)&address, address_length);
  (void)listen(-1, 1);
  (void)accept(-1, (struct sockaddr *)&address, &address_length);
  (void)getsockname(-1, (struct sockaddr *)&address, &address_length);
  (void)getpeername(-1, (struct sockaddr *)&address, &address_length);
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
  struct mq_attr attributes = {0};
  attributes.mq_maxmsg = 4;
  attributes.mq_msgsize = 32;
  mqd_t queue = mq_open("/header-queue", O_CREAT | O_RDWR, (mode_t)0600, &attributes);
  struct timespec deadline = {0};
  (void)clock_gettime(CLOCK_REALTIME, &deadline);
  (void)mq_timedsend(queue, "c", 1, 1, &deadline);
  char payload[32];
  (void)mq_timedreceive(queue, payload, sizeof(payload), 0, &deadline);
  (void)mq_getattr(queue, &attributes);
  (void)mq_setattr(queue, &attributes, 0);
  (void)mq_close(queue);
  (void)mq_unlink("/header-queue");
  int memory = shm_open("/header-contract", O_CREAT | O_RDWR, (mode_t)0600);
  (void)ftruncate(memory, (off_t)4096);
  int (*protect_mapping)(void *, size_t, int) = mprotect;
  (void)protect_mapping;
  int (*synchronize_mapping)(void *, size_t, int) = msync;
  (void)synchronize_mapping;
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
