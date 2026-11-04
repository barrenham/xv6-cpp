#pragma once
#include <sys/types.h>
#include "kernel/net/socket/socket.h"
typedef unsigned short sa_family_t;

#ifdef __cplusplus
extern "C"
{
#endif
  int socket(int, int, int);
  int socketpair(int, int, int, int[2]);
  ssize_t sendmsg(int, const struct msghdr *, int);
  ssize_t recvmsg(int, struct msghdr *, int);
  ssize_t send(int, const void *, size_t, int);
  ssize_t recv(int, void *, size_t, int);
  ssize_t sendto(int, const void *, size_t, int, const struct sockaddr *, socklen_t);
  ssize_t recvfrom(int, void *, size_t, int, struct sockaddr *, socklen_t *);
  int bind(int, const struct sockaddr *, socklen_t);
  int connect(int, const struct sockaddr *, socklen_t);
  int listen(int, int);
  int accept(int, struct sockaddr *, socklen_t *);
  int shutdown(int, int);
  int getsockname(int, struct sockaddr *, socklen_t *);
  int getpeername(int, struct sockaddr *, socklen_t *);
  int getsockopt(int, int, int, void *, socklen_t *);

#ifdef __cplusplus
}
#endif
