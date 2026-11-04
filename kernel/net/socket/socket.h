#pragma once
#define AF_UNIX 1
#define AF_LOCAL AF_UNIX
#define AF_INET 2
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define SOCK_SEQPACKET 5
#define SOCK_CLOEXEC 0x80000
#define SOCK_NONBLOCK 0x800
#define IPPROTO_ICMP 1
#define IPPROTO_TCP 6
#define IPPROTO_UDP 17
#define SOL_SOCKET 1
#define SO_ERROR 4
#define MSG_PEEK 2
#define MSG_CTRUNC 8
#define MSG_TRUNC 0x20
#define MSG_DONTWAIT 0x40
#define MSG_EOR 0x80
#define MSG_WAITALL 0x100
#define MSG_NOSIGNAL 0x4000
#define MSG_CMSG_CLOEXEC 0x40000000
#define SCM_RIGHTS 1
#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2
#define INADDR_ANY 0U
#define INADDR_LOOPBACK 0x7f000001U
typedef unsigned socklen_t;

struct in_addr
{
  unsigned s_addr;
};

struct sockaddr
{
  unsigned short sa_family;
  char sa_data[14];
};

struct sockaddr_in
{
  unsigned short sin_family;
  unsigned short sin_port;
  struct in_addr sin_addr;
  unsigned char sin_zero[8];
};

static inline unsigned short htons(unsigned short n)
{
  return __builtin_bswap16(n);
}

static inline unsigned short ntohs(unsigned short n)
{
  return __builtin_bswap16(n);
}

static inline unsigned htonl(unsigned n)
{
  return __builtin_bswap32(n);
}

static inline unsigned ntohl(unsigned n)
{
  return __builtin_bswap32(n);
}

struct sockaddr_un
{
  unsigned short sun_family;
  char sun_path[108];
};

struct iovec
{
  void *iov_base;
  unsigned long iov_len;
};

struct msghdr
{
  void *msg_name;
  socklen_t msg_namelen;
  struct iovec *msg_iov;
  int msg_iovlen;
  void *msg_control;
  socklen_t msg_controllen;
  int msg_flags;
};

struct cmsghdr
{
  socklen_t cmsg_len;
  int cmsg_level;
  int cmsg_type;
};

#define CMSG_ALIGN(n) (((n) + sizeof(unsigned long) - 1) & ~(sizeof(unsigned long) - 1))
#define CMSG_SPACE(n) (CMSG_ALIGN(sizeof(struct cmsghdr)) + CMSG_ALIGN(n))
#define CMSG_LEN(n) (CMSG_ALIGN(sizeof(struct cmsghdr)) + (n))
#define CMSG_DATA(c) ((unsigned char *)(c) + CMSG_ALIGN(sizeof(struct cmsghdr)))
#define CMSG_FIRSTHDR(m)                                                               \
  ((m)->msg_controllen >= sizeof(struct cmsghdr) ? (struct cmsghdr *)(m)->msg_control  \
                                                 : (struct cmsghdr *)0)

static inline struct cmsghdr *
CMSG_NXTHDR(const struct msghdr *m, const struct cmsghdr *c) {
  if (c->cmsg_len < sizeof(*c))
  {
    return (struct cmsghdr *)0;
  }
  unsigned long begin = (unsigned long)m->msg_control;
  unsigned long at = (unsigned long)c;
  unsigned long step = CMSG_ALIGN(c->cmsg_len);
  if (at < begin || at - begin > m->msg_controllen ||
      step > m->msg_controllen - (at - begin) ||
      sizeof(*c) > m->msg_controllen - (at - begin) - step)
  {
    return (struct cmsghdr *)0;
  }
  return (struct cmsghdr *)(at + step);
}
