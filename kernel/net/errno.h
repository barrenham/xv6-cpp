#pragma once
#include "kernel/fs/io_status.h"
#define EADDRINUSE 98
#define ECONNREFUSED 111
#define ETIMEDOUT 110
#define ENOTCONN 107
#define EPIPE 32
#define EINPROGRESS 115
#define EALREADY 114
#define EISCONN 106
#define EMSGSIZE 90
#define ENETUNREACH 101
#define ENOBUFS 105
#define ECONNRESET 104
#define EOPNOTSUPP 95

#ifndef EINVAL
#define EINVAL 22
#endif
#ifndef ENOENT
#define ENOENT 2
#endif
#ifndef ENODEV
#define ENODEV 19
#endif
#ifndef EEXIST
#define EEXIST 17
#endif
#ifndef EBUSY
#define EBUSY 16
#endif
#ifndef EFAULT
#define EFAULT 14
#endif

#ifndef EACCES
#define EACCES 13
#endif
