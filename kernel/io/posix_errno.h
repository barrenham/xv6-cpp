#pragma once
#ifndef EPERM
#define EPERM 1
#define ENOENT 2
#define EINTR 4
#define EIO 5
#define EBADF 9
#define EAGAIN 11
#define ENOMEM 12
#define EACCES 13
#define EFAULT 14
#define EBUSY 16
#define EEXIST 17
#define ENODEV 19
#define ENOTDIR 20
#define EINVAL 22
#define ENFILE 23
#define EMFILE 24
#define EFBIG 27
#define ENOSPC 28
#define ESPIPE 29
#define EROFS 30
#define EPIPE 32
#define ENAMETOOLONG 36
#define ENOSYS 38
#define ELOOP 40
#define EOVERFLOW 75
#define ENOTSOCK 88
#define EMSGSIZE 90
#define EPROTOTYPE 91
#define EOPNOTSUPP 95
#define EAFNOSUPPORT 97
#define EADDRINUSE 98
#define ENETUNREACH 101
#define ECONNRESET 104
#define EALREADY 114
#define EINPROGRESS 115
#define ENOBUFS 105
#define EISCONN 106
#define ENOTCONN 107
#define ECONNREFUSED 111
#define ETIMEDOUT 110
#endif
#ifndef EWOULDBLOCK
#define EWOULDBLOCK EAGAIN
#endif
