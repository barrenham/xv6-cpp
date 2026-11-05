#pragma once
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "kernel/io/mqueue.h"
typedef int mqd_t;
#ifdef __cplusplus
extern "C"
{
#endif
  mqd_t mq_open(const char *, int, ...);
  int mq_close(mqd_t);
  int mq_unlink(const char *);
  int mq_send(mqd_t, const char *, size_t, unsigned);
  ssize_t mq_receive(mqd_t, char *, size_t, unsigned *);
  int mq_timedsend(mqd_t, const char *, size_t, unsigned, const struct timespec *);
  ssize_t mq_timedreceive(mqd_t, char *, size_t, unsigned *, const struct timespec *);
  int mq_getattr(mqd_t, struct mq_attr *);
  int mq_setattr(mqd_t, const struct mq_attr *, struct mq_attr *);
#ifdef __cplusplus
}
#endif
